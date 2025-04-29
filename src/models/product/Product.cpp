#include "models/product/Product.hpp"
#include <bsoncxx/builder/stream/document.hpp>
#include <bsoncxx/builder/stream/array.hpp>
#include <bsoncxx/types.hpp>

using namespace bsoncxx::builder::stream;

Product::Product(const std::string &title, double price, int stock,
                 const std::string &description, const std::string &image,
                 const std::vector<std::string> &multiimage)
    : title(title), price(price), stock(stock), description(description),
      image(image), multiimage(multiimage) {}

bsoncxx::document::value Product::toBson() const
{
    array multiimage_array;
    for (const auto &img : multiimage)
    {
        multiimage_array << img;
    }

    document doc{};
    doc << "title" << title
        << "price" << price
        << "stock" << stock
        << "description" << description
        << "image" << image
        << "multiimage" << multiimage_array;
    return doc << finalize;
}

Product Product::fromBson(const bsoncxx::document::view &doc)
{
    std::string title = doc["title"].get_utf8().value.to_string();
    double price = doc["price"].get_double().value;
    int stock = doc["stock"].get_int32().value;
    std::string description = doc["description"].get_utf8().value.to_string();
    std::string image = doc["image"].get_utf8().value.to_string();

    std::vector<std::string> multiimage;
    auto multiimage_array = doc["multiimage"].get_array().value;
    for (const auto &item : multiimage_array)
    {
        multiimage.push_back(item.get_utf8().value.to_string());
    }

    Product product(title, price, stock, description, image, multiimage);
    if (doc["_id"])
    {
        product.id = doc["_id"].get_oid().value.to_string();
    }
    return product;
}
