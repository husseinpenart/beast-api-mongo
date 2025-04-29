#include "products/product_routes.hpp"
#include "models/product/Product.hpp"
#include <nlohmann/json.hpp>
#include <mongocxx/client.hpp>
#include <mongocxx/uri.hpp>
#include <iostream>

namespace http = boost::beast::http;

void ProductController::create(http::request<http::string_body> &req,
                               http::response<http::string_body> &res,
                               mongocxx::database &db)
{
    try
    {
        std::cout << "Parsing JSON body: " << req.body() << "\n";
        auto jsonBody = nlohmann::json::parse(req.body());

        std::string name = jsonBody["name"];
        double price = jsonBody["price"];
        std::string description = jsonBody.value("description", "");
        std::vector<std::string> categories = jsonBody.value("categories", std::vector<std::string>{});

        Product product(name, description, price, categories);
        auto collection = db["products"];
        auto result = collection.insert_one(product.toBson());

        std::cout << "Inserted product with ID: " << result->inserted_id().get_oid().value.to_string() << "\n";

        nlohmann::json response = {
            {"message", "Product created successfully"},
            {"id", result->inserted_id().get_oid().value.to_string()}};

        res.result(http::status::created);
        res.set(http::field::content_type, "application/json");
        res.body() = response.dump();
        res.prepare_payload();
    }
    catch (const std::exception &e)
    {
        std::cerr << "Error in ProductController::create: " << e.what() << "\n";
        res.result(http::status::bad_request);
        res.set(http::field::content_type, "text/plain");
        res.body() = std::string("Error: ") + e.what();
        res.prepare_payload();
    }
}