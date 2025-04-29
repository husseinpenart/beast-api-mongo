#pragma once

#include <string>
#include <vector>
#include <bsoncxx/document/value.hpp>
#include <bsoncxx/document/view.hpp>

class Product
{
public:
    std::string id; // MongoDB ObjectID
    std::string title;
    double price;
    int stock;
    std::string description;
    std::string image;                   // Path to single image
    std::vector<std::string> multiimage; // Paths to multiple images

    // Constructor
    Product(const std::string &title, double price, int stock,
            const std::string &description, const std::string &image,
            const std::vector<std::string> &multiimage);

    // Methods
    bsoncxx::document::value toBson() const;
    static Product fromBson(const bsoncxx::document::view &doc);
};
