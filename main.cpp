#include <iostream>
#include "external/cpp-httplib-0.15.3/httplib.h"
#include "external/json.hpp"
#include "sub pages/account.h"
#include "sub pages/admin.h"
#include "sub pages/calender.h"
#include "sub pages/join_queue.h"
#include "sub pages/queues.h"

int main() {
    httplib::Server server;

    // Blank handler functions
    auto handleCalendar = [](const httplib::Request&, httplib::Response& res) {
        res.set_content(calenderPageData(), "text/html");
    };

    auto handleActiveQueues = [](const httplib::Request&, httplib::Response& res) {
        res.set_content("Active Queues page - Coming soon!", "text/plain");
    };

    auto handleJoinQueue = [](const httplib::Request&, httplib::Response& res) {
        res.set_content("Join Queue page - Coming soon!", "text/plain");
    };

    auto handleAnalytics = [](const httplib::Request&, httplib::Response& res) {
        res.set_content("Analytics page - Coming soon!", "text/plain");
    };

    auto handleEditData = [](const httplib::Request&, httplib::Response& res) {
        res.set_content("Edit Data page - Coming soon!", "text/plain");
    };

    auto handleLogin = [](const httplib::Request&, httplib::Response& res) {
        res.set_content(loginPage(), "text/html");
    };

    auto handleAccountSettings = [](const httplib::Request&, httplib::Response& res) {
        res.set_content("Account Settings page - Coming soon!", "text/plain");
    };

    // Register routes
    server.Get("/calendar", handleCalendar);
    server.Get("/active-queues", handleActiveQueues);
    server.Get("/join-queue", handleJoinQueue);
    server.Get("/analytics", handleAnalytics);
    server.Get("/edit-data", handleEditData);
    server.Get("/login", handleLogin);
    server.Get("/account-settings", handleAccountSettings);

    server.Get("/", [](const httplib::Request&, httplib::Response& res) {
        std::string html = R"(
            <!DOCTYPE html>
            <html>
            <head>
                <title>Localhost Server</title>
                <style>
                    * {
                        margin: 0;
                        padding: 0;
                        box-sizing: border-box;
                    }
                    
                    body {
                        font-family: Arial, sans-serif;
                        background-color: #1e1e2f;
                        color: white;
                    }
                    
                    /* Navigation Bar */
                    .navbar {
                        background: #2b2b40;
                        padding: 15px 30px;
                        box-shadow: 0 2px 10px rgba(0,0,0,0.3);
                        display: flex;
                        align-items: center;
                        gap: 20px;
                    }
                    
                    .navbar a {
                        color: white;
                        text-decoration: none;
                        padding: 10px 20px;
                        border-radius: 6px;
                        transition: background 0.3s;
                    }
                    
                    .navbar a:hover {
                        background: #3a3a52;
                    }
                    
                    /* Dropdown Container */
                    .dropdown {
                        position: relative;
                        display: inline-block;
                    }
                    
                    .dropdown-button {
                        color: white;
                        padding: 10px 20px;
                        border: none;
                        background: transparent;
                        cursor: pointer;
                        border-radius: 6px;
                        transition: background 0.3s;
                        font-size: 16px;
                    }
                    
                    .dropdown-button:hover {
                        background: #3a3a52;
                    }
                    
                    .dropdown-content {
                        display: none;
                        position: absolute;
                        background-color: #2b2b40;
                        min-width: 180px;
                        box-shadow: 0 8px 16px rgba(0,0,0,0.4);
                        border-radius: 6px;
                        z-index: 1;
                        margin-top: 0px;  /* Changed from 5px to 0px */
                        padding-top: 5px; /* Add padding instead to maintain visual spacing */
                    }
                    
                    .dropdown-content a {
                        color: white;
                        padding: 12px 16px;
                        text-decoration: none;
                        display: block;
                        transition: background 0.3s;
                    }
                    
                    .dropdown-content a:first-child {
                        border-radius: 6px 6px 0 0;
                    }
                    
                    .dropdown-content a:last-child {
                        border-radius: 0 0 6px 6px;
                    }
                    
                    .dropdown-content a:hover {
                        background: #3a3a52;
                    }
                    
                    .dropdown:hover .dropdown-content {
                        display: block;
                    }
                    
                    /* Main Content */
                    .content {
                        display: flex;
                        justify-content: center;
                        align-items: center;
                        height: calc(100vh - 60px);
                    }
                    
                    .card {
                        background: #2b2b40;
                        padding: 40px;
                        border-radius: 12px;
                        box-shadow: 0 0 20px rgba(0,0,0,0.4);
                        text-align: center;
                    }
                    
                    h1 { 
                        color: #4CAF50;
                        margin-bottom: 15px;
                    }
                </style>
            </head>
            <body>
                <!-- Navigation Bar -->
                <nav class="navbar">
                    <a href="/calendar">Calendar</a>
                    <a href="/active-queues">Active Queues</a>
                    <a href="/join-queue">Join a Queue</a>
                    
                    <!-- Admin Dropdown -->
                    <div class="dropdown">
                        <button class="dropdown-button">Admin</button>
                        <div class="dropdown-content">
                            <a href="/analytics">Analytics</a>
                            <a href="/edit-data">Edit Data</a>
                        </div>
                    </div>
                    
                    <!-- Account Dropdown -->
                    <div class="dropdown">
                        <button class="dropdown-button">Account</button>
                        <div class="dropdown-content">
                            <a href="/login">Login / Logout</a>
                            <a href="/account-settings">Account Settings</a>
                        </div>
                    </div>
                </nav>
                
                <!-- Main Content -->
                <div class="content">
                    <div class="card">
                        <h1>Connected</h1>
                        <p>Hello! You successfully connected to the localhost server.</p>
                    </div>
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