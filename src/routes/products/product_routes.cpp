#include "routes/products/product_routes.hpp"
#include "controllers/product/product_controller.hpp"

namespace http = boost::beast::http;

void ProductRouter::route(http::request<http::string_body> &req, http::response<http::string_body> &res, mongocxx::database &db)
{
    std::string target = std::string(req.target());

    // Helper function to extract clean ID from path
    auto extract_id = [](const std::string &target) -> std::string
    {
        std::string id = target.substr(10); // After "/products/"
        // Remove trailing slashes or query parameters
        auto end_pos = id.find_first_of("/?");
        if (end_pos != std::string::npos)
        {
            id = id.substr(0, end_pos);
        }
        return id;
    };

    if (req.method() == http::verb::post && target == "/products")
    {
        ProductController::create(req, res, db);
    }
    else if (req.method() == http::verb::get && target == "/products")
    {
        ProductController::read(req, res, db);
    }
    else if (req.method() == http::verb::put && target.find("/products/") == 0)
    {
        std::string id = extract_id(target);
        ProductController::update(req, res, db, id);
    }
    else if (req.method() == http::verb::delete_ && target.find("/products/") == 0)
    {
        std::string id = extract_id(target);
        ProductController::del(req, res, db, id);
    }
    else if (req.method() == http::verb::get && target.find("/products/") == 0)
    {
        std::string id = extract_id(target);
        ProductController::getByid(req, res, db, id);
    }
    else
    {
        res.result(http::status::not_found);
        res.set(http::field::content_type, "text/plain");
        res.body() = "Not Found";
        res.prepare_payload();
    }
}