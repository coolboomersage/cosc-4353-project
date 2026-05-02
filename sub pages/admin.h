#ifndef ADMIN_H
#define ADMIN_H

// #include "../external/cpp-httplib-0.15.3/httplib.h"
#include <string>
#include <array>
#include <functional>
#include <vector>
#include <sstream>
#include "queue_management.h"

// Helper to escape strings for JSON injection
static inline std::string jsonEscape(const std::string& s) {
    std::string out;
    for (char c : s) {
        if      (c == '"')  out += "\\\"";
        else if (c == '\\') out += "\\\\";
        else if (c == '\n') out += "\\n";
        else if (c == '\r') out += "\\r";
        else if (c == '\t') out += "\\t";
        else                out += c;
    }
    return out;
}

// Serialize a vector of QueueEntry into a JS array literal
static inline std::string buildJsArray(const std::vector<QueueEntry>& entries) {
    std::string out = "[\n";
    for (const auto& e : entries) {
        std::string waited = std::to_string(e.waitTime) + " min";
        out += "        { name: \"" + jsonEscape(e.name)   + "\","
               " reason: \"" + jsonEscape(e.reason) + "\","
               " waited: \"" + waited                + "\" },\n";
    }
    out += "      ]";
    return out;
}

// ─── UnitTestBase ─────────────────────────────────────────────────────────────
// Non-templated abstract base so tests can be stored in a std::vector
class UnitTestBase {
public:
    virtual void runTest() = 0;
    virtual bool getResult() const = 0;
    virtual const std::string& getName() const = 0;
    virtual bool hasRun() const = 0;
    virtual ~UnitTestBase() = default;
};


// ─── UnitTest ─────────────────────────────────────────────────────────────────
template <typename TResult, typename TInput, std::size_t N>
class UnitTest : public UnitTestBase {
  private:
      std::string name;
      bool pass = false;
      bool ran  = false;
      std::function<TResult(std::array<TInput, N>)> func;
      std::array<TInput, N> inputVars;
      TResult intendedResult;

  public:
      bool getResult() const override { return pass; }
      bool hasRun()    const override { return ran;  }
      const std::string& getName() const override { return name; }

      void runTest() override {
          pass = (func(inputVars) == intendedResult);
          ran  = true;
      }

      UnitTest(std::string inputName,
              std::function<TResult(std::array<TInput, N>)> inputFunc,
              std::array<TInput, N> inputArray,
              TResult inputResult)
          : name(std::move(inputName))
          , func(std::move(inputFunc))
          , inputVars(inputArray)
          , intendedResult(inputResult)
          , pass(false)
          , ran(false)
      {}
};


// ─── TeeBuf ───────────────────────────────────────────────────────────────────
class TeeBuf : public std::streambuf {
  private:
      std::streambuf* original;
      std::stringstream capture;

  protected:
      int overflow(int c) override {
          if (c == EOF) return !EOF;
          capture.put((char)c);
          return original->sputc(c);
      }

  public:
      TeeBuf(std::streambuf* orig) : original(orig) {}

      std::string getCaptured() const { return capture.str(); }

      void clear() {
          capture.str("");
          capture.clear();
      }
};

TeeBuf* teeBuf;

void initCoutCapture() {
    teeBuf = new TeeBuf(std::cout.rdbuf());
    std::cout.rdbuf(teeBuf);
}


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
  
/* ===== FINAL DARK THEME OVERRIDE - BaoVersion ===== */
:root {
    --dark-bg: #1f1e2e;
    --dark-nav: #2a293d;
    --dark-card: #302f45;
    --dark-card-2: #101827;
    --dark-border: #46455f;
    --dark-text: #f4f4f8;
    --dark-muted: #b8b7c9;
    --green: #31c653;
}

html,
body {
    background: var(--dark-bg) !important;
    color: var(--dark-text) !important;
}

body,
main,
.container,
.page,
.dashboard,
.admin-dashboard,
.content,
.wrapper {
    background: var(--dark-bg) !important;
    color: var(--dark-text) !important;
}

nav,
.navbar,
.topbar,
.header,
.admin-header {
    background: var(--dark-nav) !important;
    color: var(--dark-text) !important;
    border-bottom: 1px solid #171625 !important;
}

nav a,
.navbar a,
.topbar a,
.header a,
.admin-header a {
    color: #d9d8ff !important;
}

.card,
.panel,
.box,
.stat-card,
.stats-card,
.summary-card,
.table-card,
.log-card,
.user-management,
.manage-queues,
.system-log,
.modal-content,
section {
    background: var(--dark-card) !important;
    color: var(--dark-text) !important;
    border: 1px solid var(--dark-border) !important;
    box-shadow: 0 8px 20px rgba(0, 0, 0, 0.35) !important;
}

.system-log,
.log-output,
pre,
code,
textarea.log-output {
    background: var(--dark-card-2) !important;
    color: #ffffff !important;
    border: 1px solid var(--dark-border) !important;
}

h1,
h2,
h3,
h4,
h5,
h6,
p,
label,
span,
small,
div {
    color: inherit;
}

.subtitle,
.muted,
.description,
.card-subtitle,
.help-text {
    color: var(--dark-muted) !important;
}

table {
    background: transparent !important;
    color: var(--dark-text) !important;
    border-collapse: collapse;
}

thead,
tr,
th,
td {
    background: transparent !important;
    color: var(--dark-text) !important;
    border-color: var(--dark-border) !important;
}

th {
    color: #dcdcff !important;
}

input,
select,
textarea {
    background: #25243a !important;
    color: var(--dark-text) !important;
    border: 1px solid var(--dark-border) !important;
}

input::placeholder,
textarea::placeholder {
    color: var(--dark-muted) !important;
}

button,
.btn {
    border: 1px solid var(--dark-border) !important;
}

button.primary,
.btn-primary,
.create-btn,
.export-btn,
.serve-btn {
    background: var(--green) !important;
    color: #06120a !important;
    border: none !important;
    font-weight: 700 !important;
}

button.secondary,
.btn-secondary,
.view-btn,
.close-btn,
.clear-btn,
.reset-btn,
.disable-btn,
.promote-btn {
    background: #25243a !important;
    color: var(--dark-text) !important;
    border: 1px solid var(--dark-border) !important;
}

.badge,
.status,
.pill {
    border: 1px solid rgba(255, 255, 255, 0.12) !important;
}

.status.open,
.badge.open,
.pill.open {
    background: #bdf5cc !important;
    color: #09601f !important;
}

.status.closed,
.badge.closed,
.pill.closed {
    background: #ffd1d6 !important;
    color: #9c1526 !important;
}

a {
    color: #d9d8ff !important;
}

hr {
    border-color: var(--dark-border) !important;
}

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
          <a href="/admin-dashboard"><button type="button" class="btn">Cancel</button></a>
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

      document.querySelectorAll('.field-error').forEach(el => el.classList.remove('visible'));
      document.querySelectorAll('.invalid').forEach(el => el.classList.remove('invalid'));

      const name = document.getElementById('serviceName');
      if (!name.value.trim() || name.value.trim().length > 100) {
        showError('serviceName', 'serviceNameErr');
        valid = false;
      }

      const desc = document.getElementById('description');
      if (!desc.value.trim()) {
        showError('description', 'descriptionErr');
        valid = false;
      }

      const dur = document.getElementById('duration');
      const durVal = parseInt(dur.value, 10);
      if (!dur.value || isNaN(durVal) || durVal < 1 || durVal > 480) {
        showError('duration', 'durationErr');
        valid = false;
      }

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
// Route: GET /admin-dashboard
static inline std::string adminDashboardPage(const std::string& username) {
    std::string rawLogs = teeBuf ? teeBuf->getCaptured() : "";
    std::string logHtml;
    std::istringstream stream(rawLogs);
    std::string line;
    sqlite3* db = nullptr;
    std::string dbPath = DATABASE_FILE_LOCATION;

    bool dbExists = std::ifstream(dbPath).good();

    if (sqlite3_open(dbPath.c_str(), &db) != SQLITE_OK) {
        sqlite3_close(db);
        std::cerr << "Failed to open database: " << sqlite3_errmsg(db) << std::endl;
    }

    if (!dbExists) {
        std::cout << "No database found while opening admin panel, creating default" << std::endl;
        if (!initDatabase(db)) {
            std::cout << "Failed to initialize database." << std::endl;
        }
    } else {
      std::cout << "DEBUG: successfully opened database on admin page call\n";
    }


    auto advising    = getQueueByService(db, 1);
    auto tutoring    = getQueueByService(db, 2);
    auto techSupport = getQueueByService(db, 3);

    std::string queueDataJs =
        "const queueData = {\n"
        "      'Advising':    " + buildJsArray(advising)    + ",\n"
        "      'Tutoring':    " + buildJsArray(tutoring)    + ",\n"
        "      'Tech Support':" + buildJsArray(techSupport) + ",\n"
        "    };";

    std::cout << "DEBUG: queueDataJs string:\n" + queueDataJs + "\n";

    while (std::getline(stream, line)) {
        if (!line.empty()) {
            std::string escaped;
            for (char c : line) {
                if      (c == '<') escaped += "&lt;";
                else if (c == '>') escaped += "&gt;";
                else if (c == '&') escaped += "&amp;";
                else               escaped += c;
            }
            logHtml += "<div>" + escaped + "</div>\n";
        }
    }
    if (logHtml.empty()) {
        logHtml = "<div style='color:#6b7280;'>No log output captured yet.</div>";
    }

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
    .btn.serve { background:#16a34a; color:white; border-color:#16a34a; }
    .btn.serve:hover { background:#15803d; border-color:#15803d; }
    .pill { padding: 3px 8px; border-radius: 999px; font-size: 12px; display:inline-block; }
    .pill.open { background:#dcfce7; color:#166534; }
    .pill.closed { background:#fee2e2; color:#991b1b; }

    .log-header { display:flex; justify-content:space-between; align-items:center; margin-bottom:10px; }
    .log-header-left { flex: 1; }
    .log-header-actions { display:flex; gap:6px; align-items:center; }
    .log { background: #0b1220; color:#e5e7eb; padding: 14px; border-radius: 12px; height: 220px; overflow:auto;
           font-family: ui-monospace, SFMono-Regular, Menlo, Monaco, Consolas, "Liberation Mono"; font-size: 12px; }
    .log div { margin-bottom: 8px; }
    .topbar { display:flex; justify-content:space-between; align-items:center; margin-top: 16px; }

    /* ── Unit Tests button ── */
    .btn-tests { display:inline-flex; align-items:center; gap:5px; padding:5px 10px; border-radius:7px;
                 background:#1e1b4b; color:#a5b4fc; border:1px solid #3730a3; font-size:12px;
                 cursor:pointer; text-decoration:none; font-family:inherit; }
    .btn-tests:hover { background:#312e81; color:#c7d2fe; }
    .btn-tests svg { flex-shrink:0; }

    /* ── Export Report button ── */
    .btn-export { display:inline-flex; align-items:center; gap:5px; padding:5px 10px; border-radius:7px;
                  background:#064e3b; color:#6ee7b7; border:1px solid #065f46; font-size:12px;
                  cursor:pointer; font-family:inherit; transition: background 0.12s, color 0.12s; }
    .btn-export:hover:not(:disabled) { background:#065f46; color:#a7f3d0; }
    .btn-export:disabled { opacity:0.55; cursor:not-allowed; }
    .btn-export svg { flex-shrink:0; }

    /* Modal Overlay */
    .modal-overlay {
      display: none; position: fixed; inset: 0;
      background: rgba(0,0,0,0.45); z-index: 1000;
      align-items: center; justify-content: center;
    }
    .modal-overlay.active { display: flex; }
    .modal {
      background: white; border-radius: 16px; width: 480px;
      max-width: 95vw; max-height: 80vh; display: flex;
      flex-direction: column; box-shadow: 0 8px 40px rgba(0,0,0,0.18); overflow: hidden;
    }
    .modal-header { padding: 18px 20px 14px; border-bottom: 1px solid #e5e7eb; display: flex; justify-content: space-between; align-items: center; }
    .modal-header h2 { margin: 0; font-size: 16px; }
    .modal-close { background: none; border: none; cursor: pointer; font-size: 20px; color: #6b7280; line-height: 1; padding: 0 4px; }
    .modal-close:hover { color: #111827; }
    .modal-body { padding: 16px 20px; overflow-y: auto; flex: 1; }
    .modal-footer { padding: 12px 20px; border-top: 1px solid #e5e7eb; display: flex; justify-content: flex-end; gap: 8px; }

    .queue-list { list-style: none; margin: 0; padding: 0; }
    .queue-item { display: flex; align-items: center; gap: 12px; padding: 10px 12px; border: 1px solid #e5e7eb; border-radius: 10px; margin-bottom: 8px; background: #fff; cursor: grab; user-select: none; transition: box-shadow 0.15s, background 0.15s; }
    .queue-item:active { cursor: grabbing; }
    .queue-item.dragging { opacity: 0.4; }
    .queue-item.drag-over { border-color: #6366f1; background: #eef2ff; box-shadow: 0 0 0 2px #6366f1; }
    .drag-handle { color: #9ca3af; font-size: 16px; cursor: grab; flex-shrink: 0; }
    .queue-position { width: 24px; height: 24px; background: #111827; color: white; border-radius: 50%; display: flex; align-items: center; justify-content: center; font-size: 11px; font-weight: 700; flex-shrink: 0; }
    .queue-info { flex: 1; }
    .queue-name { font-weight: 600; font-size: 14px; }
    .queue-meta { font-size: 12px; color: #6b7280; margin-top: 2px; }
    .queue-wait { font-size: 12px; color: #374151; background: #f3f4f6; padding: 3px 8px; border-radius: 999px; white-space: nowrap; }
    .drag-hint { font-size: 12px; color: #9ca3af; margin-bottom: 12px; }
  
/* ===== FINAL DARK THEME OVERRIDE - BaoVersion ===== */
:root {
    --dark-bg: #1f1e2e;
    --dark-nav: #2a293d;
    --dark-card: #302f45;
    --dark-card-2: #101827;
    --dark-border: #46455f;
    --dark-text: #f4f4f8;
    --dark-muted: #b8b7c9;
    --green: #31c653;
}

html,
body {
    background: var(--dark-bg) !important;
    color: var(--dark-text) !important;
}

body,
main,
.container,
.page,
.dashboard,
.admin-dashboard,
.content,
.wrapper {
    background: var(--dark-bg) !important;
    color: var(--dark-text) !important;
}

nav,
.navbar,
.topbar,
.header,
.admin-header {
    background: var(--dark-nav) !important;
    color: var(--dark-text) !important;
    border-bottom: 1px solid #171625 !important;
}

nav a,
.navbar a,
.topbar a,
.header a,
.admin-header a {
    color: #d9d8ff !important;
}

.card,
.panel,
.box,
.stat-card,
.stats-card,
.summary-card,
.table-card,
.log-card,
.user-management,
.manage-queues,
.system-log,
.modal-content,
section {
    background: var(--dark-card) !important;
    color: var(--dark-text) !important;
    border: 1px solid var(--dark-border) !important;
    box-shadow: 0 8px 20px rgba(0, 0, 0, 0.35) !important;
}

.system-log,
.log-output,
pre,
code,
textarea.log-output {
    background: var(--dark-card-2) !important;
    color: #ffffff !important;
    border: 1px solid var(--dark-border) !important;
}

h1,
h2,
h3,
h4,
h5,
h6,
p,
label,
span,
small,
div {
    color: inherit;
}

.subtitle,
.muted,
.description,
.card-subtitle,
.help-text {
    color: var(--dark-muted) !important;
}

table {
    background: transparent !important;
    color: var(--dark-text) !important;
    border-collapse: collapse;
}

thead,
tr,
th,
td {
    background: transparent !important;
    color: var(--dark-text) !important;
    border-color: var(--dark-border) !important;
}

th {
    color: #dcdcff !important;
}

input,
select,
textarea {
    background: #25243a !important;
    color: var(--dark-text) !important;
    border: 1px solid var(--dark-border) !important;
}

input::placeholder,
textarea::placeholder {
    color: var(--dark-muted) !important;
}

button,
.btn {
    border: 1px solid var(--dark-border) !important;
}

button.primary,
.btn-primary,
.create-btn,
.export-btn,
.serve-btn {
    background: var(--green) !important;
    color: #06120a !important;
    border: none !important;
    font-weight: 700 !important;
}

button.secondary,
.btn-secondary,
.view-btn,
.close-btn,
.clear-btn,
.reset-btn,
.disable-btn,
.promote-btn {
    background: #25243a !important;
    color: var(--dark-text) !important;
    border: 1px solid var(--dark-border) !important;
}

.badge,
.status,
.pill {
    border: 1px solid rgba(255, 255, 255, 0.12) !important;
}

.status.open,
.badge.open,
.pill.open {
    background: #bdf5cc !important;
    color: #09601f !important;
}

.status.closed,
.badge.closed,
.pill.closed {
    background: #ffd1d6 !important;
    color: #9c1526 !important;
}

a {
    color: #d9d8ff !important;
}

hr {
    border-color: var(--dark-border) !important;
}

</style>
</head>
<body>
  <header>
    <div><strong>Admin Dashboard</strong><span class="muted" style="color:#9ca3af;">Queue Management</span></div>
    <div>
      <span class="muted" style="color:#9ca3af;">Signed in as</span> <strong>)HTML" + username + R"HTML(</strong>
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
            <div class="muted">Current queue activity summary</div>
          </div>
          <a href="/admin/create-queue"><button class="btn primary">Create Queue</button></a>
        </div>

        <table>
          <thead>
            <tr><th>Queue</th><th>Status</th><th>Waiting</th><th>Avg Wait</th><th>Actions</th></tr>
          </thead>
          <tbody>
            <tr>
              <td>Advising</td>
              <td><span class="pill open">Open</span></td>
              <td>5</td><td>12 min</td>
              <td>
                <button class="btn serve" onclick="serveNext('Advising')">Serve Next</button>
                <button class="btn" onclick="openQueueModal('Advising')">View</button>
                <button class="btn">Close</button>
                <button class="btn">Clear</button>
              </td>
            </tr>
            <tr>
              <td>Tutoring</td>
              <td><span class="pill open">Open</span></td>
              <td>4</td><td>9 min</td>
              <td>
                <button class="btn serve" onclick="serveNext('Tutoring')">Serve Next</button>
                <button class="btn" onclick="openQueueModal('Tutoring')">View</button>
                <button class="btn">Close</button>
                <button class="btn">Clear</button>
              </td>
            </tr>
            <tr>
              <td>Tech Support</td>
              <td><span class="pill closed">Closed</span></td>
              <td>2</td><td>—</td>
              <td>
                <button class="btn serve" onclick="serveNext('Tech Support')">Serve Next</button>
                <button class="btn" onclick="openQueueModal('Tech Support')">View</button>
                <button class="btn">Open</button>
                <button class="btn">Clear</button>
              </td>
            </tr>
          </tbody>
        </table>
      </div>

      <!-- ── System Log widget ── -->
      <div class="card">
        <div class="log-header">
          <div class="log-header-left">
            <div style="font-weight:700;">System Log</div>
            <div class="muted">Live cout output</div>
          </div>
          <div class="log-header-actions">
            <!-- Unit Tests button -->
            <a href="/admin/unit-tests" class="btn-tests">
              <svg width="13" height="13" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2.2" stroke-linecap="round" stroke-linejoin="round">
                <polyline points="20 6 9 17 4 12"/>
              </svg>
              Unit Tests
            </a>
            <!-- Export Report button -->
            <button class="btn-export" id="exportBtn" onclick="exportReport()">
              <svg width="13" height="13" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2.2" stroke-linecap="round" stroke-linejoin="round">
                <path d="M21 15v4a2 2 0 0 1-2 2H5a2 2 0 0 1-2-2v-4"/>
                <polyline points="7 10 12 15 17 10"/>
                <line x1="12" y1="15" x2="12" y2="3"/>
              </svg>
              Export Report
            </button>
          </div>
        </div>
        <div class="log" id="systemLog">
          )HTML" + logHtml + R"HTML(
        </div>
      </div>
    </div>

    <div class="card" style="margin-top:16px;">
      <div style="font-weight:700;">User Management</div>
      <div class="muted" style="margin-bottom:10px;">Registered system users</div>
      <table>
        <thead>
          <tr><th>User</th><th>Role</th><th>Last Login</th><th>Actions</th></tr>
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
    )HTML" + queueDataJs + R"HTML(

    let currentQueue = null;
    let dragSrcIndex = null;

    function serveNext(queueName) {
      const users = queueData[queueName];
      if (!users || users.length === 0) {
        alert('No one is waiting in the ' + queueName + ' queue.');
        return;
      }
      const next = users.shift();
      alert('Now serving: ' + next.name + '\nReason: ' + next.reason);
    }

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
        li.addEventListener('dragover',  onDragOver);
        li.addEventListener('dragleave', onDragLeave);
        li.addEventListener('drop',      onDrop);
        li.addEventListener('dragend',   onDragEnd);
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
    function onDragLeave() { this.classList.remove('drag-over'); }
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
      alert('Queue order updated in the admin view.');
      closeQueueModal();
    }

    // ── Export Report ──────────────────────────────────────────────────────────
    // POSTs to /admin/export-report. The server calls exportDatabaseReport(),
    // writes the xlsx to a temp path, then responds with the raw file bytes and
    // Content-Disposition: attachment so the browser saves it automatically.
    async function exportReport() {
      const btn = document.getElementById('exportBtn');
      const originalHTML = btn.innerHTML;

      btn.disabled = true;
      btn.innerHTML = `
        <svg width="13" height="13" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2.2" stroke-linecap="round" stroke-linejoin="round">
          <circle cx="12" cy="12" r="10"/>
          <polyline points="12 6 12 12 16 14"/>
        </svg>
        Exporting…`;

      try {
        const res = await fetch('/admin/export-report', { method: 'POST' });

        if (!res.ok) {
          // Surface any plain-text error message from the server
          const msg = await res.text().catch(() => '');
          throw new Error(msg || 'Server returned HTTP ' + res.status);
        }

        // Read the CSV data and trigger a browser download
        const blob = await res.blob();
        const url  = URL.createObjectURL(blob);
        const a    = document.createElement('a');
        a.href     = url;
        
        // Build a timestamped filename so repeated exports don't collide
        const ts   = new Date().toISOString().slice(0, 19).replace(/[:T]/g, '-');
        a.download = 'queue_report_' + ts + '.xlsx';

        document.body.appendChild(a);
        a.click();
        document.body.removeChild(a);
        URL.revokeObjectURL(url);

      } catch (err) {
        alert('Export failed:\n' + err.message);
      } finally {
        btn.disabled  = false;
        btn.innerHTML = originalHTML;
      }
    }
  </script>
</body>
</html>
)HTML";
}


// ─── Unit Tests Page ──────────────────────────────────────────────────────────
// Route: GET /admin/unit-tests
static inline std::string unitTestsPage(
    const std::string& username,
    const std::vector<UnitTestBase*>& tests)
{
    std::string cardsHtml;
    for (std::size_t i = 0; i < tests.size(); ++i) {
        // Escape the test name for HTML
        std::string safeName;
        for (char c : tests[i]->getName()) {
            if      (c == '<') safeName += "&lt;";
            else if (c == '>') safeName += "&gt;";
            else if (c == '&') safeName += "&amp;";
            else if (c == '"') safeName += "&quot;";
            else               safeName += c;
        }

        const std::string idx = std::to_string(i);

        cardsHtml += R"HTML(
        <div class="test-card" id="card-)HTML" + idx + R"HTML(">
          <div class="test-left">
            <span class="test-index">)HTML" + idx + R"HTML(</span>
            <span class="test-name">)HTML" + safeName + R"HTML(</span>
          </div>
          <div class="test-right">
            <div class="indicator" id="indicator-)HTML" + idx + R"HTML(" data-state="untested">
              <span class="dot"></span>
              <span class="ind-label">Not Tested</span>
            </div>
            <button class="run-btn" id="btn-)HTML" + idx + R"HTML(" onclick="runTest()HTML" + idx + R"HTML()">
              Run
            </button>
          </div>
        </div>
)HTML";
    }

    if (cardsHtml.empty()) {
        cardsHtml = R"HTML(
        <div style="text-align:center; padding:40px; color:#6b7280; font-size:14px;">
          No unit tests registered. Add tests to your vector in main.cpp.
        </div>
)HTML";
    }

    const std::string totalTests = std::to_string(tests.size());

    return R"HTML(
<!doctype html>
<html>
<head>
  <meta charset="utf-8" />
  <title>Unit Tests — Admin</title>
  <style>
    *, *::before, *::after { box-sizing: border-box; }
    body { font-family: Arial, sans-serif; margin: 0; background: #f6f7fb; color: #111827; }

    /* ── Header ── */
    header {
      background: #111827; color: white;
      padding: 16px 24px; display: flex;
      justify-content: space-between; align-items: center;
    }
    header a { color: #c7d2fe; text-decoration: none; margin-left: 12px; }

    /* ── Layout ── */
    .container { padding: 24px; max-width: 860px; margin: 0 auto; }

    /* ── Summary bar ── */
    .summary {
      display: grid; grid-template-columns: repeat(4, 1fr);
      gap: 12px; margin-bottom: 20px;
    }
    .stat-card {
      background: white; border-radius: 10px;
      padding: 14px 16px; box-shadow: 0 1px 6px rgba(0,0,0,0.07);
      border-top: 3px solid transparent;
    }
    .stat-card.total  { border-color: #6366f1; }
    .stat-card.passed { border-color: #16a34a; }
    .stat-card.failed { border-color: #dc2626; }
    .stat-card.pending{ border-color: #d97706; }
    .stat-label { font-size: 11px; color: #6b7280; text-transform: uppercase; letter-spacing: .05em; }
    .stat-value { font-size: 26px; font-weight: 700; margin-top: 4px; }
    .stat-card.total   .stat-value { color: #4f46e5; }
    .stat-card.passed  .stat-value { color: #15803d; }
    .stat-card.failed  .stat-value { color: #b91c1c; }
    .stat-card.pending .stat-value { color: #b45309; }

    /* ── Toolbar ── */
    .toolbar {
      display: flex; justify-content: space-between; align-items: center;
      margin-bottom: 14px;
    }
    .toolbar h2 { margin: 0; font-size: 16px; }
    .toolbar-actions { display: flex; gap: 8px; }

    .btn {
      padding: 7px 14px; border-radius: 8px; border: 1px solid #e5e7eb;
      background: #fff; cursor: pointer; font-size: 13px; font-family: inherit;
      transition: background 0.12s;
    }
    .btn:hover { background: #f3f4f6; }
    .btn.primary { background: #111827; color: white; border-color: #111827; }
    .btn.primary:hover { background: #1f2937; }
    .btn.run-all { background: #4f46e5; color: white; border-color: #4338ca; }
    .btn.run-all:hover { background: #4338ca; }
    .btn:disabled { opacity: 0.5; cursor: not-allowed; }

    /* ── Test cards ── */
    .tests-container {
      background: white; border-radius: 12px;
      box-shadow: 0 2px 10px rgba(0,0,0,0.06); overflow: hidden;
    }
    .test-card {
      display: flex; align-items: center; justify-content: space-between;
      padding: 14px 18px; border-bottom: 1px solid #f3f4f6;
      transition: background 0.12s;
    }
    .test-card:last-child { border-bottom: none; }
    .test-card:hover { background: #fafafa; }

    .test-left { display: flex; align-items: center; gap: 12px; }
    .test-index {
      width: 26px; height: 26px; background: #f3f4f6; color: #374151;
      border-radius: 6px; display: flex; align-items: center; justify-content: center;
      font-size: 11px; font-weight: 700; flex-shrink: 0;
      font-family: ui-monospace, Menlo, Monaco, Consolas, monospace;
    }
    .test-name { font-size: 14px; font-weight: 500; color: #111827; }

    .test-right { display: flex; align-items: center; gap: 12px; }

    /* ── Indicator ── */
    .indicator {
      display: inline-flex; align-items: center; gap: 6px;
      padding: 4px 10px; border-radius: 999px; font-size: 12px;
      font-weight: 600; transition: all 0.25s; min-width: 100px;
      justify-content: center;
    }
    .indicator .dot {
      width: 7px; height: 7px; border-radius: 50%; flex-shrink: 0;
      transition: background 0.25s;
    }

    /* untested */
    .indicator[data-state="untested"] {
      background: #f3f4f6; color: #6b7280; border: 1px solid #e5e7eb;
    }
    .indicator[data-state="untested"] .dot { background: #9ca3af; }

    /* pass */
    .indicator[data-state="pass"] {
      background: #dcfce7; color: #15803d; border: 1px solid #bbf7d0;
    }
    .indicator[data-state="pass"] .dot { background: #16a34a; }

    /* fail */
    .indicator[data-state="fail"] {
      background: #fef2f2; color: #b91c1c; border: 1px solid #fecaca;
    }
    .indicator[data-state="fail"] .dot { background: #dc2626; }

    /* running (spinner-like pulse) */
    .indicator[data-state="running"] {
      background: #eff6ff; color: #1d4ed8; border: 1px solid #bfdbfe;
    }
    .indicator[data-state="running"] .dot {
      background: #3b82f6;
      animation: pulse 0.8s ease-in-out infinite alternate;
    }
    @keyframes pulse { from { opacity: 0.3; } to { opacity: 1; } }

    /* ── Run button ── */
    .run-btn {
      padding: 5px 14px; border-radius: 7px; border: 1px solid #e5e7eb;
      background: #fff; cursor: pointer; font-size: 12px; font-weight: 600;
      font-family: inherit; color: #374151; transition: all 0.12s;
    }
    .run-btn:hover:not(:disabled) { background: #111827; color: white; border-color: #111827; }
    .run-btn:disabled { opacity: 0.45; cursor: not-allowed; }

    /* ── Empty state ── */
    .empty { text-align:center; padding:40px; color:#6b7280; font-size:14px; }
  
/* ===== FINAL DARK THEME OVERRIDE - BaoVersion ===== */
:root {
    --dark-bg: #1f1e2e;
    --dark-nav: #2a293d;
    --dark-card: #302f45;
    --dark-card-2: #101827;
    --dark-border: #46455f;
    --dark-text: #f4f4f8;
    --dark-muted: #b8b7c9;
    --green: #31c653;
}

html,
body {
    background: var(--dark-bg) !important;
    color: var(--dark-text) !important;
}

body,
main,
.container,
.page,
.dashboard,
.admin-dashboard,
.content,
.wrapper {
    background: var(--dark-bg) !important;
    color: var(--dark-text) !important;
}

nav,
.navbar,
.topbar,
.header,
.admin-header {
    background: var(--dark-nav) !important;
    color: var(--dark-text) !important;
    border-bottom: 1px solid #171625 !important;
}

nav a,
.navbar a,
.topbar a,
.header a,
.admin-header a {
    color: #d9d8ff !important;
}

.card,
.panel,
.box,
.stat-card,
.stats-card,
.summary-card,
.table-card,
.log-card,
.user-management,
.manage-queues,
.system-log,
.modal-content,
section {
    background: var(--dark-card) !important;
    color: var(--dark-text) !important;
    border: 1px solid var(--dark-border) !important;
    box-shadow: 0 8px 20px rgba(0, 0, 0, 0.35) !important;
}

.system-log,
.log-output,
pre,
code,
textarea.log-output {
    background: var(--dark-card-2) !important;
    color: #ffffff !important;
    border: 1px solid var(--dark-border) !important;
}

h1,
h2,
h3,
h4,
h5,
h6,
p,
label,
span,
small,
div {
    color: inherit;
}

.subtitle,
.muted,
.description,
.card-subtitle,
.help-text {
    color: var(--dark-muted) !important;
}

table {
    background: transparent !important;
    color: var(--dark-text) !important;
    border-collapse: collapse;
}

thead,
tr,
th,
td {
    background: transparent !important;
    color: var(--dark-text) !important;
    border-color: var(--dark-border) !important;
}

th {
    color: #dcdcff !important;
}

input,
select,
textarea {
    background: #25243a !important;
    color: var(--dark-text) !important;
    border: 1px solid var(--dark-border) !important;
}

input::placeholder,
textarea::placeholder {
    color: var(--dark-muted) !important;
}

button,
.btn {
    border: 1px solid var(--dark-border) !important;
}

button.primary,
.btn-primary,
.create-btn,
.export-btn,
.serve-btn {
    background: var(--green) !important;
    color: #06120a !important;
    border: none !important;
    font-weight: 700 !important;
}

button.secondary,
.btn-secondary,
.view-btn,
.close-btn,
.clear-btn,
.reset-btn,
.disable-btn,
.promote-btn {
    background: #25243a !important;
    color: var(--dark-text) !important;
    border: 1px solid var(--dark-border) !important;
}

.badge,
.status,
.pill {
    border: 1px solid rgba(255, 255, 255, 0.12) !important;
}

.status.open,
.badge.open,
.pill.open {
    background: #bdf5cc !important;
    color: #09601f !important;
}

.status.closed,
.badge.closed,
.pill.closed {
    background: #ffd1d6 !important;
    color: #9c1526 !important;
}

a {
    color: #d9d8ff !important;
}

hr {
    border-color: var(--dark-border) !important;
}

/* ===== UNIT TEST DARK THEME FIX ===== */
.tests-container,
.test-card,
.toolbar,
.summary,
.stat-card,
.test-left,
.test-right {
    background: #302f45 !important;
    color: #f4f4f8 !important;
    border-color: #46455f !important;
}

</style>
</head>
<body>
  <header>
    <div>
      <strong>Admin Dashboard</strong>
      <span style="color:#9ca3af; font-size:12px;"> / Unit Tests</span>
    </div>
    <div>
      <span style="color:#9ca3af; font-size:12px;">Signed in as</span>
      <strong>)HTML" + username + R"HTML(</strong>
      <a href="/admin-dashboard">Dashboard</a>
      <a href="/">Home</a>
      <a href="/login">Logout</a>
    </div>
  </header>

  <div class="container">

    <!-- Summary Bar -->
    <div class="summary">
      <div class="stat-card total">
        <div class="stat-label">Total</div>
        <div class="stat-value" id="statTotal">)HTML" + totalTests + R"HTML(</div>
      </div>
      <div class="stat-card passed">
        <div class="stat-label">Passed</div>
        <div class="stat-value" id="statPassed">0</div>
      </div>
      <div class="stat-card failed">
        <div class="stat-label">Failed</div>
        <div class="stat-value" id="statFailed">0</div>
      </div>
      <div class="stat-card pending">
        <div class="stat-label">Not Tested</div>
        <div class="stat-value" id="statPending">)HTML" + totalTests + R"HTML(</div>
      </div>
    </div>

    <!-- Toolbar -->
    <div class="toolbar">
      <h2>Test Suite <span style="color:#6b7280; font-weight:400; font-size:13px;">
        — )HTML" + totalTests + R"HTML( test(s) registered</span>
      </h2>
      <div class="toolbar-actions">
        <button class="btn" onclick="resetAll()">Reset All</button>
        <button class="btn run-all" onclick="runAll()">&#9654;&nbsp; Run All</button>
      </div>
    </div>

    <!-- Test Cards -->
    <div class="tests-container" id="testsContainer">
)HTML" + cardsHtml + R"HTML(
    </div>
  </div>

  <script>
    const TOTAL = )HTML" + totalTests + R"HTML(;

    // State map: index -> 'untested' | 'pass' | 'fail' | 'running'
    const states = {};
    for (let i = 0; i < TOTAL; i++) states[i] = 'untested';

    const labelText = {
      untested: 'Not Tested',
      pass:     'Pass',
      fail:     'Fail',
      running:  'Running…',
    };

    function setIndicator(idx, state) {
      states[idx] = state;
      const ind = document.getElementById('indicator-' + idx);
      if (!ind) return;
      ind.setAttribute('data-state', state);
      ind.querySelector('.ind-label').textContent = labelText[state] || state;
      updateSummary();
    }

    function updateSummary() {
      let passed = 0, failed = 0, pending = 0;
      for (let i = 0; i < TOTAL; i++) {
        if      (states[i] === 'pass')     passed++;
        else if (states[i] === 'fail')     failed++;
        else if (states[i] === 'untested') pending++;
      }
      document.getElementById('statPassed').textContent  = passed;
      document.getElementById('statFailed').textContent  = failed;
      document.getElementById('statPending').textContent = pending;
    }

    async function runTest(idx) {
      const btn = document.getElementById('btn-' + idx);
      btn.disabled = true;
      setIndicator(idx, 'running');

      try {
        const res = await fetch('/admin/unit-tests/run/' + idx, { method: 'POST' });
        if (!res.ok) throw new Error('HTTP ' + res.status);
        const data = await res.json();          // expects { "pass": true|false }
        setIndicator(idx, data.pass ? 'pass' : 'fail');
      } catch (err) {
        console.error('Test run error:', err);
        setIndicator(idx, 'fail');
      } finally {
        btn.disabled = false;
      }
    }

    async function runAll() {
      for (let i = 0; i < TOTAL; i++) {
        await runTest(i);
      }
    }

    function resetAll() {
      for (let i = 0; i < TOTAL; i++) {
        setIndicator(i, 'untested');
        const btn = document.getElementById('btn-' + i);
        if (btn) btn.disabled = false;
      }
    }
  </script>
</body>
</html>
)HTML";
}

#endif  // ADMIN_H
