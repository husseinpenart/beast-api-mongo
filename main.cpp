// Include these at the top if not already
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <iostream>

namespace beast = boost::beast;
namespace http = beast::http;
namespace net = boost::asio;
using tcp = net::ip::tcp;

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

        std::cout << "Received request:\n"
                  << req << "\n";

        http::response<http::string_body> res{http::status::ok, req.version()};
        res.set(http::field::server, "Beast");
        res.set(http::field::content_type, "text/plain");

        // Handle routes
        if (req.target() == "/hello")
        {
            res.body() = "Hello World!";
        }
        else if (req.target() == "/json")
        {
            res.set(http::field::content_type, "application/json");
            res.body() = R"({"message": "This is JSON"})";
        }
        else
        {
            res.result(http::status::not_found);
            res.body() = "404 Not Found";
        }
        if(req.method()  == http::verb::post && req.target()=="/submit"){
            std::string body  = req.body();
            std::cout  << "Recieved post method" << body << std::endl;
            http::response<http::string_body> res{http::status::ok, req.version()};
            res.set(http::field::content_type, "text/plain");
            res.body() = "Received: " + body;
            res.prepare_payload();
            http::write(socket, res);
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
