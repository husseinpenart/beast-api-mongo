#include <iostream>
#include <string> 
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <bsoncxx/json.hpp>
#include <mongocxx/client.hpp>
#include <mongocxx/instance.hpp>
#include <mongocxx/uri.hpp>

namespace beast = boost::beast;
namespace http = beast::http;
namespace net = boost::asio;
using tcp = net::ip::tcp;

// MongoDB instance and client setup
mongocxx::instance inst{};
mongocxx::client client{mongocxx::uri{}};
auto db = client["test"]; // this is name of you db in mongodb

int main()
{
    try
    {
        net::io_context ioc;
        tcp::acceptor acceptor(ioc, tcp::endpoint(tcp::v4(), 8081));
        std::cout << "Waiting for connection on port 8081...\n";

        tcp::socket socket(ioc);
        acceptor.accept(socket);
        std::cout << "Client connected!\n";

        beast::flat_buffer buffer;
        http::request<http::string_body> req;
        http::read(socket, buffer, req);
        std::string path = std::string(req.target());

        std::cout << "Received request:\n"
                  << req << "\n";

        http::response<http::string_body> res{http::status::ok, req.version()};
        res.set(http::field::server, "Beast");
        res.set(http::field::content_type, "text/plain");

        if (req.method() == http::verb::post && req.target() == "/submit")
        {
            std::string body = req.body();
            std::cout << "Received POST /submit: " << body << std::endl;

            res.set(http::field::content_type, "text/plain");
            res.body() = "Received: " + body;
        }
        // insert any string or any type to db 
        else if (req.method() == http::verb::post && req.target() == "/product")
        {
            std::string body = req.body();
            std::cout << "Received POST /product: " << body << std::endl;

            bsoncxx::document::value doc = bsoncxx::from_json(body);
            auto collection = db["products"];
            auto result = collection.insert_one(doc.view());
            std::string insertedId = result->inserted_id().get_oid().value.to_string();

            res.set(http::field::content_type, "application/json");
            res.body() = R"({"message": "Inserted", "id": ")" + insertedId + R"("})";
        }
        // get data from db
        else if (req.method() == http::verb::get && req.target() == "/products")
        {
            auto collection = db["products"];
            mongocxx::cursor cursor = collection.find({});
            std::string json = "[";
            for (auto &&doc : cursor)
            {
                json += bsoncxx::to_json(doc) + ",";
            }
            if (json.back() == ',')
                json.pop_back(); // Remove last comma
            json += "]";

            res.set(http::field::content_type, "application/json");
            res.body() = json;
        }
        // get product by id 
        else if (req.method() == http::verb::get && path.rfind("/product/", 0) == 0)
        {
            // Extract ID from path
    std::string idStr = path.substr(std::string("/product/").length());

    try {
        bsoncxx::oid id(idStr); // Convert to MongoDB ObjectId
        auto collection = db["products"];

        bsoncxx::builder::stream::document filter_builder;
        filter_builder << "_id" << id;

        auto product = collection.find_one(filter_builder.view());

        if (product) {
            std::string json = bsoncxx::to_json(*product);
            http::response<http::string_body> res{http::status::ok, req.version()};
            res.set(http::field::content_type, "application/json");
            res.body() = json;
            res.prepare_payload();
            http::write(socket, res);
        } else {
            http::response<http::string_body> res{http::status::not_found, req.version()};
            res.set(http::field::content_type, "text/plain");
            res.body() = "Product not found.";
            res.prepare_payload();
            http::write(socket, res);
        }
    } catch (const std::exception& e) {
        http::response<http::string_body> res{http::status::bad_request, req.version()};
        res.set(http::field::content_type, "text/plain");
        res.body() = "Invalid ID format.";
        res.prepare_payload();
        http::write(socket, res);
    }
        }
        else
        {
            res.body() = "Hello Beast!";
        }

        res.prepare_payload();
        http::write(socket, res);

        std::cout << "Response sent. Press Enter to exit.\n";
        std::cin.get();
    }
    catch (const std::exception &e)
    {
        std::cerr << "Error: " << e.what() << "\n";
        std::cin.get();
    }

    return 0;
}
