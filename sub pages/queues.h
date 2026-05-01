#ifndef queues_h
#define queues_h

#include "../external/cpp-httplib-0.15.3/httplib.h"
#include <string>

std::string activeQueuesPage() {
    return R"HTML(
<!doctype html>
<html>
<head>
  <meta charset="utf-8">
  <title>Active Queues</title>
  <style>
    body { font-family: Arial, sans-serif; margin: 0; background: #1f1f33; color: #f5f5f5; }
    nav { background: #2d2b45; padding: 16px 24px; display: flex; gap: 28px; }
    nav a { color: white; text-decoration: none; font-weight: 600; }
    .container { max-width: 1000px; margin: 60px auto; padding: 24px; }
    .card { background: #302f49; padding: 24px; border-radius: 14px; box-shadow: 0 4px 12px rgba(0,0,0,0.25); }
    h1 { color: #4ade80; }
    .muted { color: #b7b7cc; }
    table { width: 100%; border-collapse: collapse; margin-top: 18px; }
    th, td { padding: 12px; border-bottom: 1px solid #4b4a66; text-align: left; }
    th { color: #c7c7e6; }
    .open { color: #4ade80; font-weight: bold; }
    .busy { color: #facc15; font-weight: bold; }
    .full { color: #f87171; font-weight: bold; }
    .btn { display: inline-block; margin-top: 20px; padding: 10px 16px; background: #4ade80; color: #111827; text-decoration: none; border-radius: 8px; font-weight: 700; }
  </style>
</head>
<body>
  <nav>
    <a href="/dashboard">Dashboard</a>
    <a href="/calendar">Calendar</a>
    <a href="/active-queues">Active Queues</a>
    <a href="/join-queue">Join a Queue</a>
    <a href="/admin-dashboard">Admin</a>
  </nav>

  <div class="container">
    <div class="card">
      <h1>Active Queues</h1>
      <p class="muted">
        This page gives users a quick view of available queues, queue status, number of people waiting, and estimated wait time.
      </p>

      <table>
        <thead>
          <tr>
            <th>Queue</th>
            <th>Status</th>
            <th>People Waiting</th>
            <th>Estimated Wait</th>
          </tr>
        </thead>
        <tbody>
          <tr>
            <td>Advising</td>
            <td class="busy">Busy</td>
            <td>5</td>
            <td>~100 min</td>
          </tr>
          <tr>
            <td>Tutoring</td>
            <td class="full">Full</td>
            <td>4</td>
            <td>120+ min</td>
          </tr>
          <tr>
            <td>Tech Support</td>
            <td class="open">Open</td>
            <td>2</td>
            <td>~30 min</td>
          </tr>
        </tbody>
      </table>

      <a class="btn" href="/join-queue">Join a Queue</a>
    </div>
  </div>
</body>
</html>
)HTML";
}

#endif
