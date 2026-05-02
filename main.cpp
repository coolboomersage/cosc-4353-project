#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include "external/cpp-httplib-0.15.3/httplib.h"
#include "external/sqlite-amalgamation-3510200/sqlite3.h"
#include "external/json.hpp"
#include "sub pages/account.h"
#include "sub pages/admin.h"
#include "sub pages/calender.h"
#include "sub pages/join_queue.h"
#include "sub pages/queues.h"
#include "sub pages/user_dashboard.h"
#include "sub pages/service_management.h"
#include "sub pages/queue_management.h"
#include "sub pages/export report.h"

inline void addTests(std::vector<UnitTestBase*> &tests){
    tests.push_back(new UnitTest<bool, std::string, 1>(
        "email pass",
        [](std::array<std::string, 1> args){ return isValidEmail(args[0]); },
        {"cbs@abc.cc"},
        true
    ));

    tests.push_back(new UnitTest<bool, std::string, 1>(
    "email fail no at",
    [](std::array<std::string, 1> args){ return isValidEmail(args[0]); },
    {"cbsabc.cc"},
    false
    ));

    tests.push_back(new UnitTest<bool, std::string, 1>(
        "email fail no suffix",
        [](std::array<std::string, 1> args){ return isValidEmail(args[0]); },
        {"cbs@abc"},
        false
    ));

    tests.push_back(new UnitTest<bool, std::string, 1>(
        "email fail spaces",
        [](std::array<std::string, 1> args){ return isValidEmail(args[0]); },
        {"cbs @abc.cc"},
        false
    ));

    tests.push_back(new UnitTest<bool, std::string, 1>(
        "email pass subdomain",
        [](std::array<std::string, 1> args){ return isValidEmail(args[0]); },
        {"me@test.school.edu"},
        true
    ));

    tests.push_back(new UnitTest<bool, std::string, 1>(
            "email pass",
            [](std::array<std::string, 1> args){ return isValidEmail(args[0]); },
            {"cbs@abc.cc"},
            true
        ));

    // jsonEscape tests
    tests.push_back(new UnitTest<std::string, std::string, 1>(
        "jsonEscape plain string",
        [](std::array<std::string, 1> args){ return jsonEscape(args[0]); },
        {"hello world"},
        "hello world"
    ));

    tests.push_back(new UnitTest<std::string, std::string, 1>(
        "jsonEscape double quote",
        [](std::array<std::string, 1> args){ return jsonEscape(args[0]); },
        {"say \"hi\""},
        "say \\\"hi\\\""
    ));

    tests.push_back(new UnitTest<std::string, std::string, 1>(
        "jsonEscape backslash",
        [](std::array<std::string, 1> args){ return jsonEscape(args[0]); },
        {"C:\\Users\\foo"},
        "C:\\\\Users\\\\foo"
    ));

    tests.push_back(new UnitTest<std::string, std::string, 1>(
        "jsonEscape newline",
        [](std::array<std::string, 1> args){ return jsonEscape(args[0]); },
        {"line1\nline2"},
        "line1\\nline2"
    ));

    tests.push_back(new UnitTest<std::string, std::string, 1>(
        "jsonEscape carriage return",
        [](std::array<std::string, 1> args){ return jsonEscape(args[0]); },
        {"line1\rline2"},
        "line1\\rline2"
    ));

    tests.push_back(new UnitTest<std::string, std::string, 1>(
        "jsonEscape tab",
        [](std::array<std::string, 1> args){ return jsonEscape(args[0]); },
        {"col1\tcol2"},
        "col1\\tcol2"
    ));

    tests.push_back(new UnitTest<std::string, std::string, 1>(
        "jsonEscape empty string",
        [](std::array<std::string, 1> args){ return jsonEscape(args[0]); },
        {""},
        ""
    ));

    tests.push_back(new UnitTest<std::string, std::string, 1>(
        "jsonEscape mixed specials",
        [](std::array<std::string, 1> args){ return jsonEscape(args[0]); },
        {"a\"b\\c\nd"},
        "a\\\"b\\\\c\\nd"
    ));

    tests.push_back(new UnitTest<std::string, int, 1>(
    "getUsernameById - userId 1 returns root",
    [](std::array<int, 1> args){ return getUsernameById(args[0]); },
    {1},
    "root"
    ));

    tests.push_back(new UnitTest<int, int, 1>(
        "getAuthLevelById - userId 1 returns 3",
        [](std::array<int, 1> args){ return getAuthLevelById(args[0]); },
        {1},
        3
    ));
}

int main() {
    initCoutCapture(); // start cout capture for admin panel
    httplib::Server server;

    sqlite3* db = nullptr;
    std::string dbPath = DATABASE_FILE_LOCATION;
    std::cout << "Attempting to open database at: " << dbPath << std::endl;

    bool dbExists = std::ifstream(dbPath).good();

    if (sqlite3_open(dbPath.c_str(), &db) != SQLITE_OK) {
        std::cout << "Failed to open database: " << sqlite3_errmsg(db) << std::endl;
        sqlite3_close(db);
        return -1;
}

    if (!dbExists) {
        std::cout << "No database found, creating default" << std::endl;
        if (!initDatabase(db)) {
            std::cout << "Failed to initialize database." << std::endl;
        }
    } else {
        std::cout << "Pre-existing database found" << std::endl;
    }

    //unitTest vector defined for use in the admin panel for unit testing
    std::vector<UnitTestBase*> tests;
    addTests(tests);

    // Handler functions
    auto handleCalendar = [](const httplib::Request&, httplib::Response& res) {
        res.set_content(calenderPageData(), "text/html");
    };

    
    auto handleActiveQueues = [](const httplib::Request&, httplib::Response& res) {
    res.set_content(activeQueuesPage(), "text/html");
};

    auto handleJoinQueue = [](const httplib::Request&, httplib::Response& res) {
        res.set_content(joinQueuePage(), "text/html");
    };


    auto handleAnalytics = [](const httplib::Request&, httplib::Response& res) {
    res.set_content(R"HTML(
<!doctype html>
<html>
<head>
  <meta charset="utf-8">
  <title>Queue Analytics</title>
  <style>
    body {
      font-family: Arial, sans-serif;
      margin: 0;
      background: #1e1e2f;
      color: #f4f4f8;
    }

    .container {
      max-width: 900px;
      margin: 60px auto;
      padding: 24px;
    }

    .card {
      background: #302f45;
      color: #f4f4f8;
      padding: 24px;
      border-radius: 14px;
      border: 1px solid #46455f;
      box-shadow: 0 8px 20px rgba(0,0,0,0.35);
      margin-bottom: 16px;
    }

    h1 {
      margin-top: 0;
      color: #f4f4f8;
    }

    p, div {
      color: #f4f4f8;
    }

    .grid {
      display: grid;
      grid-template-columns: repeat(3, 1fr);
      gap: 16px;
    }

    .big {
      font-size: 28px;
      font-weight: 700;
      color: #4ade80;
    }

    a {
      color: #f4f4f8;
      font-weight: 700;
    }

    a:hover {
      color: #4ade80;
    }
  </style>
</head>
<body>
  <div class="container">
    <p><a href="/admin-dashboard">← Back to Admin Dashboard</a></p>
    <div class="card">
      <h1>Queue Analytics</h1>
      <p>This page summarizes queue usage patterns for administrators.</p>
    </div>
    <div class="grid">
      <div class="card"><div>Total Active Queues</div><div class="big">3</div></div>
      <div class="card"><div>People Waiting</div><div class="big">11</div></div>
      <div class="card"><div>Average Wait</div><div class="big">~50 min</div></div>
    </div>
  </div>
</body>
</html>
)HTML", "text/html");
};



    auto handleEditData = [](const httplib::Request&, httplib::Response& res) {
    res.set_content(R"HTML(
<!doctype html>
<html>
<head>
  <meta charset="utf-8">
  <title>Edit Queue Data</title>
  <style>
    body {
      font-family: Arial, sans-serif;
      margin: 0;
      background: #1e1e2f;
      color: #f4f4f8;
    }

    .container {
      max-width: 900px;
      margin: 60px auto;
      padding: 24px;
    }

    .card {
      background: #302f45;
      color: #f4f4f8;
      padding: 24px;
      border-radius: 14px;
      border: 1px solid #46455f;
      box-shadow: 0 8px 20px rgba(0,0,0,0.35);
    }

    h1, p {
      color: #f4f4f8;
    }

    table {
      width: 100%;
      border-collapse: collapse;
      margin-top: 16px;
      color: #f4f4f8;
    }

    th, td {
      padding: 12px;
      border-bottom: 1px solid #46455f;
      text-align: left;
      color: #f4f4f8;
    }

    th {
      color: #ffffff;
      font-weight: 700;
    }

    td {
      color: #f4f4f8;
    }

    tr:hover {
      background: rgba(255,255,255,0.04);
    }

    a {
      color: #f4f4f8;
      font-weight: 700;
    }

    a:hover {
      color: #4ade80;
    }
  </style>
</head>
<body>
  <div class="container">
    <p><a href="/admin-dashboard">← Back to Admin Dashboard</a></p>
    <div class="card">
      <h1>Edit Queue Data</h1>
      <p>Administrators can review service records and queue information from this page.</p>
      <table>
        <thead>
          <tr><th>Service</th><th>Estimated Service Time</th><th>Status</th></tr>
        </thead>
        <tbody>
          <tr><td>Advising</td><td>20 min</td><td>Active</td></tr>
          <tr><td>Tutoring</td><td>30 min</td><td>Active</td></tr>
          <tr><td>Tech Support</td><td>15 min</td><td>Active</td></tr>
        </tbody>
      </table>
    </div>
  </div>
</body>
</html>
)HTML", "text/html");
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
    res.set_content(R"HTML(
<!doctype html>
<html>
<head>
  <meta charset="utf-8">
  <title>Account Settings</title>
  <style>
    body { font-family: Arial, sans-serif; margin: 0; background: #1f1f33; color: #f5f5f5; }
    .container { max-width: 700px; margin: 60px auto; padding: 24px; }
    .card { background: #302f49; padding: 24px; border-radius: 14px; box-shadow: 0 4px 12px rgba(0,0,0,0.25); }
    h1 { color: #4ade80; }
    .muted { color: #b7b7cc; }
    a { color: #4ade80; font-weight: 700; }
  </style>
</head>
<body>
  <div class="container">
    <p><a href="/dashboard">← Back to Dashboard</a></p>
    <div class="card">
      <h1>Account Settings</h1>
      <p class="muted">Signed-in users can view their account role and queue activity from the dashboard.</p>
      <p>This page supports the account management part of QueueSmart and provides a place for future profile updates.</p>
    </div>
  </div>
</body>
</html>
)HTML", "text/html");
};

    // Register routes
    server.Get("/calendar", handleCalendar);
    server.Get("/active-queues", handleActiveQueues);
    server.Get("/join-queue", handleJoinQueue);
    server.Get("/analytics", handleAnalytics);
    server.Get("/edit-data", handleEditData);
    server.Get("/login", handleLogin);
    server.Get("/account-settings", handleAccountSettings);

    
    server.Get("/dashboard", [](const httplib::Request&, httplib::Response& res) {
    if (currentUserId == 0) {
        res.set_redirect("/login");
        return;
    }
    res.set_content(userDashboardPage(getUsernameById(currentUserId)), "text/html");
});

    server.Get("/admin/create-queue", [](const httplib::Request& req, httplib::Response& res) {
        std::string username = "admin"; // placeholder

        res.set_content(createQueuePage(username), "text/html");
    });

    
    server.Get("/admin-dashboard", [](const httplib::Request&, httplib::Response& res) {
    if (currentUserId == 0) {
        res.set_redirect("/login");
        return;
    }
    // homepage shows admin dropdown if authLevel >= 2,
    // so match that rule here:
    if (getAuthLevelById(currentUserId) < 2) {
        res.status = 403;
        res.set_content("Forbidden: Admins only", "text/plain");
        return;
    }

    res.set_content(adminDashboardPage(getUsernameById(currentUserId)), "text/html");
});


    server.Get("/create-account", [](const httplib::Request&, httplib::Response& res) {
        if (currentUserId != 0) {
            res.set_redirect("/");
        } else {
            res.set_content(createAccountPage(), "text/html");
        }
    });

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

    server.Get("/api/services", [&](const httplib::Request&, httplib::Response& res) {
        auto services = getAllServices(db);
        res.set_content(servicesToJson(services).dump(), "application/json");
    });
    
    server.Post("/api/services/add", [&](const httplib::Request& req, httplib::Response& res) {
    auto json = nlohmann::json::parse(req.body);
    std::string message;
    bool success = addService(
        db,
        json["name"],
        json["description"],
        json["estimatedServiceTime"],
        json["priority"],
        message
    );

    nlohmann::json response;
    response["success"] = success;
    response["message"] = message;
    res.set_content(response.dump(), "application/json");
});
    
    server.Post("/api/services/update", [&](const httplib::Request& req, httplib::Response& res) {
    auto json = nlohmann::json::parse(req.body);
    std::string message;
    bool success = updateService(
        db,
        json["id"],
        json["name"],
        json["description"],
        json["estimatedServiceTime"],
        json["priority"],
        message
    );

    nlohmann::json response;
    response["success"] = success;
    response["message"] = message;
    res.set_content(response.dump(), "application/json");
});
        
    server.Post("/api/services/delete", [&](const httplib::Request& req, httplib::Response& res) {
        auto json = nlohmann::json::parse(req.body);
        std::string message;
        bool success = deleteService(db, json["id"], message);
    
        nlohmann::json response;
        response["success"] = success;
        response["message"] = message;
        res.set_content(response.dump(), "application/json");
    });
    
    server.Get(R"(/api/queue/(\d+))", [&](const httplib::Request& req, httplib::Response& res) {
        int serviceId = std::stoi(req.matches[1]);
        auto entries = getQueueByService(db, serviceId);
        res.set_content(queueToJson(entries).dump(), "application/json");
    });
    
    server.Post("/api/queue/add", [&](const httplib::Request& req, httplib::Response& res) {
        auto json = nlohmann::json::parse(req.body);
        std::string message;
        bool success = addToQueue(db, json["serviceId"], json["name"], json["reason"], message);
    
        nlohmann::json response;
        response["success"] = success;
        response["message"] = message;
        res.set_content(response.dump(), "application/json");
    });
    
    server.Post("/api/queue/remove", [&](const httplib::Request& req, httplib::Response& res) {
        auto json = nlohmann::json::parse(req.body);
        std::string message;
        bool success = removeFromQueue(db, json["queueId"], message);
    
        nlohmann::json response;
        response["success"] = success;
        response["message"] = message;
        res.set_content(response.dump(), "application/json");
    });
    
    server.Post("/api/queue/serve-next", [&](const httplib::Request& req, httplib::Response& res) {
        auto json = nlohmann::json::parse(req.body);
        std::string servedName, message;
        bool success = serveNextInQueue(db, json["serviceId"], servedName, message);
    
        nlohmann::json response;
        response["success"] = success;
        response["message"] = message;
        response["servedName"] = servedName;
        res.set_content(response.dump(), "application/json");
    });

    server.Get(R"(/api/queue/wait-time/(\d+)/(\d+))", [&](const httplib::Request& req, httplib::Response& res) {
        int serviceId = std::stoi(req.matches[1]);
        int queueId = std::stoi(req.matches[2]);

        int waitTime = estimateWaitTimeForUser(db, serviceId, queueId);

        nlohmann::json response;

        if (waitTime == -1) {
            res.status = 404;
            response["success"] = false;
            response["message"] = "Queue entry not found.";
        } else {
            response["success"] = true;
            response["estimatedWaitMinutes"] = waitTime;
        }

        res.set_content(response.dump(), "application/json");
    });


    server.Get("/admin/unit-tests", [&](const httplib::Request& req, httplib::Response& res) {
        const std::string username = "admin"; //placeholder
        res.set_content(unitTestsPage(username, tests), "text/html");
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

    server.Post("/api/create-account", [](const httplib::Request& req, httplib::Response& res) {
        auto json = nlohmann::json::parse(req.body);
        std::string email = json["email"];
        std::string password = json["password"];
        CreateAccountResult result = createAccount(email, password);
        nlohmann::json response;
        response["success"] = result.success;
        response["message"] = result.message;
        if (result.success) {
            response["userId"] = result.userId;
        }
        res.set_content(response.dump(), "application/json");
    });

    server.Post("/admin/create-queue", [](const httplib::Request& req, httplib::Response& res) {
    std::string username = req.get_header_value("X-Username");
    if (username.empty()) username = "admin";

    std::string service_name      = req.get_param_value("service_name");
    std::string description       = req.get_param_value("description");
    std::string expected_duration = req.get_param_value("expected_duration");
    std::string priority          = req.get_param_value("priority");

    if (service_name.empty() || service_name.size() > 100) {
        res.set_content(createQueuePage(username,
            "Service name is required and must be 100 characters or fewer."), "text/html");
        return;
    }

    if (description.empty()) {
        res.set_content(createQueuePage(username,
            "Description is required."), "text/html");
        return;
    }

    int duration = 0;
    try {
        duration = std::stoi(expected_duration);
    } catch (...) {
        duration = -1;
    }
    if (duration < 1 || duration > 480) {
        res.set_content(createQueuePage(username,
            "Expected duration must be a number between 1 and 480 minutes."), "text/html");
        return;
    }

    if (priority != "low" && priority != "medium" && priority != "high") {
        res.set_content(createQueuePage(username,
            "Priority level must be low, medium, or high."), "text/html");
        return;
    }
        

    int priorityValue = 2;
    if (priority == "high") {
        priorityValue = 0;
    } else if (priority == "medium") {
        priorityValue = 1;
    } else {
        priorityValue = 2;
    }


    sqlite3* localDb = nullptr;
    std::string localDbPath = DATABASE_FILE_LOCATION;
    if (sqlite3_open(localDbPath.c_str(), &localDb) != SQLITE_OK) {
        res.set_content(createQueuePage(username,
            "Failed to open database."), "text/html");
        return;
    }


        
    std::string message;
    bool success = addService(localDb, service_name, description, duration, priorityValue, message);
    sqlite3_close(localDb);

    if (!success) {
        res.set_content(createQueuePage(username, message), "text/html");
        return;
    }

    res.set_redirect("/admin-dashboard");
    });

    auto* testsPtr = &tests;

        

    server.Post(R"(/admin/unit-tests/run/(\d+))", [testsPtr](const httplib::Request& req, httplib::Response& res) {
        const std::string& idxStr = req.matches[1];
        const std::size_t  idx    = static_cast<std::size_t>(std::stoul(idxStr));

        if (idx >= testsPtr->size()) {
            res.status = 404;
            res.set_content(R"({"error":"test index out of range"})", "application/json");
            return;
        }

        (*testsPtr)[idx]->runTest();
        const bool passed = (*testsPtr)[idx]->getResult();

        res.set_content(
            std::string(R"({"pass":)") + (passed ? "true" : "false") + "}",
            "application/json"
        );
    });

    server.Post("/api/join-queue", [&db](const httplib::Request& req, httplib::Response& res) {
        res.set_header("Content-Type", "application/json");
 
        auto idParam = req.get_param_value("id");
        if (idParam.empty()) {
            res.status = 400;
            res.set_content("{\"success\":false,\"error\":\"Missing service id\"}", "application/json");
            return;
        }
 
        int serviceId = 0;
        try {
            serviceId = std::stoi(idParam);
        } catch (...) {
            res.status = 400;
            res.set_content("{\"success\":false,\"error\":\"Invalid service id\"}", "application/json");
            return;
        }
 
        // Parse reason from JSON body
        std::string reason;
        auto contentType = req.get_header_value("Content-Type");
        if (contentType.find("application/json") != std::string::npos && !req.body.empty()) {
            // Simple JSON extraction for "reason" field
            auto pos = req.body.find("\"reason\"");
            if (pos != std::string::npos) {
                auto start = req.body.find('"', pos + 8);
                if (start != std::string::npos) {
                    auto end = req.body.find('"', start + 1);
                    if (end != std::string::npos) {
                        reason = req.body.substr(start + 1, end - start - 1);
                    }
                }
            }
        }
 
        if (reason.empty()) {
            res.status = 400;
            res.set_content("{\"success\":false,\"error\":\"Missing reason\"}", "application/json");
            return;
        }
 
        if (reason.length() > 128) {
            res.status = 400;
            res.set_content("{\"success\":false,\"error\":\"Reason exceeds 128 characters\"}", "application/json");
            return;
        }
 
        std::string message; // var to ignore message content
        addToQueue(db, serviceId, getUsernameById(currentUserId), reason , message);
 
        res.status = 200;
        res.set_content("{\"success\":true}", "application/json");
    });

    server.Post("/api/leave-queue", [](const httplib::Request& req, httplib::Response& res) {
    // ── Auth check ────────────────────────────────────────────────────────────
    if (currentUserId == 0) {
        res.status = 401;
        res.set_content(R"({"success":false,"error":"not logged in"})", "application/json");
        return;
    }

    const std::string currentUsername = getUsernameById(currentUserId);

    // ── Parse body ────────────────────────────────────────────────────────────
    int serviceId = -1;
    try {
        auto body = nlohmann::json::parse(req.body);
        serviceId = body.at("serviceId").get<int>();
    } catch (...) {
        res.status = 400;
        res.set_content(R"({"success":false,"error":"invalid body"})", "application/json");
        return;
    }

    if (serviceId < 0) {
        res.status = 400;
        res.set_content(R"({"success":false,"error":"invalid serviceId"})", "application/json");
        return;
    }

    // ── Open DB and remove user ───────────────────────────────────────────────
    sqlite3* db = nullptr;
    std::string dbLoc = DATABASE_FILE_LOCATION;

    if (sqlite3_open(dbLoc.c_str(), &db) != SQLITE_OK) {
        sqlite3_close(db);
        res.status = 500;
        res.set_content(R"({"success":false,"error":"db error"})", "application/json");
        return;
    }

    removeUserFromQueue(db, currentUsername, serviceId);
    sqlite3_close(db);

    res.set_content(R"({"success":true})", "application/json");
});



    server.Post("/admin/export-report", [&db](const httplib::Request&, httplib::Response& res) {
        // Auth check — mirror the admin-dashboard guard
        if (currentUserId == 0) {
            res.status = 401;
            res.set_content("Unauthorized", "text/plain");
            return;
        }
        if (getAuthLevelById(currentUserId) < 2) {
            res.status = 403;
            res.set_content("Forbidden: Admins only", "text/plain");
            return;
        }
    
        // Build a unique temp path so concurrent exports don't collide
        const std::string outputPath = "queue_report_" 
                                     + std::to_string(std::time(nullptr)) 
                                     + ".xlsx";
    
        if (!exportDatabaseReport(db, outputPath)) {
            res.status = 500;
            res.set_content("Failed to generate report.", "text/plain");
            return;
        }
    
        // Read the file into memory
        std::ifstream file(outputPath, std::ios::binary);
        if (!file) {
            res.status = 500;
            res.set_content("Report file could not be opened after generation.", "text/plain");
            return;
        }
    
        std::string fileData((std::istreambuf_iterator<char>(file)),
                              std::istreambuf_iterator<char>());
        file.close();
    
        // Clean up the temp file
        std::remove(outputPath.c_str());
    
        // Send as a downloadable xlsx attachment
        res.set_header("Content-Disposition", "attachment; filename=\"queue_report.xlsx\"");
        res.set_content(fileData, "application/vnd.openxmlformats-officedocument.spreadsheetml.sheet");
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
                    <a href="/dashboard">Dashboard</a>
                    <a href="/calendar">Calendar</a>
                    <a href="/active-queues">Active Queues</a>
                    <a href="/join-queue">Join a Queue</a>
                    
                    <!-- Admin Dropdown (hidden by default) -->
                    <div class="dropdown hidden" id="adminDropdown">
                        <button class="dropdown-button">Admin</button>
                        <div class="dropdown-content">
                            <a href="/admin-dashboard">Dashboard</a>
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
    
    sqlite3_close(db);
}
