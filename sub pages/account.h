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

#ifdef _WIN32
    #include <windows.h>
#else
    #include <unistd.h>
    #include <limits.h>
#endif

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

// Helper function to hash password with Argon2
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

// Helper function to verify password against stored hash
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

bool checkCredentials(const std::string& username, const std::string& password) {
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
        return false;
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
            "email TEXT NOT NULL);";
        
        rc = sqlite3_exec(db, createTableSQL, nullptr, nullptr, &errMsg);
        if (rc != SQLITE_OK) {
            std::cout << "Failed to create table: " << errMsg << std::endl;
            sqlite3_free(errMsg);
            sqlite3_close(db);
            return false;
        }
        
        // Hash the default password
        std::string defaultPasswordHash = hashPassword("root123");
        
        std::string insertDefaultSQL = 
            "INSERT INTO accounts (username, hash, email) VALUES ('root', '" 
            + defaultPasswordHash + "', 'root@root');";
        
        rc = sqlite3_exec(db, insertDefaultSQL.c_str(), nullptr, nullptr, &errMsg);
        if (rc != SQLITE_OK) {
            std::cout << "Failed to insert default account: " << errMsg << std::endl;
            sqlite3_free(errMsg);
            sqlite3_close(db);
            return false;
        }
        
        std::cout << "Default account created (username: root, password: root123)" << std::endl;
    }
    
    sqlite3_stmt* stmt;
    const char* selectSQL = "SELECT hash FROM accounts WHERE username = ?;";
    
    rc = sqlite3_prepare_v2(db, selectSQL, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        std::cout << "Failed to prepare statement: " << sqlite3_errmsg(db) << std::endl;
        sqlite3_close(db);
        return false;
    }
    
    sqlite3_bind_text(stmt, 1, username.c_str(), -1, SQLITE_TRANSIENT);
    bool credentialsValid = false;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        const char* storedHash = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
        
        credentialsValid = verifyPassword(password, storedHash);
        
        if (credentialsValid) {
            std::cout << "Credentials valid for user: " << username << std::endl;
        } else {
            std::cout << "Invalid password for user: " << username << std::endl;
        }
    } else {
        std::cout << "User not found: " << username << std::endl;
    }
    
    sqlite3_finalize(stmt);
    sqlite3_close(db);
    
    return credentialsValid;
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
                
                .error-message {
                    color: #ff4444;
                    text-align: center;
                    margin-top: 20px;
                    font-size: 14px;
                    opacity: 0;
                    transition: opacity 0.3s;
                }
                
                .error-message.show {
                    opacity: 1;
                    animation: glowError 0.5s ease-in-out;
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
                    <div id="errorMessage" class="error-message">
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
                    const errorMessage = document.getElementById('errorMessage');
                    
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
                            errorMessage.classList.remove('show');
                            void errorMessage.offsetWidth;
                            errorMessage.classList.add('show');
                            
                            setTimeout(() => {
                                errorMessage.classList.remove('show');
                            }, 3000);
                        } else {
                            alert('Login successful!');
                            window.location.href = '/';
                        }
                    } catch (error) {
                        console.error('Login error:', error);
                        errorMessage.textContent = 'Server error. Please try again.';
                        errorMessage.classList.add('show');
                    }
                });
            </script>
        </body>
        </html>
    )";
}

#endif