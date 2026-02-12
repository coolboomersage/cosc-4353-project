#include <iostream>
#include "cpp-httplib-0.15.3/httplib.h"

int main() {
    httplib::Server server;

    // When a user connects to localhost:8080
    server.Get("/", [](const httplib::Request&, httplib::Response& res) {
        res.set_content("Hello! You successfully connected to the localhost server.", "text/plain");
        std::cout << "A user connected!" << std::endl;
    });

    std::cout << "Server running at http://localhost:8080\n";

    server.listen("127.0.0.1", 8080);
}