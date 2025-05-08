#include "blogModel.hpp"
#include <bsoncxx/builder/stream/document.hpp>
#include <bsoncxx/builder/stream/array.hpp>
#include <bsoncxx/types.hpp>
using namespace bsoncxx::builder::stream;
using namespace std;
blogModel::blogModel(const string &title, const string &category, const string &shortDesc, const string &description, const string &image)
    : title(title), category(category), shortDesc(shortDesc), description(description), image(image) {}

/// method converts a blogModel C++ object into a BSON document (used for MongoDB).
bsoncxx::document::value blogModel::toBson() const
{
    document doc{};
    doc << "title" << title
        << "category" << category
        << "shortDesc" << shortDesc
        << "description" << description
        << "image" << image;
    return doc << finalize;
}
// This static method creates a blogModel object from a BSON document retrieved from MongoDB.
blogModel blogModel::fromBson(const bsoncxx::document::view &doc)
{
    string title = doc["title"].get_utf8().value.to_string();
    string category = doc["category"].get_utf8().value.to_string();
    string shortDesc = doc["shortDesc"].get_utf8().value.to_string();
    string description = doc["description"].get_utf8().value.to_string();
    string image = doc["image"].get_utf8().value.to_string();

    blogModel blog(title, category, shortDesc, description, image);
    if (doc["_id"])
    {
        blog.id = doc["_id"].get_oid().value.to_string();
    }
    return blog;
}