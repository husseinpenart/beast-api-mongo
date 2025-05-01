#pragma once
#include <boost/beast/http.hpp>
#include <mongocxx/database.hpp>

namespace http = boost::beast::http;

class UserControllers
{
public:
    static void register_user(http::request<http::string_body> &req, http::response<http::string_body> &res, mongocxx::database &db);
    static void login(http::request<http::string_body> &req, http::response<http::string_body> &res, mongocxx::database &db);
    static void get_profile(http::request<http::string_body> &req, http::response<http::string_body> &res, mongocxx::database &db);

};