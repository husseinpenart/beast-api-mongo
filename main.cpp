// #include <boost/beast/core.hpp>
// #include <boost/beast/http.hpp>
// #include <boost/beast/version.hpp>
// #include <boost/asio/ip/tcp.hpp>
// #include <boost/asio/strand.hpp>
// #include <nlohmann/json.hpp>
// #include <iostream>
// #include <string>
// #include <thread>

// namespace beast = boost::beast;
// namespace http = beast::http;
// namespace net = boost::asio;
// using tcp = boost::asio::ip::tcp;
// using json = nlohmann::json;

// void handle_request(http::request<http::string_body> req, http::response<http::string_body>& res) {
//     if (req.target() == "/hello" && req.method() == http::verb::get) {
//         json j;
//         j["message"] = "Hello from C++ Beast API!";
//         res.result(http::status::ok);
//         res.set(http::field::content_type, "application/json");
//         res.body() = j.dump();
//         res.prepare_payload();
//     }
//     else {
//         res.result(http::status::not_found);
//         res.set(http::field::content_type, "text/plain");
//         res.body() = "Not Found";
//         res.prepare_payload();
//     }
// }

// void do_session(tcp::socket socket) {
//     beast::flat_buffer buffer;
//     http::request<http::string_body> req;
//     http::read(socket, buffer, req);

//     http::response<http::string_body> res;
//     handle_request(std::move(req), res);

//     http::write(socket, res);
// }

// int main() {
//     try {
//         net::io_context ioc{ 1 };
//         tcp::acceptor acceptor{ ioc, {tcp::v4(), 8080} };
//         std::cout << "Server running at http://localhost:8080\n";

//         for (;;) {
//             tcp::socket socket{ ioc };
//             acceptor.accept(socket);

//             // Use lambda to pass socket to do_session
//             std::thread{ [socket = std::move(socket)]() mutable {
//                 do_session(std::move(socket));
//             } }.detach();
//         }

//     }
//     catch (std::exception const& e) {
//         std::cerr << "Error: " << e.what() << "\n";
//         return EXIT_FAILURE;
//     }
// }
#include <bsoncxx/json.hpp>
#include <mongocxx/client.hpp>
#include <mongocxx/instance.hpp>
#include <mongocxx/uri.hpp>
#include <vector>
int main()
{
    mongocxx::instance instance{};
    mongocxx::client client{mongocxx::uri{}};
    auto db = client["test"];
    std::cout << "Collections in 'test' database:\n";
    for (auto&& name : db.list_collection_names()) {
        std::cout << "- " << name << "\n";
    }

}