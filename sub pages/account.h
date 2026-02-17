#ifndef account_h
#define account_h
#include "../external/cpp-httplib-0.15.3/httplib.h"
#include <string>
#include <filesystem>
#include "../external/sqlite-amalgamation-3510200/sqlite3.h"

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

bool checkCredentials(const std::string& username, const std::string& password) {
    sqlite3* db;
    int rc;

    // Get the directory where the executable is located
    std::string exeDir = getExecutableDirectory();
    std::string dbPath = exeDir + "/accounts.db";
    
    std::cout << "Attempting to open database at: " << dbPath << std::endl;

    // Open database (creates if doesn't exist)
    rc = sqlite3_open(dbPath.c_str(), &db);
    
    if (rc) {
        std::cout << "Can't open database: " << sqlite3_errmsg(db) << std::endl;
        sqlite3_close(db);
        return false;
    } else {
        std::cout << "Opened database successfully at: " << dbPath << std::endl;
    }
    
    // TODO: Implement actual database check here
    // For now, just checking if we can open the database
    
    sqlite3_close(db);
    return false;  // Change this when you implement actual credential checking
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