#include <iostream>
#include "external/cpp-httplib-0.15.3/httplib.h"
#include "external/sqlite-amalgamation-3510200/sqlite3.h"
#include "external/json.hpp"
#include "sub pages/account.h"
#include "sub pages/admin.h"
#include "sub pages/calender.h"
#include "sub pages/join_queue.h"
#include "sub pages/queues.h"

int main() {
    httplib::Server server;

    // Handler functions
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
        if (currentUserId != 0) {
            std::cout << "User " << getUsernameById(currentUserId) << " (ID: " << currentUserId << ") logged out" << std::endl;
            currentUserId = 0;
            res.set_redirect("/");
        } else {
            res.set_content(loginPage(), "text/html");
        }
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

    server.Get("/api/check-login", [](const httplib::Request&, httplib::Response& res) {
        nlohmann::json response;
        response["loggedIn"] = (currentUserId != 0);
        if (currentUserId != 0) {
            response["username"] = getUsernameById(currentUserId);
            response["userId"] = currentUserId;
            response["authLevel"] = getAuthLevelById(currentUserId);
        }
        res.set_content(response.dump(), "application/json");
    });

    server.Post("/api/login", [](const httplib::Request& req, httplib::Response& res) {
        auto json = nlohmann::json::parse(req.body);
        std::string username = json["username"];
        std::string password = json["password"];
        LoginResult result = checkCredentials(username, password);
        nlohmann::json response;
        response["success"] = result.success;
        if (result.success) {
            response["userId"] = result.userId;
        }
        res.set_content(response.dump(), "application/json");
    });

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
                    
                    .dropdown.hidden {
                        display: none;
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
                    
                    /* User info display */
                    .user-info {
                        margin-left: auto;
                        color: #4CAF50;
                        padding: 10px 20px;
                        background: #1e1e2f;
                        border-radius: 6px;
                        font-weight: bold;
                        display: none;
                    }
                    
                    .user-info.show {
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
                    
                    <!-- Admin Dropdown (hidden by default) -->
                    <div class="dropdown hidden" id="adminDropdown">
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
                            <a href="/login" id="loginLogoutLink">Login / Logout</a>
                            <a href="/account-settings">Account Settings</a>
                        </div>
                    </div>
                    
                    <!-- User Info Display -->
                    <div class="user-info" id="userInfo">
                        <span id="username"></span>
                    </div>
                </nav>
                
                <!-- Main Content -->
                <div class="content">
                    <div class="card">
                        <h1>Connected</h1>
                        <p>Hello! You successfully connected to the localhost server.</p>
                    </div>
                </div>
                
                <script>
                    // Check login status when page loads
                    async function checkLoginStatus() {
                        try {
                            const response = await fetch('/api/check-login');
                            const data = await response.json();
                            
                            const userInfoDiv = document.getElementById('userInfo');
                            const usernameSpan = document.getElementById('username');
                            const loginLogoutLink = document.getElementById('loginLogoutLink');
                            const adminDropdown = document.getElementById('adminDropdown');
                            
                            if (data.loggedIn) {
                                // User is logged in
                                usernameSpan.textContent = data.username;
                                userInfoDiv.classList.add('show');
                                loginLogoutLink.textContent = 'Logout';
                                
                                // Show admin dropdown if auth level is 2 or higher
                                if (data.authLevel >= 2) {
                                    adminDropdown.classList.remove('hidden');
                                } else {
                                    adminDropdown.classList.add('hidden');
                                }
                            } else {
                                // User is not logged in
                                userInfoDiv.classList.remove('show');
                                loginLogoutLink.textContent = 'Login';
                                adminDropdown.classList.add('hidden');
                            }
                        } catch (error) {
                            console.error('Error checking login status:', error);
                        }
                    }
                    
                    // Check login status on page load
                    checkLoginStatus();
                </script>
            </body>
            </html>
        )";

        res.set_content(html, "text/html");
    });

    std::cout << "Server running at http://localhost:8080\n";
    server.listen("127.0.0.1", 8080);
}