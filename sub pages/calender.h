#ifndef calender_h
#define calender_h

#include "../external/cpp-httplib-0.15.3/httplib.h"
#include <string>

std::string calenderPageData() {
    return R"HTML(
<!doctype html>
<html>
<head>
  <meta charset="utf-8">
  <title>QueueSmart Calendar</title>
  <style>
    body { font-family: Arial, sans-serif; margin: 0; background: #1f1f33; color: #f5f5f5; }
    nav { background: #2d2b45; padding: 16px 24px; display: flex; gap: 28px; }
    nav a { color: white; text-decoration: none; font-weight: 600; }
    .container { max-width: 900px; margin: 60px auto; padding: 24px; }
    .card { background: #302f49; padding: 24px; border-radius: 14px; box-shadow: 0 4px 12px rgba(0,0,0,0.25); margin-bottom: 18px; }
    h1 { color: #4ade80; }
    .muted { color: #b7b7cc; }
    table { width: 100%; border-collapse: collapse; margin-top: 16px; }
    th, td { padding: 12px; border-bottom: 1px solid #4b4a66; text-align: left; }
    th { color: #c7c7e6; }
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
      <h1>Queue Schedule Overview</h1>
      <p class="muted">
        This page summarizes available service periods for QueueSmart. It supports the scheduling view of the system and helps users understand when services are available.
      </p>

      <table>
        <thead>
          <tr>
            <th>Service</th>
            <th>Availability</th>
            <th>Typical Duration</th>
          </tr>
        </thead>
        <tbody>
          <tr>
            <td>Advising</td>
            <td>Monday–Friday</td>
            <td>20 minutes</td>
          </tr>
          <tr>
            <td>Tutoring</td>
            <td>Monday–Thursday</td>
            <td>30 minutes</td>
          </tr>
          <tr>
            <td>Tech Support</td>
            <td>Monday–Friday</td>
            <td>15 minutes</td>
          </tr>
        </tbody>
      </table>
    </div>
  </div>
</body>
</html>
)HTML";
}

#endif
