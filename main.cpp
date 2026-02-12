#include <iostream>
#include "cpp-httplib-0.15.3/httplib.h"

int main() {
    httplib::Server server;

    server.Get("/", [](const httplib::Request&, httplib::Response& res) {
        std::string html = R"(
            <!DOCTYPE html>
            <html>
            <head>
                <title>Localhost Server</title>
                <style>
                    body {
                        font-family: Arial, sans-serif;
                        background-color: #1e1e2f;
                        color: white;
                        display: flex;
                        justify-content: center;
                        align-items: center;
                        height: 100vh;
                        margin: 0;
                    }
                    .card {
                        background: #2b2b40;
                        padding: 40px;
                        border-radius: 12px;
                        box-shadow: 0 0 20px rgba(0,0,0,0.4);
                        text-align: center;
                    }
                    h1 { color: #4CAF50; }
                </style>
            </head>
            <body>
                <div class="card">
                    <h1>Connected</h1>
                    <p>Hello! You successfully connected to the localhost server.</p>
                </div>
            </body>
            </html>
        )";

        res.set_content(html, "text/html");
        std::cout << "A user connected!" << std::endl;
    });

    std::cout << "Server running at http://localhost:8080\n";
    server.listen("127.0.0.1", 8080);
}