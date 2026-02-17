#ifndef account_h
#define account_h
#include "../external/cpp-httplib-0.15.3/httplib.h"
#include "../external/sqlite-amalgamation-3510200/sqlite3.h"
#include "../external/phc-winner-argon2-20190702/include/argon2.h"
#include <cstring>
#include <filesystem>
#include <iomanip>
#include <sstream>
#include <random>
#include <vector>

#ifdef _WIN32
    #include <windows.h>
#else
    #include <unistd.h>
    #include <limits.h>
#endif

// Global session variable to track logged-in user (0 = no user logged in)
int currentUserId = 0;

std::string getExecutableDirectory() {
    char buffer[1024];
    
    #ifdef _WIN32
        // Windows implementation
        GetModuleFileNameA(NULL, buffer, sizeof(buffer));
    #else
        // Linux implementation
        ssize_t len = readlink("/proc/self/exe", buffer, sizeof(buffer) - 1);
        if (len != -1) {
            buffer[len] = '\0';
        } else {
            // Fallback if /proc/self/exe doesn't work
            return ".";
        }
    #endif
    
    std::filesystem::path exePath(buffer);
    return exePath.parent_path().string();
}


std::string hashPassword(const std::string& password) {
    const uint32_t HASH_LEN = 32;
    const uint32_t SALT_LEN = 16;
    const uint32_t T_COST = 2;
    const uint32_t M_COST = 65536;
    const uint32_t PARALLELISM = 1;
    
    // Generate random salt
    uint8_t salt[SALT_LEN];
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, 255);
    for (int i = 0; i < SALT_LEN; i++) {
        salt[i] = dis(gen);
    }
    
    uint8_t hash[HASH_LEN];
    int result = argon2id_hash_raw(
        T_COST,
        M_COST,
        PARALLELISM,
        password.c_str(),
        password.length(),
        salt,
        SALT_LEN,
        hash,
        HASH_LEN
    );
    
    if (result != ARGON2_OK) {
        std::cerr << "Argon2 hashing failed: " << argon2_error_message(result) << std::endl;
        return "";
    }
    
    std::stringstream ss;
    for (int i = 0; i < SALT_LEN; i++) {
        ss << std::hex << std::setw(2) << std::setfill('0') << (int)salt[i];
    }
    ss << ":";
    for (int i = 0; i < HASH_LEN; i++) {
        ss << std::hex << std::setw(2) << std::setfill('0') << (int)hash[i];
    }
    
    return ss.str();
}

bool verifyPassword(const std::string& password, const std::string& storedHash) {
    const uint32_t HASH_LEN = 32;
    const uint32_t SALT_LEN = 16;
    const uint32_t T_COST = 2;
    const uint32_t M_COST = 65536;
    const uint32_t PARALLELISM = 1;
    
    size_t separatorPos = storedHash.find(':');
    if (separatorPos == std::string::npos) {
        return false;
    }
    
    std::string saltHex = storedHash.substr(0, separatorPos);
    std::string hashHex = storedHash.substr(separatorPos + 1);
    uint8_t salt[SALT_LEN];
    uint8_t expectedHash[HASH_LEN];
    
    for (int i = 0; i < SALT_LEN; i++) {
        std::string byte = saltHex.substr(i * 2, 2);
        salt[i] = std::stoi(byte, nullptr, 16);
    }
    
    for (int i = 0; i < HASH_LEN; i++) {
        std::string byte = hashHex.substr(i * 2, 2);
        expectedHash[i] = std::stoi(byte, nullptr, 16);
    }
    
    uint8_t computedHash[HASH_LEN];
    int result = argon2id_hash_raw(
        T_COST,
        M_COST,
        PARALLELISM,
        password.c_str(),
        password.length(),
        salt,
        SALT_LEN,
        computedHash,
        HASH_LEN
    );
    
    if (result != ARGON2_OK) {
        return false;
    }
    
    return std::memcmp(expectedHash, computedHash, HASH_LEN) == 0;
}

bool isValidEmail(const std::string& email) {
    size_t atPos = email.find('@');
    if (atPos == std::string::npos || atPos == 0 || atPos == email.length() - 1) {
        return false;
    }
    
    size_t lastDotPos = email.find_last_of('.');
    if (lastDotPos == std::string::npos || lastDotPos <= atPos + 1 || lastDotPos == email.length() - 1) {
        return false;
    }
    
    std::string suffix = email.substr(lastDotPos + 1);
    std::vector<std::string> validSuffixes = {"com", "cc", "org", "net", "edu", "gov", "io", "co", "uk", "ca", "au"};
    
    for (const auto& validSuffix : validSuffixes) {
        if (suffix == validSuffix) {
            return true;
        }
    }
    
    return false;
}

struct LoginResult {
    bool success;
    int userId;
};

struct CreateAccountResult {
    bool success;
    std::string message;
    int userId;
};

LoginResult checkCredentials(const std::string& username, const std::string& password) {
    LoginResult result = {false, 0};
    sqlite3* db;
    int rc;
    char* errMsg = nullptr;

    std::string exeDir = getExecutableDirectory();
    std::string dbPath = exeDir + "/accounts.db";
    
    std::cout << "Attempting to open database at: " << dbPath << std::endl;

    bool dbExists = std::ifstream(dbPath).good();
    
    rc = sqlite3_open(dbPath.c_str(), &db);
    
    if (rc) {
        std::cout << "Can't open database: " << sqlite3_errmsg(db) << std::endl;
        sqlite3_close(db);
        return result;
    } else {
        std::cout << "Opened database successfully at: " << dbPath << std::endl;
    }
    
    if (!dbExists) {
        std::cout << "Database is new, initializing with default data..." << std::endl;
        
        const char* createTableSQL = 
            "CREATE TABLE IF NOT EXISTS accounts ("
            "id INTEGER PRIMARY KEY AUTOINCREMENT, "
            "username TEXT NOT NULL, "
            "hash TEXT UNIQUE NOT NULL, "
            "email TEXT NOT NULL, "
            "auth INTEGER NOT NULL);";
        
        rc = sqlite3_exec(db, createTableSQL, nullptr, nullptr, &errMsg);
        if (rc != SQLITE_OK) {
            std::cout << "Failed to create table: " << errMsg << std::endl;
            sqlite3_free(errMsg);
            sqlite3_close(db);
            return result;
        }
        
        std::string defaultPasswordHash = hashPassword("root123");
        
        std::string insertDefaultSQL = 
            "INSERT INTO accounts (username, hash, email, auth) VALUES ('root', '" 
            + defaultPasswordHash + "', 'root@root', 3);";
        
        rc = sqlite3_exec(db, insertDefaultSQL.c_str(), nullptr, nullptr, &errMsg);
        if (rc != SQLITE_OK) {
            std::cout << "Failed to insert default account: " << errMsg << std::endl;
            sqlite3_free(errMsg);
            sqlite3_close(db);
            return result;
        }
        
        std::cout << "Default account created (username: root, password: root123)" << std::endl;
    }
    
    sqlite3_stmt* stmt;
    const char* selectSQL = "SELECT id, hash FROM accounts WHERE username = ?;";
    
    rc = sqlite3_prepare_v2(db, selectSQL, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        std::cout << "Failed to prepare statement: " << sqlite3_errmsg(db) << std::endl;
        sqlite3_close(db);
        return result;
    }
    
    sqlite3_bind_text(stmt, 1, username.c_str(), -1, SQLITE_TRANSIENT);
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        int userId = sqlite3_column_int(stmt, 0);
        const char* storedHash = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        
        bool credentialsValid = verifyPassword(password, storedHash);
        
        if (credentialsValid) {
            std::cout << "Credentials valid for user: " << username << " (ID: " << userId << ")" << std::endl;
            result.success = true;
            result.userId = userId;
            currentUserId = userId;
        } else {
            std::cout << "Invalid password for user: " << username << std::endl;
        }
    } else {
        std::cout << "User not found: " << username << std::endl;
    }
    
    sqlite3_finalize(stmt);
    sqlite3_close(db);
    
    return result;
}

CreateAccountResult createAccount(const std::string& email, const std::string& password) {
    CreateAccountResult result = {false, "", 0};
    
    if (!isValidEmail(email)) {
        result.message = "Invalid email format. Must include @ and end with valid suffix (.com, .cc, etc.)";
        return result;
    }
    
    sqlite3* db;
    int rc;
    char* errMsg = nullptr;

    std::string exeDir = getExecutableDirectory();
    std::string dbPath = exeDir + "/accounts.db";
    
    rc = sqlite3_open(dbPath.c_str(), &db);
    
    if (rc) {
        result.message = "Database error";
        sqlite3_close(db);
        return result;
    }
    
    // Check if email/username already exists
    sqlite3_stmt* checkStmt;
    const char* checkSQL = "SELECT id FROM accounts WHERE username = ?;";
    
    rc = sqlite3_prepare_v2(db, checkSQL, -1, &checkStmt, nullptr);
    if (rc != SQLITE_OK) {
        result.message = "Database error";
        sqlite3_close(db);
        return result;
    }
    
    sqlite3_bind_text(checkStmt, 1, email.c_str(), -1, SQLITE_TRANSIENT);
    if (sqlite3_step(checkStmt) == SQLITE_ROW) {
        result.message = "Email already registered";
        sqlite3_finalize(checkStmt);
        sqlite3_close(db);
        return result;
    }
    sqlite3_finalize(checkStmt);

    if (password.length() < 5){
        result.message = "password must be at least 5 characters";
        sqlite3_close(db);
        return result;
    }
    
    std::string passwordHash = hashPassword(password);
    if (passwordHash.empty()) {
        result.message = "Password hashing failed";
        sqlite3_close(db);
        return result;
    }
    
    // Insert new account (username = email, auth level = 1 for normal users)
    sqlite3_stmt* insertStmt;
    const char* insertSQL = "INSERT INTO accounts (username, hash, email, auth) VALUES (?, ?, ?, 1);";
    
    rc = sqlite3_prepare_v2(db, insertSQL, -1, &insertStmt, nullptr);
    if (rc != SQLITE_OK) {
        result.message = "Database error";
        sqlite3_close(db);
        return result;
    }
    
    sqlite3_bind_text(insertStmt, 1, email.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(insertStmt, 2, passwordHash.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(insertStmt, 3, email.c_str(), -1, SQLITE_TRANSIENT);
    
    rc = sqlite3_step(insertStmt);
    if (rc != SQLITE_DONE) {
        result.message = "Failed to create account";
        sqlite3_finalize(insertStmt);
        sqlite3_close(db);
        return result;
    }
    
    result.userId = sqlite3_last_insert_rowid(db);
    result.success = true;
    result.message = "Account created successfully";
    
    sqlite3_finalize(insertStmt);
    sqlite3_close(db);
    
    std::cout << "New account created: " << email << " (ID: " << result.userId << ")" << std::endl;
    
    return result;
}

std::string getUsernameById(int userId) {
    if (userId == 0) {
        return "";
    }
    
    sqlite3* db;
    int rc;
    std::string username = "";

    std::string exeDir = getExecutableDirectory();
    std::string dbPath = exeDir + "/accounts.db";
    
    rc = sqlite3_open(dbPath.c_str(), &db);
    
    if (rc) {
        std::cout << "Can't open database: " << sqlite3_errmsg(db) << std::endl;
        sqlite3_close(db);
        return username;
    }
    
    sqlite3_stmt* stmt;
    const char* selectSQL = "SELECT username FROM accounts WHERE id = ?;";
    
    rc = sqlite3_prepare_v2(db, selectSQL, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        std::cout << "Failed to prepare statement: " << sqlite3_errmsg(db) << std::endl;
        sqlite3_close(db);
        return username;
    }
    
    sqlite3_bind_int(stmt, 1, userId);
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        username = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
    }
    
    sqlite3_finalize(stmt);
    sqlite3_close(db);
    
    return username;
}

int getAuthLevelById(int userId) {
    if (userId == 0) {
        return 0;
    }
    
    sqlite3* db;
    int rc;
    int authLevel = 0;

    std::string exeDir = getExecutableDirectory();
    std::string dbPath = exeDir + "/accounts.db";
    
    rc = sqlite3_open(dbPath.c_str(), &db);
    
    if (rc) {
        std::cout << "Can't open database: " << sqlite3_errmsg(db) << std::endl;
        sqlite3_close(db);
        return authLevel;
    }
    
    sqlite3_stmt* stmt;
    const char* selectSQL = "SELECT auth FROM accounts WHERE id = ?;";
    
    rc = sqlite3_prepare_v2(db, selectSQL, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        std::cout << "Failed to prepare statement: " << sqlite3_errmsg(db) << std::endl;
        sqlite3_close(db);
        return authLevel;
    }
    
    sqlite3_bind_int(stmt, 1, userId);
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        authLevel = sqlite3_column_int(stmt, 0);
    }
    
    sqlite3_finalize(stmt);
    sqlite3_close(db);
    
    return authLevel;
}

std::string createAccountPage() {
    return R"(
        <!DOCTYPE html>
        <html>
        <head>
            <title>Create Account</title>
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
                    display: flex;
                    justify-content: center;
                    align-items: center;
                    height: 100vh;
                }
                
                .login-card {
                    background: #2b2b40;
                    padding: 40px;
                    border-radius: 12px;
                    box-shadow: 0 0 20px rgba(0,0,0,0.4);
                    width: 100%;
                    max-width: 400px;
                }
                
                h1 {
                    color: #4CAF50;
                    margin-bottom: 30px;
                    text-align: center;
                }
                
                .input-group {
                    margin-bottom: 20px;
                }
                
                label {
                    display: block;
                    margin-bottom: 8px;
                    color: #a0a0b0;
                    font-size: 14px;
                }
                
                input[type="text"],
                input[type="password"],
                input[type="email"] {
                    width: 100%;
                    padding: 12px;
                    background: #1e1e2f;
                    border: 2px solid #3a3a52;
                    border-radius: 6px;
                    color: white;
                    font-size: 16px;
                    transition: border-color 0.3s;
                }
                
                input[type="text"]:focus,
                input[type="password"]:focus,
                input[type="email"]:focus {
                    outline: none;
                    border-color: #4CAF50;
                }
                
                .login-button {
                    width: 100%;
                    padding: 12px;
                    background: #4CAF50;
                    border: none;
                    border-radius: 6px;
                    color: white;
                    font-size: 16px;
                    font-weight: bold;
                    cursor: pointer;
                    transition: background 0.3s;
                    margin-top: 10px;
                }
                
                .login-button:hover {
                    background: #45a049;
                }
                
                .message {
                    text-align: center;
                    margin-top: 20px;
                    font-size: 14px;
                    opacity: 0;
                    transition: opacity 0.3s;
                }
                
                .message.error {
                    color: #ff4444;
                }
                
                .message.success {
                    color: #4CAF50;
                }
                
                .message.show {
                    opacity: 1;
                }
                
                .message.error.show {
                    animation: glowError 0.5s ease-in-out;
                }
                
                .message.success.show {
                    animation: glowSuccess 0.5s ease-in-out;
                }
                
                @keyframes glowError {
                    0% {
                        text-shadow: 0 0 0px #ff4444;
                    }
                    50% {
                        text-shadow: 0 0 20px #ff4444, 0 0 30px #ff4444;
                    }
                    100% {
                        text-shadow: 0 0 0px #ff4444;
                    }
                }
                
                @keyframes glowSuccess {
                    0% {
                        text-shadow: 0 0 0px #4CAF50;
                    }
                    50% {
                        text-shadow: 0 0 20px #4CAF50, 0 0 30px #4CAF50;
                    }
                    100% {
                        text-shadow: 0 0 0px #4CAF50;
                    }
                }
                
                .back-link {
                    display: block;
                    text-align: center;
                    margin-top: 20px;
                    color: #4CAF50;
                    text-decoration: none;
                    font-size: 14px;
                }
                
                .back-link:hover {
                    text-decoration: underline;
                }
                
                .info-text {
                    font-size: 12px;
                    color: #a0a0b0;
                    margin-top: 5px;
                }
            </style>
        </head>
        <body>
            <div class="login-card">
                <h1>Create Account</h1>
                <form id="createAccountForm">
                    <div class="input-group">
                        <label for="email">Email</label>
                        <input type="email" id="email" name="email" required>
                        <div class="info-text">Must include @ and valid suffix (.com, .cc, etc.)</div>
                    </div>
                    <div class="input-group">
                        <label for="password">Password</label>
                        <input type="password" id="password" name="password" required>
                    </div>
                    <button type="submit" class="login-button">Create Account</button>
                    <div id="message" class="message"></div>
                </form>
                <a href="/login" class="back-link">Back to Login</a>
            </div>
            
            <script>
                document.getElementById('createAccountForm').addEventListener('submit', async function(e) {
                    e.preventDefault();
                    
                    const email = document.getElementById('email').value;
                    const password = document.getElementById('password').value;
                    const messageDiv = document.getElementById('message');
                    
                    try {
                        // Send credentials to backend
                        const response = await fetch('/api/create-account', {
                            method: 'POST',
                            headers: {
                                'Content-Type': 'application/json',
                            },
                            body: JSON.stringify({ email, password })
                        });
                        
                        const result = await response.json();
                        
                        if (!result.success) {
                            // Show error message
                            messageDiv.textContent = result.message || 'Failed to create account';
                            messageDiv.className = 'message error';
                            void messageDiv.offsetWidth; // Force reflow
                            messageDiv.classList.add('show');
                            
                            setTimeout(() => {
                                messageDiv.classList.remove('show');
                            }, 3000);
                        } else {
                            // Show success message
                            messageDiv.textContent = 'Account created successfully!';
                            messageDiv.className = 'message success';
                            void messageDiv.offsetWidth; // Force reflow
                            messageDiv.classList.add('show');
                            
                            // Redirect to login after a short delay
                            setTimeout(() => {
                                window.location.href = '/login';
                            }, 1500);
                        }
                    } catch (error) {
                        console.error('Create account error:', error);
                        messageDiv.textContent = 'Server error. Please try again.';
                        messageDiv.className = 'message error';
                        messageDiv.classList.add('show');
                    }
                });
            </script>
        </body>
        </html>
    )";
}

std::string loginPage() {
    return R"(
        <!DOCTYPE html>
        <html>
        <head>
            <title>Login</title>
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
                    display: flex;
                    justify-content: center;
                    align-items: center;
                    height: 100vh;
                }
                
                .login-card {
                    background: #2b2b40;
                    padding: 40px;
                    border-radius: 12px;
                    box-shadow: 0 0 20px rgba(0,0,0,0.4);
                    width: 100%;
                    max-width: 400px;
                }
                
                h1 {
                    color: #4CAF50;
                    margin-bottom: 30px;
                    text-align: center;
                }
                
                .input-group {
                    margin-bottom: 20px;
                }
                
                label {
                    display: block;
                    margin-bottom: 8px;
                    color: #a0a0b0;
                    font-size: 14px;
                }
                
                input[type="text"],
                input[type="password"] {
                    width: 100%;
                    padding: 12px;
                    background: #1e1e2f;
                    border: 2px solid #3a3a52;
                    border-radius: 6px;
                    color: white;
                    font-size: 16px;
                    transition: border-color 0.3s;
                }
                
                input[type="text"]:focus,
                input[type="password"]:focus {
                    outline: none;
                    border-color: #4CAF50;
                }
                
                .login-button {
                    width: 100%;
                    padding: 12px;
                    background: #4CAF50;
                    border: none;
                    border-radius: 6px;
                    color: white;
                    font-size: 16px;
                    font-weight: bold;
                    cursor: pointer;
                    transition: background 0.3s;
                    margin-top: 10px;
                }
                
                .login-button:hover {
                    background: #45a049;
                }
                
                .create-account-button {
                    width: 100%;
                    padding: 12px;
                    background: #2196F3;
                    border: none;
                    border-radius: 6px;
                    color: white;
                    font-size: 16px;
                    font-weight: bold;
                    cursor: pointer;
                    transition: background 0.3s;
                    margin-top: 10px;
                }
                
                .create-account-button:hover {
                    background: #1976D2;
                }
                
                .message {
                    text-align: center;
                    margin-top: 20px;
                    font-size: 14px;
                    opacity: 0;
                    transition: opacity 0.3s;
                }
                
                .message.error {
                    color: #ff4444;
                }
                
                .message.success {
                    color: #4CAF50;
                }
                
                .message.show {
                    opacity: 1;
                }
                
                .message.error.show {
                    animation: glowError 0.5s ease-in-out;
                }
                
                .message.success.show {
                    animation: glowSuccess 0.5s ease-in-out;
                }
                
                @keyframes glowError {
                    0% {
                        text-shadow: 0 0 0px #ff4444;
                    }
                    50% {
                        text-shadow: 0 0 20px #ff4444, 0 0 30px #ff4444;
                    }
                    100% {
                        text-shadow: 0 0 0px #ff4444;
                    }
                }
                
                @keyframes glowSuccess {
                    0% {
                        text-shadow: 0 0 0px #4CAF50;
                    }
                    50% {
                        text-shadow: 0 0 20px #4CAF50, 0 0 30px #4CAF50;
                    }
                    100% {
                        text-shadow: 0 0 0px #4CAF50;
                    }
                }
                
                .back-link {
                    display: block;
                    text-align: center;
                    margin-top: 20px;
                    color: #4CAF50;
                    text-decoration: none;
                    font-size: 14px;
                }
                
                .back-link:hover {
                    text-decoration: underline;
                }
            </style>
        </head>
        <body>
            <div class="login-card">
                <h1>Login</h1>
                <form id="loginForm">
                    <div class="input-group">
                        <label for="username">Username</label>
                        <input type="text" id="username" name="username" required>
                    </div>
                    <div class="input-group">
                        <label for="password">Password</label>
                        <input type="password" id="password" name="password" required>
                    </div>
                    <button type="submit" class="login-button">Login</button>
                    <button type="button" class="create-account-button" onclick="window.location.href='/create-account'">Create Account</button>
                    <div id="message" class="message error">
                        Incorrect username / password
                    </div>
                </form>
                <a href="/" class="back-link">Back to Home</a>
            </div>
            
            <script>
                document.getElementById('loginForm').addEventListener('submit', async function(e) {
                    e.preventDefault();
                    
                    const username = document.getElementById('username').value;
                    const password = document.getElementById('password').value;
                    const messageDiv = document.getElementById('message');
                    
                    try {
                        // Send credentials to backend
                        const response = await fetch('/api/login', {
                            method: 'POST',
                            headers: {
                                'Content-Type': 'application/json',
                            },
                            body: JSON.stringify({ username, password })
                        });
                        
                        const result = await response.json();
                        
                        if (!result.success) {
                            // Show error message
                            messageDiv.textContent = 'Incorrect username / password';
                            messageDiv.className = 'message error';
                            void messageDiv.offsetWidth; // Force reflow
                            messageDiv.classList.add('show');
                            
                            setTimeout(() => {
                                messageDiv.classList.remove('show');
                            }, 3000);
                        } else {
                            // Show success message
                            messageDiv.textContent = 'Login successful!';
                            messageDiv.className = 'message success';
                            void messageDiv.offsetWidth; // Force reflow
                            messageDiv.classList.add('show');
                            
                            // Redirect after a short delay
                            setTimeout(() => {
                                window.location.href = '/';
                            }, 1500);
                        }
                    } catch (error) {
                        console.error('Login error:', error);
                        messageDiv.textContent = 'Server error. Please try again.';
                        messageDiv.className = 'message error';
                        messageDiv.classList.add('show');
                    }
                });
            </script>
        </body>
        </html>
    )";
}

#endif