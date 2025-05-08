#include "controllers/post/postControllers.hpp"
#include "models/blog/blogModel.hpp"
#include <nlohmann/json.hpp>
#include <bsoncxx/builder/stream/document.hpp>
#include <bsoncxx/builder/stream/helpers.hpp>
#include <bsoncxx/oid.hpp>
#include <bsoncxx/json.hpp>
#include <mongocxx/exception/exception.hpp>
#include <string>
#include <fstream>
#include <filesystem>
#include <iostream>
#include <type_traits>
#include "postControllers.hpp"
#include "utils/Base.hpp"
namespace http = boost::beast::http;
using json = nlohmann::json;

using namespace std;
namespace fs = std::filesystem;

void postContrllers::create(http::request<http::string_body> &req, http::response<http::string_body> &res, mongocxx::database &db)
{
    try
    {
        // 1. Check Content-Type
        std::string content_type = std::string(req[http::field::content_type]);
        if (content_type.find("multipart/form-data") == std::string::npos)
        {
            throw std::runtime_error("Expected multipart/form-data");
        }

        // 2. Extract boundary and body
        std::string body = req.body();
        std::string boundary = "--" + content_type.substr(content_type.find("boundary=") + 9);

        std::vector<std::string> parts;
        size_t pos = 0, next;
        while ((next = body.find(boundary, pos)) != std::string::npos)
        {
            parts.push_back(body.substr(pos, next - pos));
            pos = next + boundary.length();
        }
        parts.push_back(body.substr(pos));

        // 3. Variables
        std::string title, category, shortDesc, description, image_path;
        fs::create_directories(get_upload_path());

        // 4. Parse each part
        for (const auto &part : parts)
        {
            if (part.empty() || part.find("Content-Disposition") == std::string::npos)
                continue;

            if (part.find("name=\"title\"") != std::string::npos)
            {
                title = part.substr(part.find("\r\n\r\n") + 4, part.rfind("\r\n--") - part.find("\r\n\r\n") - 4);
            }
            else if (part.find("name=\"category\"") != std::string::npos)
            {
                category = part.substr(part.find("\r\n\r\n") + 4, part.rfind("\r\n--") - part.find("\r\n\r\n") - 4);
            }
            else if (part.find("name=\"shortDesc\"") != std::string::npos)
            {
                shortDesc = part.substr(part.find("\r\n\r\n") + 4, part.rfind("\r\n--") - part.find("\r\n\r\n") - 4);
            }
            else if (part.find("name=\"description\"") != std::string::npos)
            {
                description = part.substr(part.find("\r\n\r\n") + 4, part.rfind("\r\n--") - part.find("\r\n\r\n") - 4);
            }
            else if (part.find("name=\"image\"") != std::string::npos)
            {
                std::string filename = "blogimg_" + std::to_string(std::time(nullptr)) + ".jpg";
                image_path = get_upload_path() + filename;
                std::ofstream file(image_path, std::ios::binary);
                file << part.substr(part.find("\r\n\r\n") + 4, part.rfind("\r\n--") - part.find("\r\n\r\n") - 4);
                file.close();
            }
        }

        // 5. Create and insert blog
        blogModel blog(title, category, shortDesc, description, image_path);
        auto collection = db["blogs"];
        auto result = collection.insert_one(blog.toBson());

        // 6. Send response
        json response = {
            {"message", "Blog created"},
            {"id", result->inserted_id().get_oid().value.to_string()}};

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
// get all

void postContrllers::read(http::request<http::string_body> &req, http::response<http::string_body> &res, mongocxx::database &db)
{
    try
    {
        auto collection = db["blogs"];
        json response = json::array();
        for (auto doc : collection.find({}))
        {
            blogModel blog = blogModel::fromBson(doc);
            response.push_back({{"id", blog.id},
                                {"title", blog.title},
                                {"category", blog.category},
                                {"shortDesc", blog.shortDesc},
                                {"description", blog.description},
                                {"image", blog.image}

            });
        }
        cout << "response : " << response.dump() << endl;
        res.result(http::status::ok);
        res.set(http::field::content_type, "application/json");
        res.body() = response.dump();
        res.prepare_payload();
    }
    catch (const std::exception &e)
    {
        res.result(http::status::internal_server_error);
        res.set(http::field::content_type, "text/plain");
        res.body() = "Error: " + std::string(e.what());
        res.prepare_payload();
    }
}

// update blog
void postContrllers::update(http::request<http::string_body> &req,
                            http::response<http::string_body> &res,
                            mongocxx::database &db,
                            const std::string &id)
{
    try
    {
        cout << "Im Hereeeeeeeee" << endl;
        json jsonBody = json::parse(req.body());

        if (id.empty())
        {
            throw std::runtime_error("id is required");
        }

        std::string title = jsonBody.value("title", "");
        std::string category = jsonBody.value("category", "");
        std::string shortDesc = jsonBody.value("shortDesc", "");
        std::string description = jsonBody.value("description", "");
        std::string image = jsonBody.value("image", "");

        auto collection = db["blogs"];

        bsoncxx::builder::stream::document filter_builder{};
        filter_builder << "_id" << bsoncxx::oid(id);
        auto filter = filter_builder << bsoncxx::builder::stream::finalize;

        bsoncxx::builder::stream::document update_builder{};
        update_builder << "$set" << bsoncxx::builder::stream::open_document
                       << "title" << title
                       << "category" << category
                       << "shortDesc" << shortDesc
                       << "description" << description
                       << "image" << image
                       << bsoncxx::builder::stream::close_document;

        auto update = update_builder << bsoncxx::builder::stream::finalize;

        auto result = collection.update_one(filter.view(), update.view());
        if (!result || result->modified_count() == 0)
        {
            throw std::runtime_error("No document updated. Possibly invalid ID or no changes.");
        }

        json response = {{"message", "Blog titled '" + title + "' updated successfully"}};
        res.result(http::status::ok);
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

void postContrllers::del(http::request<http::string_body> &req, http::response<http::string_body> &res, mongocxx::database &db, const std::string &id)
{
    try
    {
        if (id.empty())
            throw std::runtime_error("ID required");
        auto collection = db["blogs"];
        bsoncxx::builder::stream::document filter_builder{};
        filter_builder << "_id" << bsoncxx::oid(id);
        auto filter = filter_builder << bsoncxx::builder::stream::finalize;
        collection.delete_one(filter.view());
        json response = {{"messaage: ", "item with id of: ", id, "deleted successfully"}};
        res.result(http::status::ok);
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

void postContrllers::getbyId(http::request<http::string_body> &req, http::response<http::string_body> &res, mongocxx::database &db, const std::string &id)
{
    try
    {
        if (id.empty())
        {
            throw std::runtime_error("id is required");
        }
        auto collection = db["blogs"];

        // Create filter for the given ID
        bsoncxx::builder::stream::document filter_builder{};
        filter_builder << "_id" << bsoncxx::oid(id);
        auto filter = filter_builder << bsoncxx::builder::stream::finalize;

        // Try to find the document
        auto maybe_doc = collection.find_one(filter.view());

        if (!maybe_doc)
        {
            throw std::runtime_error("No document found with the given ID");
        }

        // Convert BSON document to JSON
        json result_json = json::parse(bsoncxx::to_json(maybe_doc->view()));

        res.result(http::status::ok);
        res.set(http::field::content_type, "application/json");
        res.body() = result_json.dump();
        res.prepare_payload();
    }
    catch (const std::exception &e)
    {
        std::cerr << e.what() << '\n';
    }
}