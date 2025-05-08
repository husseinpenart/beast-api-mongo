#include "routes/postRoute/post_route.hpp"
#include "controllers/post/postControllers.hpp"
#include <iostream>
namespace http = boost::beast::http;
using namespace std;

void postRouter::route(http::request<http::string_body> &req,
                       http::response<http::string_body> &res, mongocxx::database &db)
{
    string target = string(req.target());
    if (req.method() == http::verb::post && target == "/blogs")
    {
        postContrllers::create(req, res, db);
    }
    else if (req.method() == http::verb::get && target == "/blogs")
    {
        postContrllers::read(req, res, db);
    }
    else if (req.method() == http::verb::put && target.find("/blogs/") == 0)
    {
        string id = target.substr(7);
        postContrllers::update(req, res, db, id);
    }
    else if (req.method() == http::verb::delete_ && target.find("/blogs/") == 0)
    {
        string id = target.substr(7);
        postContrllers::del(req, res, db, id);
    }
    else if (req.method() == http::verb::get && target.find("/blogs/") == 0)
    {
        std::string id = target.substr(7);
        postContrllers::getbyId(req, res, db, id);
    }
    else
    {
        res.result(http::status::not_found);
        res.set(http::field::content_type, "text/plain");
        res.body() = "Not Found";
        res.prepare_payload();
    }
}