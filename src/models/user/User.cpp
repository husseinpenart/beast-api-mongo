#include "User.hpp"

User::User(const std::string& name, const std::string& username, const std::string& email,
           const std::string& password, const std::string& phone)
    : name(name), username(username), email(email), password(password), phone(phone) {}

bsoncxx::document::value User::toBson() const {
    return bsoncxx::builder::stream::document{}
        << "name" << name
        << "username" << username
        << "email" << email
        << "password" << password
        << "phone" << phone
        << bsoncxx::builder::stream::finalize;
}

User User::fromBson(const bsoncxx::document::view& doc) {
    User user(
        doc["name"].get_utf8().value.to_string(),
        doc["username"].get_utf8().value.to_string(),
        doc["email"].get_utf8().value.to_string(),
        doc["password"].get_utf8().value.to_string(),
        doc["phone"] ? doc["phone"].get_utf8().value.to_string() : ""
    );
    user.id = doc["_id"].get_oid().value.to_string();
    return user;
}