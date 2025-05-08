#pragma once
#include "helper/types.hpp" // Include  alias definitions
#include <iostream>
class ProductController
{
public:
    static void create(HttpReq req, HttpRes res, MongoDB db);
    static void read(HttpReq req, HttpRes res, MongoDB db);
    static void update(HttpReq req, HttpRes res, MongoDB db, const std::string &id);
    static void del(HttpReq req, HttpRes res, MongoDB db, const std::string &id);
    static void getByid(HttpReq req, HttpRes res, MongoDB db, const std::string &id);
};
