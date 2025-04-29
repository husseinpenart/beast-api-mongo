#pragma once
#include <mongocxx/database.hpp>

class Base {
protected:
    static std::string get_upload_path() {
        return "uploads/"; 
    }
};