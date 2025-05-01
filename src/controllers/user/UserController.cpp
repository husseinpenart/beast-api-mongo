#include "UserControllers.hpp"
#include "models/user/User.hpp"
#include <nlohmann/json.hpp>
#include <jwt-cpp/jwt.h>
#include <regex>
#include <mongocxx/exception/exception.hpp>
#include <windows.h>
#include <bcrypt.h>
#include <iomanip>
#include <sstream>
#include <random>
#include <cstdlib>
#include <stdexcept>
#include "utils/Env.hpp"
namespace http = boost::beast::http;
using json = nlohmann::json;
using namespace std;
// Load environment variables at startup (static initialization)
static const auto env = Env::load(".env", true);
static const std::string JWT_SECRET = Env::get(env, "JWT_SECRET", "fallback-secret-key-for-dev-only");
#pragma comment(lib, "bcrypt.lib")

std::string generate_salt()
{
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, 255);
    std::vector<UCHAR> salt(16);
    for (UCHAR &byte : salt)
    {
        byte = static_cast<UCHAR>(dis(gen));
    }
    std::stringstream ss;
    for (UCHAR byte : salt)
    {
        ss << std::hex << std::setw(2) << std::setfill('0') << (int)byte;
    }
    return ss.str();
}

std::string hash_password(const std::string &password, const std::string &salt)
{
    BCRYPT_ALG_HANDLE hAlg = nullptr;
    NTSTATUS status = BCryptOpenAlgorithmProvider(&hAlg, BCRYPT_SHA256_ALGORITHM, nullptr, BCRYPT_ALG_HANDLE_HMAC_FLAG);
    if (!BCRYPT_SUCCESS(status))
    {
        throw std::runtime_error("Failed to open BCrypt algorithm provider");
    }

    std::vector<UCHAR> derived_key(32);
    status = BCryptDeriveKeyPBKDF2(
        hAlg,
        (PUCHAR)password.c_str(), password.length(),
        (PUCHAR)salt.c_str(), salt.length() / 2,
        10000,
        derived_key.data(), derived_key.size(),
        0);

    BCryptCloseAlgorithmProvider(hAlg, 0);

    if (!BCRYPT_SUCCESS(status))
    {
        throw std::runtime_error("Failed to derive key with PBKDF2");
    }

    std::stringstream ss;
    for (UCHAR byte : derived_key)
    {
        ss << std::hex << std::setw(2) << std::setfill('0') << (int)byte;
    }
    return salt + ":" + ss.str();
}

bool verify_password(const std::string &password, const std::string &stored)
{
    size_t pos = stored.find(':');
    if (pos == std::string::npos)
    {
        return false;
    }
    std::string salt = stored.substr(0, pos);
    std::string hashed = hash_password(password, salt);
    return hashed == stored;
}

void UserControllers::register_user(http::request<http::string_body> &req, http::response<http::string_body> &res, mongocxx::database &db)
{
    try
    {
        json jsonBody = json::parse(req.body());
        std::string name = jsonBody.value("name", "");
        std::string username = jsonBody.value("username", "");
        std::string email = jsonBody.value("email", "");
        std::string password = jsonBody.value("password", "");
        std::string phone = jsonBody.value("phone", "");

        if (name.empty() || username.empty() || email.empty() || password.empty())
        {
            throw std::runtime_error("All fields (name, username, email, password) are required");
        }

        const std::regex email_regex(R"(^[a-zA-Z0-9._%+-]+@[a-zA-Z0-9.-]+\.[a-zA-Z]{2,}$)");
        if (!std::regex_match(email, email_regex))
        {
            throw std::runtime_error("Invalid email format");
        }

        const std::regex password_regex(R"(^(?=.*[a-z])(?=.*[A-Z])(?=.*\d).{8,}$)");
        if (!std::regex_match(password, password_regex))
        {
            throw std::runtime_error("Password must be at least 8 characters, with one uppercase, one lowercase, and one digit");
        }

        if (!phone.empty())
        {
            const std::regex phone_regex(R"(^\+?\d{10,15}$)");
            if (!std::regex_match(phone, phone_regex))
            {
                throw std::runtime_error("Invalid phone number format (10-15 digits, optional +)");
            }
        }

        auto collection = db["users"];
        bsoncxx::builder::stream::document filter_builder{};
        filter_builder << "$or" << bsoncxx::builder::stream::open_array
                       << bsoncxx::builder::stream::open_document << "username" << username << bsoncxx::builder::stream::close_document
                       << bsoncxx::builder::stream::open_document << "email" << email << bsoncxx::builder::stream::close_document
                       << bsoncxx::builder::stream::close_array;
        auto filter = filter_builder << bsoncxx::builder::stream::finalize;
        if (collection.count_documents(filter.view()) > 0)
        {
            throw std::runtime_error("Username or email already exists");
        }

        std::string salt = generate_salt();
        std::string hashed_password = hash_password(password, salt);

        User user(name, username, email, hashed_password, phone);
        auto result = collection.insert_one(user.toBson());

        auto token = jwt::create()
                         .set_issuer("beast_mongo_api")
                         .set_subject(result->inserted_id().get_oid().value.to_string())
                         .set_payload_claim("username", jwt::claim(username))
                         .set_issued_at(std::chrono::system_clock::now())
                         .set_expires_at(std::chrono::system_clock::now() + std::chrono::hours(24))
                         .sign(jwt::algorithm::hs256{JWT_SECRET});

        json response = {
            {"message", "User registered successfully"},
            {"jwt", token}};

        res.result(http::status::created);
        res.set(http::field::content_type, "application/json");
        res.body() = response.dump();
        res.prepare_payload();
    }
    catch (const std::exception &e)
    {
        res.result(http::status::bad_request);
        res.set(http::field::content_type, "text/plain");
        res.body() = "Error: " + std::string(e.what());
        res.prepare_payload();
    }
}

void UserControllers::login(http::request<http::string_body> &req, http::response<http::string_body> &res, mongocxx::database &db)
{
    try
    {
        json jsonBody = json::parse(req.body());
        std::string username = jsonBody.value("username", "");
        std::string password = jsonBody.value("password", "");

        if (username.empty() || password.empty())
        {
            throw std::runtime_error("Username and password are required");
        }

        auto collection = db["users"];
        bsoncxx::builder::stream::document filter_builder{};
        filter_builder << "username" << username;
        auto filter = filter_builder << bsoncxx::builder::stream::finalize;
        auto result = collection.find_one(filter.view());

        if (!result)
        {
            throw std::runtime_error("Invalid username or password");
        }

        User user = User::fromBson(result->view());
        if (!verify_password(password, user.password))
        {
            throw std::runtime_error("Invalid username or password");
        }

        auto token = jwt::create()
                         .set_issuer("beast_mongo_api")
                         .set_subject(user.id)
                         .set_payload_claim("username", jwt::claim(username))
                         .set_issued_at(std::chrono::system_clock::now())
                         .set_expires_at(std::chrono::system_clock::now() + std::chrono::hours(24))
                         .sign(jwt::algorithm::hs256{JWT_SECRET});

        json response = {
            {"message", "Login successful"},
            {"jwt", token}};

        res.result(http::status::ok);
        res.set(http::field::content_type, "application/json");
        res.body() = response.dump();
        res.prepare_payload();
    }
    catch (const std::exception &e)
    {
        res.result(http::status::unauthorized);
        res.set(http::field::content_type, "text/plain");
        res.body() = "Error: " + std::string(e.what());
        res.prepare_payload();
    }
}

void UserControllers::get_profile(http::request<http::string_body> &req, http::response<http::string_body> &res, mongocxx::database &db)
{
    try
    {

        // Extract Authorization header
        std::string auth_header = std::string(req[http::field::authorization]);
        if (auth_header.empty())
        {
            throw std::runtime_error("Authorization header missing");
        }

        // Extract token from "Bearer <token>"
        if (auth_header.find("Bearer ") != 0)
        {
            throw std::runtime_error("Invalid Authorization header format");
        }
        std::string token = auth_header.substr(7); // Skip "Bearer "

        auto decoded = jwt::decode(token);
        auto verifier = jwt::verify()
                            .allow_algorithm(jwt::algorithm::hs256{JWT_SECRET}) // Use loaded secret
                            .with_issuer("beast_mongo_api");
        verifier.verify(decoded);

        // Get user ID from token's subject
        std::string user_id = decoded.get_subject();

        // Query MongoDB for user
        auto collection = db["users"];
        bsoncxx::builder::stream::document filter_builder{};
        filter_builder << "_id" << bsoncxx::oid(user_id);
        auto filter = filter_builder << bsoncxx::builder::stream::finalize;
        auto result = collection.find_one(filter.view());

        if (!result)
        {
            throw std::runtime_error("User not found");
        }

        // Convert to User object and prepare response
        User user = User::fromBson(result->view());
        json response = {
            {"id", user.id},
            {"name", user.name},
            {"username", user.username},
            {"email", user.email},
            {"phone", user.phone}};

        res.result(http::status::ok);
        res.set(http::field::content_type, "application/json");
        res.body() = response.dump();
        res.prepare_payload();
    }
    catch (const jwt::error::token_verification_error &e)
    {
        res.result(http::status::unauthorized);
        res.set(http::field::content_type, "text/plain");
        res.body() = "Error: Invalid or expired token";
        res.prepare_payload();
    }
    catch (const std::exception &e)
    {
        res.result(http::status::bad_request);
        res.set(http::field::content_type, "text/plain");
        res.body() = "Error: " + std::string(e.what());
        res.prepare_payload();
    }
}