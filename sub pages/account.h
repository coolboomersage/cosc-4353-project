#ifndef account_h
#define account_h
#include "../cpp-httplib-0.15.3/httplib.h"

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
                document.getElementById('loginForm').addEventListener('submit', function(e) {
                    e.preventDefault();
                    
                    const username = document.getElementById('username').value;
                    const password = document.getElementById('password').value;
                    const errorMessage = document.getElementById('errorMessage');
                    
                    // TODO: Replace this with actual database check
                    // For now, using a simple example check
                    const validCredentials = checkCredentials(username, password);
                    
                    if (!validCredentials) {
                        // Remove the 'show' class if it exists to reset animation
                        errorMessage.classList.remove('show');
                        
                        // Trigger reflow to restart animation
                        void errorMessage.offsetWidth;
                        
                        // Add the 'show' class to display error and play animation
                        errorMessage.classList.add('show');
                        
                        // Remove the class after animation completes (non-repeating)
                        setTimeout(() => {
                            errorMessage.classList.remove('show');
                        }, 3000);
                    } else {
                        // Successful login - redirect or perform action
                        alert('Login successful!');
                        window.location.href = '/';
                    }
                });
                
                function checkCredentials(username, password) {
                    // TODO: Implement actual database check here
                    // This is a placeholder function
                    // Return false to test the error animation
                    
                    // Example hardcoded check (replace with database call):
                    if (username === "admin" && password === "password") {
                        return true;
                    }
                    
                    return false;
                }
            </script>
        </body>
        </html>
    )";
}

#endif