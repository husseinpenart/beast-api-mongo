#pragma once
#include <mongocxx/database.hpp>
#include <boost/beast/http.hpp>
#include <iostream>
namespace http = boost::beast::http;

class postContrllers
{
public:
    static void create(http::request<http::string_body> &req, http::response<http::string_body> &res, mongocxx::database &db);
    static void read(http::request<http::string_body> &req, http::response<http::string_body> &res, mongocxx::database &db);
    static void update(http::request<http::string_body> &req, http::response<http::string_body> &res, mongocxx::database &db, const std::string &id);
    static void del(http::request<http::string_body> &req, http::response<http::string_body> &res, mongocxx::database &db, const std::string &id);
    static void getbyId(http::request<http::string_body> &req, http::response<http::string_body> &res, mongocxx::database &db, const std::string &id);
};