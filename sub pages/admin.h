#ifndef ADMIN_H
#define ADMIN_H

// #include "../external/cpp-httplib-0.15.3/httplib.h"
#include <string>

// UI-only Admin Dashboard page (static placeholder data)
static inline std::string adminDashboardPage(const std::string& username) {
    return R"(
<!doctype html>
<html>
<head>
  <meta charset="utf-8" />
  <title>Admin Dashboard</title>
  <style>
    body { font-family: Arial, sans-serif; margin: 0; background: #f6f7fb; color: #111827; }
    header { background: #111827; color: white; padding: 16px 24px; display:flex; justify-content:space-between; align-items:center; }
    header a { color: #c7d2fe; text-decoration:none; margin-left: 12px; }
    .container { padding: 24px; max-width: 1100px; margin: 0 auto; }

    .grid { display: grid; grid-template-columns: repeat(4, 1fr); gap: 16px; }
    .card { background: white; border-radius: 12px; padding: 16px; box-shadow: 0 2px 10px rgba(0,0,0,0.06); }
    .muted { color: #6b7280; font-size: 12px; }
    .big { font-size: 22px; font-weight: 700; margin-top: 8px; }

    .row { display: grid; grid-template-columns: 2fr 1fr; gap: 16px; margin-top: 16px; }
    table { width:100%; border-collapse: collapse; }
    th, td { padding: 10px; border-bottom: 1px solid #e5e7eb; text-align:left; font-size: 14px; }
    th { color:#374151; font-size: 12px; text-transform: uppercase; letter-spacing: .04em; }
    .btn { padding: 6px 10px; border-radius: 8px; border: 1px solid #e5e7eb; background: #fff; cursor: pointer; }
    .btn.primary { background:#111827; color:white; border-color:#111827; }
    .pill { padding: 3px 8px; border-radius: 999px; font-size: 12px; display:inline-block; }
    .pill.open { background:#dcfce7; color:#166534; }
    .pill.closed { background:#fee2e2; color:#991b1b; }

    .log { background: #0b1220; color:#e5e7eb; padding: 14px; border-radius: 12px; height: 260px; overflow:auto;
           font-family: ui-monospace, SFMono-Regular, Menlo, Monaco, Consolas, "Liberation Mono"; font-size: 12px; }
    .log div { margin-bottom: 8px; }
    .topbar { display:flex; justify-content:space-between; align-items:center; margin-top: 16px; }
  </style>
</head>
<body>
  <header>
    <div><strong>Admin Dashboard</strong> <span class="muted" style="color:#9ca3af;">(UI placeholders)</span></div>
    <div>
      <span class="muted" style="color:#9ca3af;">Signed in as</span> <strong>)" + username + R"(</strong>
      <a href="/">Home</a>
      <a href="/login">Logout</a>
    </div>
  </header>

  <div class="container">
    <div class="grid">
      <div class="card"><div class="muted">Total Users</div><div class="big">42</div></div>
      <div class="card"><div class="muted">Active Queues</div><div class="big">3</div></div>
      <div class="card"><div class="muted">People Waiting</div><div class="big">11</div></div>
      <div class="card"><div class="muted">Alerts</div><div class="big">2</div></div>
    </div>

    <div class="row">
      <div class="card">
        <div class="topbar">
          <div>
            <div style="font-weight:700;">Manage Queues</div>
            <div class="muted">Static table for now</div>
          </div>
          <button class="btn primary">Create Queue</button>
        </div>

        <table>
          <thead>
            <tr>
              <th>Queue</th><th>Status</th><th>Waiting</th><th>Avg Wait</th><th>Actions</th>
            </tr>
          </thead>
          <tbody>
            <tr>
              <td>Advising</td>
              <td><span class="pill open">Open</span></td>
              <td>5</td>
              <td>12 min</td>
              <td>
                <button class="btn">View</button>
                <button class="btn">Close</button>
                <button class="btn">Clear</button>
              </td>
            </tr>
            <tr>
              <td>Tutoring</td>
              <td><span class="pill open">Open</span></td>
              <td>4</td>
              <td>9 min</td>
              <td>
                <button class="btn">View</button>
                <button class="btn">Close</button>
                <button class="btn">Clear</button>
              </td>
            </tr>
            <tr>
              <td>Tech Support</td>
              <td><span class="pill closed">Closed</span></td>
              <td>2</td>
              <td>—</td>
              <td>
                <button class="btn">View</button>
                <button class="btn">Open</button>
                <button class="btn">Clear</button>
              </td>
            </tr>
          </tbody>
        </table>
      </div>

      <div class="card">
        <div style="font-weight:700;">System Log</div>
        <div class="muted" style="margin-bottom:10px;">Notifications can plug in here later</div>
        <div class="log">
          <div>[INFO] Server started</div>
          <div>[INFO] root logged in</div>
          <div>[WARN] Queue "Advising" nearing capacity</div>
          <div>[INFO] New account created: user42@example.com</div>
          <div>[INFO] Placeholder log entry...</div>
        </div>
      </div>
    </div>

    <div class="card" style="margin-top:16px;">
      <div style="font-weight:700;">User Management</div>
      <div class="muted" style="margin-bottom:10px;">Static table for now</div>
      <table>
        <thead>
          <tr>
            <th>User</th><th>Role</th><th>Last Login</th><th>Actions</th>
          </tr>
        </thead>
        <tbody>
          <tr>
            <td>root</td><td>Admin</td><td>Today</td>
            <td><button class="btn">Reset PW</button> <button class="btn">Disable</button></td>
          </tr>
          <tr>
            <td>user1@example.com</td><td>User</td><td>Yesterday</td>
            <td><button class="btn">Promote</button> <button class="btn">Disable</button></td>
          </tr>
          <tr>
            <td>user2@example.com</td><td>User</td><td>—</td>
            <td><button class="btn">Promote</button> <button class="btn">Disable</button></td>
          </tr>
        </tbody>
      </table>
    </div>

  </div>
</body>
</html>
)";
}

#endif
