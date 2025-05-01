#pragma once
#include <string>

class Base {
public:
    static std::string get_upload_path() {
        return "../../Uploads/";
    }
};