/*
    TestTokenAuth.cpp

    Copyright (C) 2025 ANTONIO ROMANO

    This file is part of libC4gOpen.

    libC4gOpen is free software; you can redistribute it and/or modify
    it under the terms of the GNU Lesser General Public License as
    published by the Free Software Foundation; either version 2.1 of
    the License, or (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with this program. If not, see <http://www.gnu.org/licenses/>.
*/

#include <iostream>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <c4g_open_driver/C4gOpen.hpp>
#include <c4g_open_driver/TokenManager.hpp>

using namespace std;
using namespace c4g_open_driver;

const char disclaimer[] = "\n*********************************************************************\n"
                          "                            TestTokenAuth                              \n\n"
                          "   C4G Open Library - Token Authentication Test                        \n\n"
                          "  Copyright (C) 2025 ANTONIO ROMANO                                   \n\n"
                          "\n*********************************************************************\n";

int32_t main(int32_t argc, char *argv[])
{
    cout << disclaimer << "\n";

    // Test TokenManager directly
    cout << "Testing TokenManager...\n";
    
    TokenManager tokenManager;
    if (!tokenManager.initialize()) {
        cout << "Failed to initialize TokenManager\n";
        return 1;
    }
    
    cout << "Token authentication enabled: " << (tokenManager.isTokenAuthEnabled() ? "YES" : "NO") << "\n";
    
    // Generate a test token
    string testToken = tokenManager.generateToken("Test token for demo");
    if (testToken.empty()) {
        cout << "Failed to generate token\n";
        return 1;
    }
    
    // Validate the token
    if (tokenManager.validateToken(testToken)) {
        cout << "Token validation: PASSED\n";
    } else {
        cout << "Token validation: FAILED\n";
    }
    
    // Test invalid token
    if (!tokenManager.validateToken("invalid_token")) {
        cout << "Invalid token rejection: PASSED\n";
    } else {
        cout << "Invalid token rejection: FAILED\n";
    }
    
    // List tokens
    auto tokens = tokenManager.listTokens();
    cout << "Number of valid tokens: " << tokens.size() << "\n";
    
    // Test C4gOpen with token authentication
    cout << "\nTesting C4gOpen with token authentication...\n";
    
    // Get port from environment variable (if available)
    int32_t port = 1001; // Default port
    const char* env_var_port_number = std::getenv("C4G_PORT_NUMBER");
    if (env_var_port_number != NULL) {
        port = atoi(env_var_port_number);
    }
    
    C4gOpen c4gOpen(port);
    
    cout << "Token authentication enabled: " << (c4gOpen.isTokenAuthEnabled() ? "YES" : "NO") << "\n";
    
    // Test without setting token (should fail if auth is enabled)
    if (c4gOpen.isTokenAuthEnabled()) {
        cout << "Testing operation without token...\n";
        if (!c4gOpen.setMode(1, C4G_OPEN_MODE_0)) {
            cout << "Operation correctly blocked without token\n";
        } else {
            cout << "WARNING: Operation allowed without token\n";
        }
    }
    
    // Set valid token and test again
    cout << "Setting valid token and testing operation...\n";
    c4gOpen.setAuthToken(testToken);
    
    // This should now succeed (though may fail for other reasons like no robot connected)
    if (c4gOpen.setMode(1, C4G_OPEN_MODE_0)) {
        cout << "Operation with valid token: ALLOWED\n";
    } else {
        if (c4gOpen.getLastError() == TOKEN_AUTHENTICATION_FAILED) {
            cout << "Operation failed due to token authentication\n";
        } else {
            cout << "Operation failed for other reasons (normal if no robot connected)\n";
        }
    }
    
    // Test token management through C4gOpen
    cout << "\nTesting token management through C4gOpen...\n";
    string newToken = c4gOpen.generateToken("Another test token");
    cout << "Generated token through C4gOpen: " << (!newToken.empty() ? "SUCCESS" : "FAILED") << "\n";
    
    auto allTokens = c4gOpen.listTokens();
    cout << "Total tokens via C4gOpen: " << allTokens.size() << "\n";
    
    cout << "\nTokenAuth test completed.\n\n";
    
    return 0;
}