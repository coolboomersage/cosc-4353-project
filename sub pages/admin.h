#ifndef ADMIN_H
#define ADMIN_H

// #include "../external/cpp-httplib-0.15.3/httplib.h"
#include <string>

// ─── Create Queue Page ────────────────────────────────────────────────────────
// Route: GET /admin/create-queue
static inline std::string createQueuePage(const std::string& username, const std::string& error = "") {
    std::string errorBanner = "";
    if (!error.empty()) {
        errorBanner = R"HTML(<div class="banner error">)HTML" + error + R"HTML(</div>)HTML";
    }

    return R"HTML(
<!doctype html>
<html>
<head>
  <meta charset="utf-8" />
  <title>Create Queue — Admin</title>
  <style>
    body { font-family: Arial, sans-serif; margin: 0; background: #f6f7fb; color: #111827; }
    header { background: #111827; color: white; padding: 16px 24px; display:flex; justify-content:space-between; align-items:center; }
    header a { color: #c7d2fe; text-decoration:none; margin-left: 12px; }
    .container { padding: 24px; max-width: 640px; margin: 0 auto; }

    .card { background: white; border-radius: 12px; padding: 24px; box-shadow: 0 2px 10px rgba(0,0,0,0.06); margin-top: 24px; }
    .card h2 { margin: 0 0 4px 0; font-size: 18px; }
    .muted { color: #6b7280; font-size: 12px; }

    .field { margin-top: 18px; display: flex; flex-direction: column; gap: 6px; }
    label { font-size: 13px; font-weight: 600; color: #374151; }
    label span.req { color: #ef4444; margin-left: 2px; }
    input[type="text"], input[type="number"], textarea, select {
      padding: 9px 12px; border: 1px solid #d1d5db; border-radius: 8px;
      font-size: 14px; font-family: inherit; width: 100%; box-sizing: border-box;
      transition: border-color 0.15s;
    }
    input:focus, textarea:focus, select:focus {
      outline: none; border-color: #6366f1;
      box-shadow: 0 0 0 3px rgba(99,102,241,0.15);
    }
    input.invalid, textarea.invalid, select.invalid {
      border-color: #ef4444;
      box-shadow: 0 0 0 3px rgba(239,68,68,0.12);
    }
    textarea { resize: vertical; min-height: 90px; }
    .hint { font-size: 11px; color: #9ca3af; }
    .char-count { font-size: 11px; color: #9ca3af; text-align: right; }
    .char-count.warn { color: #f59e0b; }
    .char-count.over { color: #ef4444; }

    .priority-group { display: flex; gap: 10px; margin-top: 2px; }
    .priority-group label {
      flex: 1; display: flex; align-items: center; justify-content: center;
      gap: 6px; padding: 8px; border: 1px solid #d1d5db; border-radius: 8px;
      cursor: pointer; font-weight: 500; font-size: 13px; transition: all 0.15s;
    }
    .priority-group input[type="radio"] { display: none; }
    .priority-group input[type="radio"]:checked + span { font-weight: 700; }
    .priority-group label:has(input[value="low"]:checked)    { background:#f0fdf4; border-color:#16a34a; color:#15803d; }
    .priority-group label:has(input[value="medium"]:checked) { background:#fffbeb; border-color:#d97706; color:#b45309; }
    .priority-group label:has(input[value="high"]:checked)   { background:#fef2f2; border-color:#dc2626; color:#b91c1c; }

    .actions { display:flex; justify-content:flex-end; gap:10px; margin-top: 24px; }
    .btn { padding: 8px 16px; border-radius: 8px; border: 1px solid #e5e7eb; background: #fff; cursor: pointer; font-size: 14px; }
    .btn.primary { background:#111827; color:white; border-color:#111827; }
    .btn.primary:hover { background:#1f2937; }
    .btn:hover { background:#f9fafb; }

    .banner { padding: 10px 14px; border-radius: 8px; margin-top: 16px; font-size: 13px; }
    .banner.error { background:#fef2f2; color:#991b1b; border: 1px solid #fecaca; }

    .field-error { font-size: 11px; color: #ef4444; display: none; }
    .field-error.visible { display: block; }
  </style>
</head>
<body>
  <header>
    <div><strong>Admin Dashboard</strong> <span style="color:#9ca3af; font-size:12px;">/ Create Queue</span></div>
    <div>
      <span style="color:#9ca3af; font-size:12px;">Signed in as</span> <strong>)HTML" + username + R"HTML(</strong>
      <a href="/admin">Dashboard</a>
      <a href="/">Home</a>
      <a href="/login">Logout</a>
    </div>
  </header>

  <div class="container">
    )HTML" + errorBanner + R"HTML(

    <div class="card">
      <h2>Create New Queue</h2>
      <div class="muted">All fields marked <span style="color:#ef4444;">*</span> are required.</div>

      <form id="createQueueForm" action="/admin/create-queue" method="POST" novalidate>

        <!-- Service Name -->
        <div class="field">
          <label for="serviceName">Service Name <span class="req">*</span></label>
          <input type="text" id="serviceName" name="service_name"
                 maxlength="100" placeholder="e.g. Academic Advising"
                 oninput="updateCharCount(this, 'nameCount', 100); clearInvalid(this)" />
          <div class="char-count" id="nameCount">0 / 100</div>
          <div class="field-error" id="serviceNameErr">Service name is required (max 100 characters).</div>
        </div>

        <!-- Description -->
        <div class="field">
          <label for="description">Description <span class="req">*</span></label>
          <textarea id="description" name="description"
                    placeholder="Briefly describe what this queue is for..."
                    oninput="clearInvalid(this)"></textarea>
          <div class="field-error" id="descriptionErr">Description is required.</div>
        </div>

        <!-- Expected Duration -->
        <div class="field">
          <label for="duration">Expected Duration <span class="req">*</span> <span class="hint">(minutes)</span></label>
          <input type="number" id="duration" name="expected_duration"
                 min="1" max="480" placeholder="e.g. 15"
                 oninput="clearInvalid(this)" />
          <div class="hint">Enter a value between 1 and 480 minutes.</div>
          <div class="field-error" id="durationErr">Please enter a valid duration between 1 and 480 minutes.</div>
        </div>

        <!-- Priority Level -->
        <div class="field">
          <label>Priority Level <span class="req">*</span></label>
          <div class="priority-group" id="priorityGroup">
            <label>
              <input type="radio" name="priority" value="low" checked />
              <span>Low</span>
            </label>
            <label>
              <input type="radio" name="priority" value="medium" />
              <span>Medium</span>
            </label>
            <label>
              <input type="radio" name="priority" value="high" />
              <span>High</span>
            </label>
          </div>
          <div class="field-error" id="priorityErr">Please select a priority level.</div>
        </div>

        <div class="actions">
          <a href="/admin"><button type="button" class="btn">Cancel</button></a>
          <button type="submit" class="btn primary">Create Queue</button>
        </div>

      </form>
    </div>
  </div>

  <script>
    function updateCharCount(input, countId, max) {
      const len = input.value.length;
      const el = document.getElementById(countId);
      el.textContent = len + ' / ' + max;
      el.className = 'char-count' + (len >= max ? ' over' : len >= max * 0.85 ? ' warn' : '');
    }

    function clearInvalid(input) {
      input.classList.remove('invalid');
    }

    function showError(inputId, errId) {
      const input = document.getElementById(inputId);
      if (input) input.classList.add('invalid');
      const err = document.getElementById(errId);
      if (err) err.classList.add('visible');
    }

    document.getElementById('createQueueForm').addEventListener('submit', function(e) {
      let valid = true;

      // Reset all errors
      document.querySelectorAll('.field-error').forEach(el => el.classList.remove('visible'));
      document.querySelectorAll('.invalid').forEach(el => el.classList.remove('invalid'));

      // Validate Service Name
      const name = document.getElementById('serviceName');
      if (!name.value.trim() || name.value.trim().length > 100) {
        showError('serviceName', 'serviceNameErr');
        valid = false;
      }

      // Validate Description
      const desc = document.getElementById('description');
      if (!desc.value.trim()) {
        showError('description', 'descriptionErr');
        valid = false;
      }

      // Validate Duration
      const dur = document.getElementById('duration');
      const durVal = parseInt(dur.value, 10);
      if (!dur.value || isNaN(durVal) || durVal < 1 || durVal > 480) {
        showError('duration', 'durationErr');
        valid = false;
      }

      // Validate Priority (radio — always has a default, but guard anyway)
      const priority = document.querySelector('input[name="priority"]:checked');
      if (!priority) {
        document.getElementById('priorityErr').classList.add('visible');
        valid = false;
      }

      if (!valid) e.preventDefault();
    });
  </script>
</body>
</html>
)HTML";
}

// ─── Admin Dashboard Page ─────────────────────────────────────────────────────
// Route: GET /admin
static inline std::string adminDashboardPage(const std::string& username) {
    return R"HTML(
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

    /* Modal Overlay */
    .modal-overlay {
      display: none;
      position: fixed;
      inset: 0;
      background: rgba(0,0,0,0.45);
      z-index: 1000;
      align-items: center;
      justify-content: center;
    }
    .modal-overlay.active { display: flex; }

    /* Modal Box */
    .modal {
      background: white;
      border-radius: 16px;
      width: 480px;
      max-width: 95vw;
      max-height: 80vh;
      display: flex;
      flex-direction: column;
      box-shadow: 0 8px 40px rgba(0,0,0,0.18);
      overflow: hidden;
    }
    .modal-header {
      padding: 18px 20px 14px;
      border-bottom: 1px solid #e5e7eb;
      display: flex;
      justify-content: space-between;
      align-items: center;
    }
    .modal-header h2 { margin: 0; font-size: 16px; }
    .modal-close {
      background: none;
      border: none;
      cursor: pointer;
      font-size: 20px;
      color: #6b7280;
      line-height: 1;
      padding: 0 4px;
    }
    .modal-close:hover { color: #111827; }
    .modal-body { padding: 16px 20px; overflow-y: auto; flex: 1; }
    .modal-footer {
      padding: 12px 20px;
      border-top: 1px solid #e5e7eb;
      display: flex;
      justify-content: flex-end;
      gap: 8px;
    }

    /* Queue list */
    .queue-list { list-style: none; margin: 0; padding: 0; }
    .queue-item {
      display: flex;
      align-items: center;
      gap: 12px;
      padding: 10px 12px;
      border: 1px solid #e5e7eb;
      border-radius: 10px;
      margin-bottom: 8px;
      background: #fff;
      cursor: grab;
      user-select: none;
      transition: box-shadow 0.15s, background 0.15s;
    }
    .queue-item:active { cursor: grabbing; }
    .queue-item.dragging {
      opacity: 0.4;
    }
    .queue-item.drag-over {
      border-color: #6366f1;
      background: #eef2ff;
      box-shadow: 0 0 0 2px #6366f1;
    }
    .drag-handle {
      color: #9ca3af;
      font-size: 16px;
      cursor: grab;
      flex-shrink: 0;
    }
    .queue-position {
      width: 24px;
      height: 24px;
      background: #111827;
      color: white;
      border-radius: 50%;
      display: flex;
      align-items: center;
      justify-content: center;
      font-size: 11px;
      font-weight: 700;
      flex-shrink: 0;
    }
    .queue-info { flex: 1; }
    .queue-name { font-weight: 600; font-size: 14px; }
    .queue-meta { font-size: 12px; color: #6b7280; margin-top: 2px; }
    .queue-wait {
      font-size: 12px;
      color: #374151;
      background: #f3f4f6;
      padding: 3px 8px;
      border-radius: 999px;
      white-space: nowrap;
    }
    .drag-hint { font-size: 12px; color: #9ca3af; margin-bottom: 12px; }
  </style>
</head>
<body>
  <header>
    <div><strong>Admin Dashboard</strong> <span class="muted" style="color:#9ca3af;">(UI placeholders)</span></div>
    <div>
      <span class="muted" style="color:#9ca3af;">Signed in as</span> <strong>)" + username + R"HTML(</strong>
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
          <a href="/admin/create-queue"><button class="btn primary">Create Queue</button></a>
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
                <button class="btn" onclick="openQueueModal('Advising')">View</button>
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
                <button class="btn" onclick="openQueueModal('Tutoring')">View</button>
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
                <button class="btn" onclick="openQueueModal('Tech Support')">View</button>
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

  <!-- Queue View Modal -->
  <div class="modal-overlay" id="queueModal" onclick="handleOverlayClick(event)">
    <div class="modal">
      <div class="modal-header">
        <h2 id="modalTitle">Queue: Advising</h2>
        <button class="modal-close" onclick="closeQueueModal()">&#x2715;</button>
      </div>
      <div class="modal-body">
        <div class="drag-hint">&#9776; Drag rows to reorder the queue.</div>
        <ul class="queue-list" id="queueList"></ul>
      </div>
      <div class="modal-footer">
        <button class="btn" onclick="closeQueueModal()">Close</button>
        <button class="btn primary" onclick="saveQueueOrder()">Save Order</button>
      </div>
    </div>
  </div>

  <script>
    // Static user data per queue
    const queueData = {
      'Advising': [
        { name: 'Alice Johnson',    reason: 'Course registration help',  waited: '2 min' },
        { name: 'Bob Smith',        reason: 'Degree audit question',      waited: '7 min' },
        { name: 'Carol Williams',   reason: 'Transfer credit inquiry',    waited: '13 min' },
        { name: 'David Lee',        reason: 'Scholarship advising',       waited: '18 min' },
        { name: 'Eva Martinez',     reason: 'General advising',           waited: '24 min' },
      ],
      'Tutoring': [
        { name: 'Frank Brown',      reason: 'Calculus II help',           waited: '3 min' },
        { name: 'Grace Kim',        reason: 'Python debugging',           waited: '9 min' },
        { name: 'Henry Davis',      reason: 'Linear algebra review',      waited: '15 min' },
        { name: 'Isla Thompson',    reason: 'Essay feedback',             waited: '20 min' },
      ],
      'Tech Support': [
        { name: 'Jack Wilson',      reason: 'VPN setup issue',            waited: '5 min' },
        { name: 'Karen Moore',      reason: 'Printer not working',        waited: '11 min' },
      ],
    };

    let currentQueue = null;
    let dragSrcIndex = null;

    function openQueueModal(queueName) {
      currentQueue = queueName;
      document.getElementById('modalTitle').textContent = 'Queue: ' + queueName;
      renderQueueList();
      document.getElementById('queueModal').classList.add('active');
    }

    function closeQueueModal() {
      document.getElementById('queueModal').classList.remove('active');
      currentQueue = null;
    }

    function handleOverlayClick(e) {
      if (e.target === document.getElementById('queueModal')) closeQueueModal();
    }

    function renderQueueList() {
      const list = document.getElementById('queueList');
      list.innerHTML = '';
      const users = queueData[currentQueue];
      users.forEach((user, index) => {
        const li = document.createElement('li');
        li.className = 'queue-item';
        li.draggable = true;
        li.dataset.index = index;
        li.innerHTML = `
          <span class="drag-handle">&#9776;</span>
          <span class="queue-position">${index + 1}</span>
          <div class="queue-info">
            <div class="queue-name">${user.name}</div>
            <div class="queue-meta">${user.reason}</div>
          </div>
          <span class="queue-wait">Waited ${user.waited}</span>
        `;
        li.addEventListener('dragstart', onDragStart);
        li.addEventListener('dragover', onDragOver);
        li.addEventListener('dragleave', onDragLeave);
        li.addEventListener('drop', onDrop);
        li.addEventListener('dragend', onDragEnd);
        list.appendChild(li);
      });
    }

    function onDragStart(e) {
      dragSrcIndex = parseInt(this.dataset.index);
      this.classList.add('dragging');
      e.dataTransfer.effectAllowed = 'move';
    }

    function onDragOver(e) {
      e.preventDefault();
      e.dataTransfer.dropEffect = 'move';
      this.classList.add('drag-over');
    }

    function onDragLeave() {
      this.classList.remove('drag-over');
    }

    function onDrop(e) {
      e.preventDefault();
      const destIndex = parseInt(this.dataset.index);
      if (dragSrcIndex === null || dragSrcIndex === destIndex) return;

      const users = queueData[currentQueue];
      const [moved] = users.splice(dragSrcIndex, 1);
      users.splice(destIndex, 0, moved);
      renderQueueList();
    }

    function onDragEnd() {
      document.querySelectorAll('.queue-item').forEach(el => {
        el.classList.remove('dragging', 'drag-over');
      });
      dragSrcIndex = null;
    }

    function saveQueueOrder() {
      // Placeholder: in a real implementation this would POST the new order to the server
      alert('Queue order saved! (UI placeholder — no server call made)');
      closeQueueModal();
    }
  </script>
</body>
</html>
)HTML";
}

#endif