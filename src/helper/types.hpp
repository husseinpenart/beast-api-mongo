#pragma once
#include <boost/beast/http.hpp>
#include <mongocxx/database.hpp>

namespace http = boost::beast::http;

// Define a more concise alias for commonly used request and response global 
using HttpReq = http::request<http::string_body>&;
using HttpRes = http::response<http::string_body>&;
using MongoDB = mongocxx::database&;
