#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <mongocxx/client.hpp>
#include <mongocxx/instance.hpp>
#include <mongocxx/uri.hpp>
#include <iostream>
#include "routes/mainRouter/Router.hpp"
#include "utils/Base.hpp"

namespace beast = boost::beast;
namespace http = beast::http;
namespace net = boost::asio;
using tcp = net::ip::tcp;

// Global MongoDB setup
mongocxx::instance inst{};              
mongocxx::client client{mongocxx::uri{}}; 
auto db = client["store"]; 

void handle_connection(tcp::socket socket, mongocxx::database &db)
{
    try
    {
        beast::flat_buffer buffer;
        http::request<http::string_body> req;
        http::read(socket, buffer, req);
        std::cout << "Got request: " << req.method_string() << " " << req.target() << "\n";

        http::response<http::string_body> res{http::status::ok, req.version()};
        res.set(http::field::server, "BeastAPI");

        // Pass request to Router
        Router::route(req, res, db);

        res.prepare_payload();
        http::write(socket, res);

        beast::error_code ec;
        socket.shutdown(tcp::socket::shutdown_both, ec);
    }
    catch (const std::exception &e)
    {
        std::cerr << "Connection error: " << e.what() << "\n";
    }
}

int main()
{
    try
    {
        net::io_context ioc;
        tcp::acceptor acceptor(ioc, tcp::endpoint(tcp::v4(), 8081));
        std::cout << "Server running on http://localhost:8081\n";

        while (true)
        {
            tcp::socket socket(ioc);
            acceptor.accept(socket);
            handle_connection(std::move(socket), db);
        }
    }
    catch (const std::exception &e)
    {
        std::cerr << "Server error: " << e.what() << "\n";
        return 1;
    }
    return 0;
}