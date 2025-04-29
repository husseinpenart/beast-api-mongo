#include "controllers/product/product_controller.hpp"
#include "models/product/Product.hpp"
#include <nlohmann/json.hpp>
#include <fstream>
#include <filesystem>
#include <iostream>
#include <bsoncxx/builder/stream/document.hpp>
#include <bsoncxx/builder/stream/array.hpp>
#include <bsoncxx/builder/stream/helpers.hpp>
#include <bsoncxx/oid.hpp>
#include <mongocxx/exception/exception.hpp>

namespace http = boost::beast::http;
namespace fs = std::filesystem;
using json = nlohmann::json;

std::string get_upload_path()
{
    return "../../uploads/";
}

void ProductController::create(http::request<http::string_body> &req, http::response<http::string_body> &res, mongocxx::database &db)
{
    try
    {

        // Check for multipart/form-data
        std::string content_type = std::string(req[http::field::content_type]);
        if (content_type.find("multipart/form-data") == std::string::npos)
        {
            throw std::runtime_error("Expected multipart/form-data");
        }

        // Parse multipart form data (simplified)
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

        std::string title, description, image_path;
        double price = 0.0;
        int stock = 0;
        std::vector<std::string> multiimage_paths;
        fs::create_directories(get_upload_path());

        for (const auto &part : parts)
        {
            if (part.empty() || part.find("Content-Disposition") == std::string::npos)
                continue;
            if (part.find("name=\"title\"") != std::string::npos)
            {
                title = part.substr(part.find("\r\n\r\n") + 4, part.rfind("\r\n--") - part.find("\r\n\r\n") - 4);
            }
            else if (part.find("name=\"price\"") != std::string::npos)
            {
                price = std::stod(part.substr(part.find("\r\n\r\n") + 4, part.rfind("\r\n--") - part.find("\r\n\r\n") - 4));
            }
            else if (part.find("name=\"stock\"") != std::string::npos)
            {
                try
                {
                    stock = std::stoi(part.substr(part.find("\r\n\r\n") + 4, part.rfind("\r\n--") - part.find("\r\n\r\n") - 4));
                }
                catch (const std::invalid_argument &e)
                {
                    stock = 0; // default value if the argument is invalid
                }
            }
            else if (part.find("name=\"description\"") != std::string::npos)
            {
                description = part.substr(part.find("\r\n\r\n") + 4, part.rfind("\r\n--") - part.find("\r\n\r\n") - 4);
            }
            else if (part.find("name=\"image\"") != std::string::npos)
            {
                std::string filename = "image_" + std::to_string(std::time(nullptr)) + ".jpg";
                image_path = get_upload_path() + filename;
                std::ofstream file(image_path, std::ios::binary);
                file << part.substr(part.find("\r\n\r\n") + 4, part.rfind("\r\n--") - part.find("\r\n\r\n") - 4);
                file.close();
            }
            else if (part.find("name=\"multiimage\"") != std::string::npos)
            {
                std::string filename = "multiimage_" + std::to_string(std::time(nullptr)) + "_" + std::to_string(multiimage_paths.size()) + ".jpg";
                std::string path = get_upload_path() + filename;
                std::ofstream file(path, std::ios::binary);
                file << part.substr(part.find("\r\n\r\n") + 4, part.rfind("\r\n--") - part.find("\r\n\r\n") - 4);
                file.close();
                multiimage_paths.push_back(path);
            }
        }

        Product product(title, price, stock, description, image_path, multiimage_paths);
        auto collection = db["products"];
        auto result = collection.insert_one(product.toBson());

        json response = {
            {"message", "Product created"},
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

void ProductController::read(http::request<http::string_body> &req, http::response<http::string_body> &res, mongocxx::database &db)
{
    try
    {
        auto collection = db["products"];
        json response = json::array();

        for (auto doc : collection.find({}))
        {
            Product product = Product::fromBson(doc);
            response.push_back({{"id", product.id},
                                {"title", product.title},
                                {"price", product.price},
                                {"stock", product.stock},
                                {"description", product.description},
                                {"image", product.image},
                                {"multiimage", product.multiimage}});
        }

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

void ProductController::update(http::request<http::string_body> &req, http::response<http::string_body> &res, mongocxx::database &db)
{
    try
    {
        json jsonBody = json::parse(req.body());
        std::string id = jsonBody["id"];
        if (id.empty())
            throw std::runtime_error("ID required");

        std::string title = jsonBody.value("title", "");
        double price = jsonBody.value("price", 0.0);
        int stock = jsonBody.value("stock", 0);
        std::string description = jsonBody.value("description", "");
        std::string image = jsonBody.value("image", "");
        std::vector<std::string> multiimage = jsonBody.value("multiimage", std::vector<std::string>{});

        auto collection = db["products"];

        // Build filter
        bsoncxx::builder::stream::document filter_builder{};
        filter_builder << "_id" << bsoncxx::oid(id);
        auto filter = filter_builder << bsoncxx::builder::stream::finalize;

        // Build multiimage array separately
        bsoncxx::builder::stream::array multiimage_array{};
        for (const auto &img : multiimage)
        {
            multiimage_array << img;
        }

        // Build update document
        bsoncxx::builder::stream::document update_builder{};
        update_builder << "$set" << bsoncxx::builder::stream::open_document
                       << "title" << title
                       << "price" << price
                       << "stock" << stock
                       << "description" << description
                       << "image" << image
                       << "multiimage" << multiimage_array
                       << bsoncxx::builder::stream::close_document;
        auto update = update_builder << bsoncxx::builder::stream::finalize;

        collection.update_one(filter.view(), update.view());

        json response = {{"message", "Product updated"}};
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

void ProductController::del(http::request<http::string_body> &req, http::response<http::string_body> &res, mongocxx::database &db)
{
    try
    {
        json jsonBody = json::parse(req.body());
        std::string id = jsonBody["id"];
        if (id.empty())
            throw std::runtime_error("ID required");

        auto collection = db["products"];
        bsoncxx::builder::stream::document filter_builder{};
        filter_builder << "_id" << bsoncxx::oid(id);
        auto filter = filter_builder << bsoncxx::builder::stream::finalize;

        collection.delete_one(filter.view());

        json response = {{"message", "Product deleted"}};
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
