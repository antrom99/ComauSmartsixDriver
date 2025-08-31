/*
    TokenManager.hpp

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

/**
    @file TokenManager.hpp
    @brief Token-based authentication system for C4G Open driver.
    
    This file contains the definition of TokenManager class that provides
    personal access token generation, validation, and management for
    secure access to robot operations.
*/

#ifndef _TOKENMANAGER_HPP_
#define _TOKENMANAGER_HPP_

#include <string>
#include <vector>
#include <unordered_set>
#include <chrono>
#include <fstream>

namespace c4g_open_driver
{

/**
 * @class TokenManager
 * @brief Manages personal access tokens for robot authentication
 * 
 * This class provides functionality to generate, validate, and manage
 * personal access tokens for controlling robot operations. Tokens are
 * stored in a configuration file and can have expiration times.
 */
class TokenManager
{
private:
    std::string tokenFilePath;                    //< Path to the token storage file
    std::unordered_set<std::string> validTokens;  //< Set of currently valid tokens
    bool tokenAuthEnabled;                        //< Flag to enable/disable token authentication
    
    /**
     * @brief Load tokens from configuration file
     * @return true if tokens loaded successfully, false otherwise
     */
    bool loadTokensFromFile();
    
    /**
     * @brief Save tokens to configuration file
     * @return true if tokens saved successfully, false otherwise
     */
    bool saveTokensToFile();
    
    /**
     * @brief Generate a random UUID-style token
     * @return Generated token string
     */
    std::string generateRandomToken();

public:
    /**
     * @brief Constructor
     * @param configPath Path to token configuration file (default: ~/.c4g_tokens)
     */
    TokenManager(const std::string& configPath = "");
    
    /**
     * @brief Destructor
     */
    ~TokenManager();
    
    /**
     * @brief Initialize the token manager
     * @return true if initialization successful, false otherwise
     */
    bool initialize();
    
    /**
     * @brief Generate a new personal access token
     * @param description Optional description for the token
     * @return Generated token string, empty if generation failed
     */
    std::string generateToken(const std::string& description = "");
    
    /**
     * @brief Validate a given token
     * @param token Token string to validate
     * @return true if token is valid, false otherwise
     */
    bool validateToken(const std::string& token);
    
    /**
     * @brief Revoke a token (remove from valid tokens)
     * @param token Token string to revoke
     * @return true if token was revoked, false if token not found
     */
    bool revokeToken(const std::string& token);
    
    /**
     * @brief List all valid tokens
     * @return Vector of valid token strings
     */
    std::vector<std::string> listTokens();
    
    /**
     * @brief Enable or disable token authentication
     * @param enabled true to enable, false to disable
     */
    void setTokenAuthEnabled(bool enabled);
    
    /**
     * @brief Check if token authentication is enabled
     * @return true if enabled, false otherwise
     */
    bool isTokenAuthEnabled() const;
    
    /**
     * @brief Clear all tokens
     */
    void clearAllTokens();
};

} // namespace c4g_open_driver

#endif /* _TOKENMANAGER_HPP_ */