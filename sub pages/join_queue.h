#ifndef join_queue_h
#define join_queue_h
#include "../external/cpp-httplib-0.15.3/httplib.h"
#include "queue_management.h"


    // Modified signature — pass the resolved username at the call site:
//   joinQueuePage(getUsernameById(currentUserId))
static inline std::string joinQueuePage() {
    const std::string& currentUsername = getUsernameById(currentUserId);
    sqlite3* db = nullptr;
    std::string dbPath = DATABASE_FILE_LOCATION;
    std::cout << "Attempting to open database at: " << dbPath << std::endl;

    bool dbExists = std::ifstream(dbPath).good();

    if (sqlite3_open(dbPath.c_str(), &db) != SQLITE_OK) {
        sqlite3_close(db);
        std::cerr << "Failed to open database: " << sqlite3_errmsg(db) << std::endl;
    }

    if (!dbExists) {
        std::cout << "No database found, creating default" << std::endl;
        if (!initDatabase(db)) {
            std::cout << "Failed to initialize database." << std::endl;
        }
    } else {
        std::cout << "Pre-existing database found" << std::endl;
    }

    std::vector<ServiceEntry> services = getServices(db);

    // Collect every queue the current user is already in
    std::vector<int> userQueues = getServiceIdUserCurrentlyIn(db, currentUsername);
    // Put them in a set for O(1) lookup
    std::set<int> userQueueSet(userQueues.begin(), userQueues.end());

    sqlite3_close(db);

    // Build dynamic queue cards
    std::string queueCards;
    for (const auto& svc : services) {
        int estimatedWait = svc.estServiceTime * svc.length;

        float t = std::min(estimatedWait, 120) / 120.0f;
        int hue = static_cast<int>(120.0f * (1.0f - t));

        std::string accentColor      = "hsl(" + std::to_string(hue) + ", 72%, 48%)";
        std::string accentColorLight = "hsl(" + std::to_string(hue) + ", 60%, 65%)";
        std::string barStyle         = "background: linear-gradient(90deg, " + accentColor + ", " + accentColorLight + ");";

        std::string statusClass, statusLabel;
        if (estimatedWait < 60) {
            statusClass = "status-open"; statusLabel = "Open";
        } else if (estimatedWait < 120) {
            statusClass = "status-busy"; statusLabel = "Busy";
        } else {
            statusClass = "status-full"; statusLabel = "Full";
        }

        std::string waitLabel = "~" + std::to_string(estimatedWait);
        if (estimatedWait >= 120) waitLabel = "120+";

        std::string idStr = std::to_string(svc.id);

        // Determine whether the user is already in this specific queue
        bool alreadyInQueue = (userQueueSet.count(svc.id) > 0);

        // If already in the queue, fetch their existing reason to pre-fill
        std::string existingReason = "";
        if (alreadyInQueue) {
            // Re-open a fresh handle just for this lookup
            sqlite3* db2 = nullptr;
            if (sqlite3_open(dbPath.c_str(), &db2) == SQLITE_OK) {
                existingReason = getUserReasonByID(db2, currentUsername, svc.id);
                sqlite3_close(db2);
            }
        }

        // In-queue indicator (only shown when the user is already in this queue)
        std::string inQueueNote = alreadyInQueue
            ? "<div class=\"in-queue-note\"><div class=\"in-queue-dot\"></div>You are in this queue</div>"
            : "";

        // Textarea: pre-filled + disabled when already in queue; disabled when full
        std::string textareaAttrs = "";
        if (alreadyInQueue || statusClass == "status-full") textareaAttrs += " disabled";

        // Escape single quotes in existingReason for HTML attribute safety
        std::string safeReason = existingReason;
        // (a full HTML-escape helper is preferable in production)
        for (std::size_t pos = 0; (pos = safeReason.find('"', pos)) != std::string::npos; pos += 6)
            safeReason.replace(pos, 1, "&quot;");

        // Action button
        std::string actionButton;
        if (alreadyInQueue) {
            actionButton =
                "<button class=\"btn btn-leave\" data-service-id=\"" + idStr + "\">"
                "  Leave Queue"
                "</button>";
        } else if (statusClass == "status-full") {
            actionButton =
                "<button class=\"btn btn-join\" data-service-id=\"" + idStr + "\" disabled>"
                "  Queue Full"
                "</button>";
        } else {
            actionButton =
                "<button class=\"btn btn-join\" data-service-id=\"" + idStr + "\">"
                "  Join Queue"
                "</button>";
        }

        queueCards +=
            "<div class=\"queue-card\">"
            "  <div class=\"card-accent-bar\" style=\"" + barStyle + "\"></div>"
            "  <div class=\"card-body\">"
            "    <div class=\"card-top\">"
            "      <div>"
            "        <div class=\"queue-name\">" + svc.name + "</div>"
            "      </div>"
            "      <span class=\"status-badge " + statusClass + "\">" + statusLabel + "</span>"
            "    </div>"
            "    <div class=\"stats-row\">"
            "      <div class=\"stat\"><div class=\"stat-value\">" + waitLabel + "</div><div class=\"stat-label\">Min Wait</div></div>"
            "      <div class=\"stat\"><div class=\"stat-value\">" + std::to_string(svc.length) + "</div><div class=\"stat-label\">In Queue</div></div>"
            "    </div>"
            + inQueueNote +
            "    <div class=\"reason-field\">"
            "      <div class=\"reason-label\">"
            "        <span>Reason for visit</span>"
            "        <span class=\"char-count\" id=\"charCount-" + idStr + "\">" + std::to_string(existingReason.size()) + " / 128</span>"
            "      </div>"
            "      <textarea class=\"reason-input\" id=\"reason-" + idStr + "\""
            "        maxlength=\"128\" placeholder=\"Briefly describe why you're joining this queue\""
            "        oninput=\"handleReasonInput('" + idStr + "', this)\""
            "        " + textareaAttrs + ">" + safeReason + "</textarea>"
            "      <div class=\"reason-error\" id=\"reasonError-" + idStr + "\">Please enter a reason before joining.</div>"
            "    </div>"
            "    <div class=\"btn-row\">"
            + actionButton +
            "    </div>"
            "  </div>"
            "</div>";
    }

    return
    R"(<!DOCTYPE html>
<html>
<head>
    <title>Join a Queue</title>
    <link href="https://fonts.googleapis.com/css2?family=DM+Sans:wght@300;400;500;600&family=Space+Mono:wght@400;700&display=swap" rel="stylesheet">
    <style>
        * { margin: 0; padding: 0; box-sizing: border-box; }
        :root {
            --bg: #1e1e2f; --surface: #2b2b40; --surface2: #32324a;
            --border: #3f3f5c; --accent: #4CAF50;
            --accent-dim: rgba(76, 175, 80, 0.15); --warn: #f59e0b;
            --warn-dim: rgba(245, 158, 11, 0.12); --danger: #ef4444;
            --danger-dim: rgba(239, 68, 68, 0.12); --blue: #60a5fa;
            --blue-dim: rgba(96, 165, 250, 0.12);
            --text: #e2e8f0; --muted: #8888aa;
        }
        body { font-family: 'DM Sans', sans-serif; background-color: var(--bg); color: var(--text); min-height: 100vh; }
        .navbar { background: var(--surface); padding: 15px 30px; box-shadow: 0 2px 10px rgba(0,0,0,0.3); display: flex; align-items: center; gap: 20px; }
        .navbar a { color: white; text-decoration: none; padding: 10px 20px; border-radius: 6px; transition: background 0.3s; }
        .navbar a:hover { background: #3a3a52; }
        .dropdown { position: relative; display: inline-block; }
        .dropdown.hidden { display: none; }
        .dropdown-button { color: white; padding: 10px 20px; border: none; background: transparent; cursor: pointer; border-radius: 6px; transition: background 0.3s; font-size: 16px; font-family: 'DM Sans', sans-serif; }
        .dropdown-button:hover { background: #3a3a52; }
        .dropdown-content { display: none; position: absolute; background-color: var(--surface); min-width: 180px; box-shadow: 0 8px 16px rgba(0,0,0,0.4); border-radius: 6px; z-index: 1; padding-top: 5px; }
        .dropdown-content a { color: white; padding: 12px 16px; text-decoration: none; display: block; transition: background 0.3s; }
        .dropdown-content a:first-child { border-radius: 6px 6px 0 0; }
        .dropdown-content a:last-child { border-radius: 0 0 6px 6px; }
        .dropdown-content a:hover { background: #3a3a52; }
        .dropdown:hover .dropdown-content { display: block; }
        .user-info { margin-left: auto; color: var(--accent); padding: 10px 20px; background: var(--bg); border-radius: 6px; font-weight: bold; display: none; }
        .user-info.show { display: block; }
        .page { max-width: 1100px; margin: 0 auto; padding: 40px 24px 60px; }
        .page-header { margin-bottom: 36px; }
        .page-header h1 { font-family: 'Space Mono', monospace; font-size: 28px; font-weight: 700; color: var(--text); letter-spacing: -0.5px; }
        .page-header p { color: var(--muted); margin-top: 6px; font-size: 15px; }
        .filter-bar { display: flex; gap: 10px; margin-bottom: 28px; flex-wrap: wrap; }
        .filter-btn { padding: 7px 16px; border-radius: 20px; border: 1px solid var(--border); background: transparent; color: var(--muted); font-family: 'DM Sans', sans-serif; font-size: 13px; cursor: pointer; transition: all 0.2s; }
        .filter-btn:hover, .filter-btn.active { background: var(--accent-dim); border-color: var(--accent); color: var(--accent); }
        .queue-grid { display: grid; grid-template-columns: repeat(auto-fill, minmax(320px, 1fr)); gap: 20px; }
        .queue-card { background: var(--surface); border-radius: 14px; border: 1px solid var(--border); overflow: hidden; transition: transform 0.2s, box-shadow 0.2s; animation: fadeUp 0.4s ease both; }
        .queue-card:hover { transform: translateY(-3px); box-shadow: 0 12px 32px rgba(0,0,0,0.4); }
        @keyframes fadeUp { from { opacity: 0; transform: translateY(16px); } to { opacity: 1; transform: translateY(0); } }
        .queue-card:nth-child(1) { animation-delay: 0.05s; }
        .queue-card:nth-child(2) { animation-delay: 0.10s; }
        .queue-card:nth-child(3) { animation-delay: 0.15s; }
        .queue-card:nth-child(4) { animation-delay: 0.20s; }
        .queue-card:nth-child(5) { animation-delay: 0.25s; }
        .queue-card:nth-child(6) { animation-delay: 0.30s; }
        .card-accent-bar { height: 4px; width: 100%; }
        .card-body { padding: 20px 22px 22px; }
        .card-top { display: flex; justify-content: space-between; align-items: flex-start; margin-bottom: 14px; }
        .queue-name { font-family: 'Space Mono', monospace; font-size: 15px; font-weight: 700; color: var(--text); line-height: 1.35; }
        .queue-dept { font-size: 12px; color: var(--muted); margin-top: 3px; }
        .status-badge { padding: 4px 10px; border-radius: 20px; font-size: 11px; font-weight: 600; text-transform: uppercase; letter-spacing: 0.5px; white-space: nowrap; flex-shrink: 0; margin-left: 12px; }
        .status-open { background: var(--accent-dim); color: var(--accent); }
        .status-busy { background: var(--warn-dim); color: var(--warn); }
        .status-full { background: var(--danger-dim); color: var(--danger); }
        .stats-row { display: flex; gap: 0; background: var(--surface2); border-radius: 10px; overflow: hidden; margin-bottom: 16px; }
        .stat { flex: 1; padding: 12px 14px; text-align: center; border-right: 1px solid var(--border); }
        .stat:last-child { border-right: none; }
        .stat-value { font-family: 'Space Mono', monospace; font-size: 20px; font-weight: 700; color: var(--text); line-height: 1; }
        .stat-label { font-size: 10px; color: var(--muted); text-transform: uppercase; letter-spacing: 0.6px; margin-top: 5px; }
        .reason-field { margin-bottom: 14px; }
        .reason-label { display: flex; justify-content: space-between; align-items: baseline; margin-bottom: 6px; }
        .reason-label span:first-child { font-size: 12px; font-weight: 600; color: var(--muted); text-transform: uppercase; letter-spacing: 0.5px; }
        .char-count { font-size: 11px; color: var(--muted); font-family: 'Space Mono', monospace; }
        .char-count.near-limit { color: var(--warn); }
        .char-count.at-limit { color: var(--danger); }
        .reason-input { width: 100%; background: var(--surface2); border: 1px solid var(--border); border-radius: 8px; color: var(--text); font-family: 'DM Sans', sans-serif; font-size: 13px; line-height: 1.5; padding: 10px 12px; resize: none; height: 72px; transition: border-color 0.2s, box-shadow 0.2s; }
        .reason-input::placeholder { color: var(--muted); }
        .reason-input:focus { outline: none; border-color: var(--accent); box-shadow: 0 0 0 3px var(--accent-dim); }
        .reason-input.invalid { border-color: var(--danger); box-shadow: 0 0 0 3px var(--danger-dim); }
        .reason-input:disabled { opacity: 0.4; cursor: not-allowed; }
        .reason-error { font-size: 11px; color: var(--danger); margin-top: 5px; display: none; }
        .reason-error.show { display: block; }
        .btn-row { display: flex; gap: 10px; }
        .btn { flex: 1; padding: 10px 16px; border-radius: 8px; border: none; font-family: 'DM Sans', sans-serif; font-size: 14px; font-weight: 600; cursor: pointer; transition: all 0.2s; }
        .btn-join { background: var(--accent); color: #0d1f0d; }
        .btn-join:hover { background: #5dbf61; transform: scale(1.02); }
        .btn-join:disabled { background: var(--surface2); color: var(--muted); cursor: not-allowed; transform: none; }
        .btn-leave { background: var(--danger-dim); color: var(--danger); border: 1px solid rgba(239, 68, 68, 0.3); }
        .btn-leave:hover { background: rgba(239, 68, 68, 0.25); transform: scale(1.02); }
        .btn-leave:disabled { opacity: 0.5; cursor: not-allowed; transform: none; }
        .in-queue-note { display: flex; align-items: center; gap: 6px; font-size: 12px; color: var(--accent); margin-bottom: 10px; font-weight: 500; }
        .in-queue-dot { width: 7px; height: 7px; border-radius: 50%; background: var(--accent); animation: pulse 1.8s infinite; }
        @keyframes pulse { 0%, 100% { opacity: 1; } 50% { opacity: 0.3; } }
    </style>
</head>
<body>
<nav class="navbar">
    <a href="/dashboard">Dashboard</a>
    <a href="/calendar">Calendar</a>
    <a href="/active-queues">Active Queues</a>
    <a href="/join-queue">Join a Queue</a>
    <div class="dropdown hidden" id="adminDropdown">
        <button class="dropdown-button">Admin</button>
        <div class="dropdown-content">
            <a href="/admin-dashboard">Dashboard</a>
            <a href="/analytics">Analytics</a>
            <a href="/edit-data">Edit Data</a>
        </div>
    </div>
    <div class="dropdown">
        <button class="dropdown-button">Account</button>
        <div class="dropdown-content">
            <a href="/login" id="loginLogoutLink">Login / Logout</a>
            <a href="/account-settings">Account Settings</a>
        </div>
    </div>
    <div class="user-info" id="userInfo"><span id="username"></span></div>
</nav>
<div class="page">
    <div class="page-header">
        <h1>Join a Queue</h1>
        <p>Browse available office hours, advising sessions, and support queues.</p>
    </div>
    <div class="filter-bar">
        <button class="filter-btn active">All</button>
        <button class="filter-btn">Advising</button>
        <button class="filter-btn">Tutoring</button>
        <button class="filter-btn">Financial Aid</button>
        <button class="filter-btn">IT Support</button>
        <button class="filter-btn">Health Services</button>
    </div>
    <div class="queue-grid">)" + queueCards + R"(</div>
</div>
<script>
    document.querySelectorAll('.filter-btn').forEach(btn => {
        btn.addEventListener('click', () => {
            document.querySelectorAll('.filter-btn').forEach(b => b.classList.remove('active'));
            btn.classList.add('active');
        });
    });

    function handleReasonInput(id, el) {
        const len = el.value.length;
        const counter = document.getElementById('charCount-' + id);
        counter.textContent = len + ' / 128';
        counter.className = 'char-count' + (len >= 128 ? ' at-limit' : len >= 100 ? ' near-limit' : '');
        if (len > 0) {
            el.classList.remove('invalid');
            document.getElementById('reasonError-' + id).classList.remove('show');
        }
    }

    // ── Join Queue ────────────────────────────────────────────────────────────
    document.querySelectorAll('.btn-join:not([disabled])').forEach(btn => {
        btn.addEventListener('click', async () => {
            const serviceId = parseInt(btn.getAttribute('data-service-id'));
            const reasonInput = document.getElementById('reason-' + serviceId);
            const reason = reasonInput.value.trim();

            if (!reason) {
                reasonInput.classList.add('invalid');
                document.getElementById('reasonError-' + serviceId).classList.add('show');
                reasonInput.focus();
                return;
            }

            try {
                const res = await fetch(`/api/join-queue?id=${serviceId}`, {
                    method: 'POST',
                    headers: { 'Content-Type': 'application/json' },
                    body: JSON.stringify({ reason })
                });
                const data = await res.json();

                if (data.success) {
                    btn.textContent = 'Joined!';
                    btn.style.background = 'var(--accent-dim)';
                    btn.style.color = 'var(--accent)';
                    btn.disabled = true;
                    reasonInput.disabled = true;
                } else {
                    btn.textContent = 'Error';
                }
            } catch (e) {
                btn.textContent = 'Error';
            }
        });
    });

    // ── Leave Queue ───────────────────────────────────────────────────────────
    document.querySelectorAll('.btn-leave').forEach(btn => {
        btn.addEventListener('click', async () => {
            const serviceId = parseInt(btn.getAttribute('data-service-id'));

            btn.textContent = 'Leaving…';
            btn.disabled = true;

            try {
                const res = await fetch(`/api/leave-queue`, {
                    method: 'POST',
                    headers: { 'Content-Type': 'application/json' },
                    body: JSON.stringify({ serviceId })
                });
                const data = await res.json();

                if (data.success) {
                    // Reload so the card reverts to the Join state with a fresh queue count
                    window.location.reload();
                } else {
                    btn.textContent = 'Error';
                    btn.disabled = false;
                }
            } catch (e) {
                btn.textContent = 'Error';
                btn.disabled = false;
            }
        });
    });

    async function checkLoginStatus() {
        try {
            const response = await fetch('/api/check-login');
            const data = await response.json();
            const userInfoDiv = document.getElementById('userInfo');
            const usernameSpan = document.getElementById('username');
            const loginLogoutLink = document.getElementById('loginLogoutLink');
            const adminDropdown = document.getElementById('adminDropdown');
            if (data.loggedIn) {
                usernameSpan.textContent = data.username;
                userInfoDiv.classList.add('show');
                loginLogoutLink.textContent = 'Logout';
                if (data.authLevel >= 2) { adminDropdown.classList.remove('hidden'); }
            } else {
                loginLogoutLink.textContent = 'Login';
            }
        } catch (e) {}
    }
    checkLoginStatus();
</script>
</body>
</html>)";
}
#endif