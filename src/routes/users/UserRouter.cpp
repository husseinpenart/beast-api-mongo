#include "UserRouter.hpp"
#include "controllers/user/UserControllers.hpp"
namespace http = boost::beast::http;

void UserRouter::route(http::request<http::string_body> &req, http::response<http::string_body> &res, mongocxx::database &db)
{
    std::string target = std::string(req.target());
    if (req.method() == http::verb::post && target == "/register")
    {
        UserControllers::register_user(req, res, db);
    }
    else if (req.method() == http::verb::post && target == "/login")
    {
        UserControllers::login(req, res, db);
    }
    else if (req.method() == http::verb::get && target == "/profile")
    {
        UserControllers::get_profile(req, res, db);
    }
    else
    {
        res.result(http::status::not_found);
        res.set(http::field::content_type, "text/plain");
        res.body() = "Not Found";
        res.prepare_payload();
    }
}