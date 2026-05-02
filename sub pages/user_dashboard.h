#ifndef USER_DASHBOARD_H
#define USER_DASHBOARD_H

#include <string>

// User Dashboard page with queue status, database-backed queue records, and notifications
static inline std::string userDashboardPage(const std::string& username) {

  sqlite3* db = nullptr;
  std::string dbPath = DATABASE_FILE_LOCATION;
  std::cout << "Attempting to open database at: " << dbPath << std::endl;

  bool dbExists = std::ifstream(dbPath).good();

  if (sqlite3_open(dbPath.c_str(), &db) != SQLITE_OK) {
      sqlite3_close(db);
      std::cerr << "Failed to open database in user dash: " << sqlite3_errmsg(db) << std::endl;
  }

  if (!dbExists) {
      std::cout << "No database found, creating default" << std::endl;
      if (!initDatabase(db)) {
          std::cout << "Failed to initialize database." << std::endl;
      }
  } else {
      std::cout << "Pre-existing database found" << std::endl;
  }

  // Build active queue rows using the user's current queue memberships
  std::string activeQueueRows;

  std::vector<int> serviceIds = getServiceIdUserCurrentlyIn(db, username);

  for (int serviceId : serviceIds) {
      // Fetch the service name
      std::string serviceName = "Unknown Service";
      const char* serviceSQL = "SELECT name FROM services WHERE id = ?;";
      sqlite3_stmt* svcStmt = nullptr;
      if (sqlite3_prepare_v2(db, serviceSQL, -1, &svcStmt, nullptr) == SQLITE_OK) {
          sqlite3_bind_int(svcStmt, 1, serviceId);
          if (sqlite3_step(svcStmt) == SQLITE_ROW) {
              serviceName = reinterpret_cast<const char*>(sqlite3_column_text(svcStmt, 0));
          }
          sqlite3_finalize(svcStmt);
      }

      // Find the user's specific entry in this service's queue
      std::vector<QueueEntry> entries = getQueueByService(db, serviceId);
      for (const auto& entry : entries) {
          if (entry.name == username) {
              std::string etaStr = entry.waitTime > 0
                  ? "~" + std::to_string(entry.waitTime) + " min"
                  : "—";

              activeQueueRows +=
                  "<tr>"
                  "<td>" + serviceName + "</td>"
                  "<td><span class=\"pill inqueue\">In Queue</span></td>"
                  "<td>#" + std::to_string(entry.position) + "</td>"
                  "<td>" + etaStr + "</td>"
                  "</tr>";
              break; // A user can only occupy one slot per service
          }
      }
  }

  if (activeQueueRows.empty()) {
      activeQueueRows = "<tr><td colspan=\"4\" class=\"muted\">You are not currently in any queues.</td></tr>";
  }

 
    // Build history rows by querying the history table
  std::string historyRows;
  
  // First, find the current user's account ID from their username
  int currentUserId = -1;
  
  const char* userIdSQL = "SELECT id FROM accounts WHERE username = ?;";
  sqlite3_stmt* userStmt = nullptr;
  
  if (sqlite3_prepare_v2(db, userIdSQL, -1, &userStmt, nullptr) == SQLITE_OK) {
      sqlite3_bind_text(userStmt, 1, username.c_str(), -1, SQLITE_TRANSIENT);
  
      if (sqlite3_step(userStmt) == SQLITE_ROW) {
          currentUserId = sqlite3_column_int(userStmt, 0);
      }
  
      sqlite3_finalize(userStmt);
  }
  
  if (currentUserId != -1) {
      const char* historySQL =
          "SELECT h.message, h.queue_id "
          "FROM history h "
          "WHERE h.user_id = ? "
          "ORDER BY h.id DESC;";
  
      sqlite3_stmt* stmt = nullptr;
  
      if (sqlite3_prepare_v2(db, historySQL, -1, &stmt, nullptr) == SQLITE_OK) {
          sqlite3_bind_int(stmt, 1, currentUserId);
  
          while (sqlite3_step(stmt) == SQLITE_ROW) {
              const unsigned char* actionText = sqlite3_column_text(stmt, 0);
              std::string action = actionText ? reinterpret_cast<const char*>(actionText) : "";
  
              int queueId = sqlite3_column_int(stmt, 1);
              std::string service = "Queue Activity";
  
              std::string actionLabel, pillClass;
  
              if (action.find("JOINED_QUEUE") != std::string::npos ||
                  action.find("join") != std::string::npos ||
                  action.find("Joined") != std::string::npos) {
                  actionLabel = "Joined";
                  pillClass = "inqueue";
              } else if (action.find("REMOVED_FROM_QUEUE") != std::string::npos ||
                         action.find("remove") != std::string::npos ||
                         action.find("left") != std::string::npos ||
                         action.find("served") != std::string::npos ||
                         action.find("Removed") != std::string::npos) {
                  actionLabel = "Left / Served";
                  pillClass = "done";
              } else {
                  actionLabel = action;
                  pillClass = "waiting";
              }
  
              historyRows +=
                  "<tr>"
                  "<td>" + service + "</td>"
                  "<td><span class=\"pill " + pillClass + "\">" + actionLabel + "</span></td>"
                  "<td>" + std::to_string(queueId) + "</td>"
                  "</tr>";
          }
  
          sqlite3_finalize(stmt);
      }
  }


  if (historyRows.empty()) {
      historyRows = "<tr><td colspan=\"3\" class=\"muted\">No history found.</td></tr>";
  }

    return R"(
<!doctype html>
<html>
<head>
  <meta charset="utf-8" />
  <title>User Dashboard</title>
  <style>
    body { font-family: Arial, sans-serif; margin: 0; background: #1e1e2f; color: white; }
    header { background: #2b2b40; color: white; padding: 16px 24px; display:flex; justify-content:space-between; align-items:center; box-shadow: 0 2px 10px rgba(0,0,0,0.3);}
    header a { color: #c7d2fe; text-decoration:none; margin-left: 12px; }
    .container { padding: 24px; max-width: 1100px; margin: 0 auto; }

    .grid { display: grid; grid-template-columns: repeat(4, 1fr); gap: 16px; }
    .card { background: #2b2b40; border-radius: 12px; padding: 16px; box-shadow: 0 2px 10px rgba(0,0,0,0.25); }
    .muted { color: #9ca3af; font-size: 12px; }
    .big { font-size: 22px; font-weight: 700; margin-top: 8px; color: #4CAF50; }

    .row { display: grid; grid-template-columns: 2fr 1fr; gap: 16px; margin-top: 16px; }

    table { width:100%; border-collapse: collapse; }
    th, td { padding: 10px; border-bottom: 1px solid #3a3a52; text-align:left; font-size: 14px; }
    th { color:#cbd5e1; font-size: 12px; text-transform: uppercase; letter-spacing: .04em; }

    .btn { padding: 8px 12px; border-radius: 8px; border: 1px solid #3a3a52; background: #1e1e2f; color: white; cursor: pointer; }
    .btn.primary { background:#4CAF50; border-color:#4CAF50; color:#0b1220; font-weight: 700; }
    .btn:hover { background: #3a3a52; }

    .pill { padding: 3px 8px; border-radius: 999px; font-size: 12px; display:inline-block; }
    .pill.waiting { background:#fde68a; color:#92400e; }
    .pill.inqueue { background:#bfdbfe; color:#1e3a8a; }
    .pill.done { background:#dcfce7; color:#166534; }

    .notice { background:#0b1220; border: 1px solid #3a3a52; border-radius: 12px; padding: 14px; }
    .notice div { margin-bottom: 8px; }
  </style>
</head>
<body>
  <header>
    <div>
      <strong>User Dashboard</strong>
      <span class="muted">Queue Status</span>
    </div>
    <div>
      <span class="muted">Signed in as</span> <strong>)" + username + R"(</strong>
      <a href="/">Home</a>
      <a href="/login">Logout</a>
    </div>
  </header>

  <div class="container">

    <div class="grid">
      <div class="card"><div class="muted">Your Status</div><div class="big">Active</div><div class="muted">Queue participation shown below</div></div>
      <div class="card"><div class="muted">Wait-Time Feature</div><div class="big">On</div><div class="muted">ETA calculated from queue position</div></div>
      <div class="card"><div class="muted">Queue Records</div><div class="big">Live</div><div class="muted">Loaded from database</div></div>
      <div class="card"><div class="muted">Notifications</div><div class="big">Ready</div><div class="muted">Queue status updates</div></div>
    </div>

    <div class="row">
      <div class="card">
        <div style="display:flex; justify-content:space-between; align-items:center;">
          <div>
            <div style="font-weight:700;">Active Queues</div>
            <div class="muted">Queues you are currently in</div>
          </div>
          <div style="display:flex; gap:10px;">
            <button class="btn">Refresh</button>
            <a class="btn primary" href="/join-queue" style="text-decoration:none;">Join a Queue</a>
          </div>
        </div>

        <table style="margin-top:12px;">
          <thead>
            <tr>
              <th>Queue</th><th>Status</th><th>Position</th><th>ETA</th>
            </tr>
          </thead>
          <tbody>
            )" + activeQueueRows + R"(
          </tbody>
        </table>
      </div>

      <div class="notice">
        <div style="font-weight:700;">Notifications</div>
        <div class="muted" style="margin-bottom:10px;">Queue status notifications</div>
        <div>• Your turn is coming up soon (Tutoring).</div>
        <div>• Queue "Advising" changed ETA.</div>
        <div class="muted">Notifications help users track queue updates and estimated wait-time changes.</div>
      </div>
    </div>

    <div class="card" style="margin-top:16px;">
      <div style="font-weight:700;">History</div>
      <div class="muted" style="margin-bottom:10px;">Queue activity history for your account</div>
      <table>
        <thead>
          <tr>
            <th>Service</th><th>Action</th><th>Queue ID</th>
          </tr>
        </thead>
        <tbody>
          )" + historyRows + R"(
        </tbody>
      </table>
    </div>

  </div>
</body>
</html>
)";
}
#endif
