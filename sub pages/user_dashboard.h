#ifndef USER_DASHBOARD_H
#define USER_DASHBOARD_H

#include <string>

// UI-only User Dashboard page (static placeholder data)
static inline std::string userDashboardPage(const std::string& username) {
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
      <span class="muted">(UI placeholders)</span>
    </div>
    <div>
      <span class="muted">Signed in as</span> <strong>)" + username + R"(</strong>
      <a href="/">Home</a>
      <a href="/login">Logout</a>
    </div>
  </header>

  <div class="container">

    <div class="grid">
      <div class="card"><div class="muted">Your Status</div><div class="big">In Queue</div><div class="muted">Tutoring</div></div>
      <div class="card"><div class="muted">Position</div><div class="big">#3</div><div class="muted">Estimated 9–12 min</div></div>
      <div class="card"><div class="muted">Queues Joined</div><div class="big">5</div><div class="muted">This month</div></div>
      <div class="card"><div class="muted">Notifications</div><div class="big">2</div><div class="muted">Unread</div></div>
    </div>

    <div class="row">
      <div class="card">
        <div style="display:flex; justify-content:space-between; align-items:center;">
          <div>
            <div style="font-weight:700;">Current / Recent Activity</div>
            <div class="muted">Static table for now</div>
          </div>
          <div style="display:flex; gap:10px;">
            <button class="btn">Refresh</button>
            <a class="btn primary" href="/join-queue" style="text-decoration:none;">Join a Queue</a>
          </div>
        </div>

        <table style="margin-top:12px;">
          <thead>
            <tr>
              <th>Queue</th><th>Status</th><th>Position</th><th>ETA</th><th>Action</th>
            </tr>
          </thead>
          <tbody>
            <tr>
              <td>Tutoring</td>
              <td><span class="pill inqueue">In Queue</span></td>
              <td>#3</td>
              <td>9–12 min</td>
              <td><button class="btn">Leave</button></td>
            </tr>
            <tr>
              <td>Advising</td>
              <td><span class="pill waiting">Waiting</span></td>
              <td>#8</td>
              <td>~25 min</td>
              <td><button class="btn">View</button></td>
            </tr>
            <tr>
              <td>Tech Support</td>
              <td><span class="pill done">Done</span></td>
              <td>—</td>
              <td>—</td>
              <td><button class="btn">Details</button></td>
            </tr>
          </tbody>
        </table>
      </div>

      <div class="notice">
        <div style="font-weight:700;">Notifications</div>
        <div class="muted" style="margin-bottom:10px;">UI-only list for now</div>
        <div>• Your turn is coming up soon (Tutoring).</div>
        <div>• Queue “Advising” changed ETA.</div>
        <div class="muted">Tip: Notifications tab can be built next.</div>
      </div>
    </div>

    <div class="card" style="margin-top:16px;">
      <div style="font-weight:700;">History</div>
      <div class="muted" style="margin-bottom:10px;">Static table for now</div>
      <table>
        <thead>
          <tr>
            <th>Date</th><th>Queue</th><th>Result</th><th>Duration</th>
          </tr>
        </thead>
        <tbody>
          <tr><td>Today</td><td>Tutoring</td><td>In progress</td><td>—</td></tr>
          <tr><td>Yesterday</td><td>Advising</td><td>Completed</td><td>14 min</td></tr>
          <tr><td>Feb 10</td><td>Tech Support</td><td>Completed</td><td>6 min</td></tr>
        </tbody>
      </table>
    </div>

  </div>
</body>
</html>
)";
}

#endif
