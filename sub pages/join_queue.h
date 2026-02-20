#ifndef join_queue_h
#define join_queue_h
#include "../external/cpp-httplib-0.15.3/httplib.h"


    static inline std::string joinQueuePage() {
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
        .stats-row { display: flex; gap: 0; background: var(--surface2); border-radius: 10px; overflow: hidden; margin-bottom: 18px; }
        .stat { flex: 1; padding: 12px 14px; text-align: center; border-right: 1px solid var(--border); }
        .stat:last-child { border-right: none; }
        .stat-value { font-family: 'Space Mono', monospace; font-size: 20px; font-weight: 700; color: var(--text); line-height: 1; }
        .stat-label { font-size: 10px; color: var(--muted); text-transform: uppercase; letter-spacing: 0.6px; margin-top: 5px; }
        .location-tag { display: inline-flex; align-items: center; gap: 5px; font-size: 12px; color: var(--muted); margin-bottom: 16px; }
        .location-tag svg { width: 13px; height: 13px; flex-shrink: 0; }
        .btn-row { display: flex; gap: 10px; }
        .btn { flex: 1; padding: 10px 16px; border-radius: 8px; border: none; font-family: 'DM Sans', sans-serif; font-size: 14px; font-weight: 600; cursor: pointer; transition: all 0.2s; }
        .btn-join { background: var(--accent); color: #0d1f0d; }
        .btn-join:hover { background: #5dbf61; transform: scale(1.02); }
        .btn-join:disabled { background: var(--surface2); color: var(--muted); cursor: not-allowed; transform: none; }
        .btn-leave { background: var(--danger-dim); color: var(--danger); border: 1px solid rgba(239, 68, 68, 0.3); }
        .btn-leave:hover { background: rgba(239, 68, 68, 0.25); transform: scale(1.02); }
        .in-queue-note { display: flex; align-items: center; gap: 6px; font-size: 12px; color: var(--accent); margin-bottom: 10px; font-weight: 500; }
        .in-queue-dot { width: 7px; height: 7px; border-radius: 50%; background: var(--accent); animation: pulse 1.8s infinite; }
        @keyframes pulse { 0%, 100% { opacity: 1; } 50% { opacity: 0.3; } }
        .bar-green  { background: linear-gradient(90deg, #4CAF50, #81c784); }
        .bar-blue   { background: linear-gradient(90deg, #60a5fa, #93c5fd); }
        .bar-amber  { background: linear-gradient(90deg, #f59e0b, #fcd34d); }
        .bar-purple { background: linear-gradient(90deg, #a78bfa, #c4b5fd); }
        .bar-rose   { background: linear-gradient(90deg, #f43f5e, #fb7185); }
        .bar-teal   { background: linear-gradient(90deg, #2dd4bf, #5eead4); }
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
    <div class="queue-grid">
        <div class="queue-card">
            <div class="card-accent-bar bar-green"></div>
            <div class="card-body">
                <div class="card-top">
                    <div>
                        <div class="queue-name">Computer Science<br>Office Hours</div>
                        <div class="queue-dept">Dept. of Computer Science</div>
                    </div>
                    <span class="status-badge status-open">Open</span>
                </div>
                <div class="in-queue-note">
                    <span class="in-queue-dot"></span>
                    You are currently in this queue
                </div>
                <div class="stats-row">
                    <div class="stat"><div class="stat-value">~12</div><div class="stat-label">Min Wait</div></div>
                    <div class="stat"><div class="stat-value">8</div><div class="stat-label">In Queue</div></div>
                    <div class="stat"><div class="stat-value">3</div><div class="stat-label">Staff</div></div>
                </div>
                <div class="location-tag">
                    <svg viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2"><path d="M12 2a7 7 0 0 0-7 7c0 5.25 7 13 7 13s7-7.75 7-13a7 7 0 0 0-7-7zm0 9.5a2.5 2.5 0 1 1 0-5 2.5 2.5 0 0 1 0 5z"/></svg>
                    Engineering Hall, Room 204
                </div>
                <div class="btn-row">
                    <button class="btn btn-join" disabled>Already Joined</button>
                    <button class="btn btn-leave">Leave Queue</button>
                </div>
            </div>
        </div>
        <div class="queue-card">
            <div class="card-accent-bar bar-blue"></div>
            <div class="card-body">
                <div class="card-top">
                    <div>
                        <div class="queue-name">Academic Advising<br>&#8212; STEM Track</div>
                        <div class="queue-dept">Office of Academic Affairs</div>
                    </div>
                    <span class="status-badge status-busy">Busy</span>
                </div>
                <div class="stats-row">
                    <div class="stat"><div class="stat-value">~34</div><div class="stat-label">Min Wait</div></div>
                    <div class="stat"><div class="stat-value">21</div><div class="stat-label">In Queue</div></div>
                    <div class="stat"><div class="stat-value">2</div><div class="stat-label">Staff</div></div>
                </div>
                <div class="location-tag">
                    <svg viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2"><path d="M12 2a7 7 0 0 0-7 7c0 5.25 7 13 7 13s7-7.75 7-13a7 7 0 0 0-7-7zm0 9.5a2.5 2.5 0 1 1 0-5 2.5 2.5 0 0 1 0 5z"/></svg>
                    Administration Bldg., Room 101
                </div>
                <div class="btn-row"><button class="btn btn-join">Join Queue</button></div>
            </div>
        </div>)"

        R"(<div class="queue-card">
            <div class="card-accent-bar bar-amber"></div>
            <div class="card-body">
                <div class="card-top">
                    <div>
                        <div class="queue-name">Math Tutoring<br>Center</div>
                        <div class="queue-dept">Learning Resource Center</div>
                    </div>
                    <span class="status-badge status-open">Open</span>
                </div>
                <div class="stats-row">
                    <div class="stat"><div class="stat-value">~7</div><div class="stat-label">Min Wait</div></div>
                    <div class="stat"><div class="stat-value">4</div><div class="stat-label">In Queue</div></div>
                    <div class="stat"><div class="stat-value">5</div><div class="stat-label">Staff</div></div>
                </div>
                <div class="location-tag">
                    <svg viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2"><path d="M12 2a7 7 0 0 0-7 7c0 5.25 7 13 7 13s7-7.75 7-13a7 7 0 0 0-7-7zm0 9.5a2.5 2.5 0 1 1 0-5 2.5 2.5 0 0 1 0 5z"/></svg>
                    Barker Library, 3rd Floor
                </div>
                <div class="btn-row"><button class="btn btn-join">Join Queue</button></div>
            </div>
        </div>
        <div class="queue-card">
            <div class="card-accent-bar bar-purple"></div>
            <div class="card-body">
                <div class="card-top">
                    <div>
                        <div class="queue-name">Financial Aid<br>Counseling</div>
                        <div class="queue-dept">Office of Financial Aid</div>
                    </div>
                    <span class="status-badge status-full">Full</span>
                </div>
                <div class="stats-row">
                    <div class="stat"><div class="stat-value">~60+</div><div class="stat-label">Min Wait</div></div>
                    <div class="stat"><div class="stat-value">40</div><div class="stat-label">In Queue</div></div>
                    <div class="stat"><div class="stat-value">1</div><div class="stat-label">Staff</div></div>
                </div>
                <div class="location-tag">
                    <svg viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2"><path d="M12 2a7 7 0 0 0-7 7c0 5.25 7 13 7 13s7-7.75 7-13a7 7 0 0 0-7-7zm0 9.5a2.5 2.5 0 1 1 0-5 2.5 2.5 0 0 1 0 5z"/></svg>
                    Student Services Bldg., Room 12
                </div>
                <div class="btn-row"><button class="btn btn-join" disabled>Queue Full</button></div>
            </div>
        </div>
        <div class="queue-card">
            <div class="card-accent-bar bar-rose"></div>
            <div class="card-body">
                <div class="card-top">
                    <div>
                        <div class="queue-name">IT Help Desk</div>
                        <div class="queue-dept">Information Technology Services</div>
                    </div>
                    <span class="status-badge status-open">Open</span>
                </div>
                <div class="stats-row">
                    <div class="stat"><div class="stat-value">~5</div><div class="stat-label">Min Wait</div></div>
                    <div class="stat"><div class="stat-value">2</div><div class="stat-label">In Queue</div></div>
                    <div class="stat"><div class="stat-value">4</div><div class="stat-label">Staff</div></div>
                </div>
                <div class="location-tag">
                    <svg viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2"><path d="M12 2a7 7 0 0 0-7 7c0 5.25 7 13 7 13s7-7.75 7-13a7 7 0 0 0-7-7zm0 9.5a2.5 2.5 0 1 1 0-5 2.5 2.5 0 0 1 0 5z"/></svg>
                    Technology Center, Ground Floor
                </div>
                <div class="btn-row"><button class="btn btn-join">Join Queue</button></div>
            </div>
        </div>
        <div class="queue-card">
            <div class="card-accent-bar bar-teal"></div>
            <div class="card-body">
                <div class="card-top">
                    <div>
                        <div class="queue-name">Student Health<br>Services</div>
                        <div class="queue-dept">Campus Health &amp; Wellness</div>
                    </div>
                    <span class="status-badge status-busy">Busy</span>
                </div>
                <div class="stats-row">
                    <div class="stat"><div class="stat-value">~22</div><div class="stat-label">Min Wait</div></div>
                    <div class="stat"><div class="stat-value">11</div><div class="stat-label">In Queue</div></div>
                    <div class="stat"><div class="stat-value">3</div><div class="stat-label">Staff</div></div>
                </div>
                <div class="location-tag">
                    <svg viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2"><path d="M12 2a7 7 0 0 0-7 7c0 5.25 7 13 7 13s7-7.75 7-13a7 7 0 0 0-7-7zm0 9.5a2.5 2.5 0 1 1 0-5 2.5 2.5 0 0 1 0 5z"/></svg>
                    Wellness Center, Suite 100
                </div>
                <div class="btn-row"><button class="btn btn-join">Join Queue</button></div>
            </div>
        </div>
    </div>
</div>)"

        R"(<script>
    document.querySelectorAll('.filter-btn').forEach(btn => {
        btn.addEventListener('click', () => {
            document.querySelectorAll('.filter-btn').forEach(b => b.classList.remove('active'));
            btn.classList.add('active');
        });
    });
    document.querySelectorAll('.btn-join:not([disabled])').forEach(btn => {
        btn.addEventListener('click', () => {
            btn.textContent = 'Joined!';
            btn.style.background = 'var(--accent-dim)';
            btn.style.color = 'var(--accent)';
            btn.disabled = true;
        });
    });
    document.querySelectorAll('.btn-leave').forEach(btn => {
        btn.addEventListener('click', () => {
            const card = btn.closest('.queue-card');
            const note = card.querySelector('.in-queue-note');
            const joinBtn = card.querySelector('.btn-join');
            if (note) note.remove();
            joinBtn.disabled = false;
            joinBtn.textContent = 'Join Queue';
            joinBtn.style.background = '';
            joinBtn.style.color = '';
            btn.remove();
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