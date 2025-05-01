#pragma once
#include <boost/beast/http.hpp>
#include <mongocxx/database.hpp>

namespace http = boost::beast::http;

class UserRouter {
public:
    static void route(http::request<http::string_body>& req, http::response<http::string_body>& res, mongocxx::database& db);
};