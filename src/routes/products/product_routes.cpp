#include "routes/products/product_routes.hpp"
#include "controllers/product/product_controller.hpp"

namespace http = boost::beast::http;

void ProductRouter::route(http::request<http::string_body> &req, http::response<http::string_body> &res, mongocxx::database &db)
{
    std::string target = std::string(req.target());
    if (req.method() == http::verb::post && target == "/products")
    {
        ProductController::create(req, res, db);
    }
    else if (req.method() == http::verb::get && target == "/products")
    {
        ProductController::read(req, res, db);
    }
    else if (req.method() == http::verb::put && target == "/products")
    {
        ProductController::update(req, res, db);
    }
    else if (req.method() == http::verb::delete_ && target == "/products")
    {
        ProductController::del(req, res, db);
    }
    else
    {
        res.result(http::status::not_found);
        res.set(http::field::content_type, "text/plain");
        res.body() = "Not Found";
        res.prepare_payload();
    }
}