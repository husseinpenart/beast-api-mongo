#include "routes/mainRouter/router.hpp"
#include "routes/products/product_routes.hpp"
#include "routes/users/UserRouter.hpp"

namespace http = boost::beast::http;

void Router::route(http::request<http::string_body>& req, http::response<http::string_body>& res, mongocxx::database& db) {
    std::string target = std::string(req.target());
    if (target.find("/products") == 0) { // C++17-compatible replacement for starts_with
        ProductRouter::route(req, res, db);
    } else if (target.find("/register") == 0 || target.find("/login") == 0 || target.find("/profile") == 0) {
        UserRouter::route(req, res, db);
    } else {
        res.result(http::status::not_found);
        res.set(http::field::content_type, "text/plain");
        res.body() = "Not Found";
        res.prepare_payload();
    }
}