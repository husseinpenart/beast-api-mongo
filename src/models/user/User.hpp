#pragma once
#include <string>
#include <bsoncxx/builder/stream/document.hpp>
#include <bsoncxx/json.hpp>

class User {
public:
    std::string id; // MongoDB _id (ObjectId)
    std::string name;
    std::string username;
    std::string email;
    std::string password; // Hashed password
    std::string phone;

    User(const std::string& name, const std::string& username, const std::string& email,
         const std::string& password, const std::string& phone = "");

    // Convert User to BSON for MongoDB
    bsoncxx::document::value toBson() const;

    // Create User from BSON document
    static User fromBson(const bsoncxx::document::view& doc);
};