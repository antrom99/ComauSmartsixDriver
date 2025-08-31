/*
    TokenManager.cpp

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

#include <c4g_open_driver/TokenManager.hpp>
#include <iostream>
#include <sstream>
#include <random>
#include <algorithm>
#include <sys/stat.h>
#include <unistd.h>
#include <pwd.h>

namespace c4g_open_driver
{

TokenManager::TokenManager(const std::string& configPath)
    : tokenAuthEnabled(false)
{
    if (configPath.empty()) {
        // Use default path: ~/.c4g_tokens
        const char* homeDir = getenv("HOME");
        if (homeDir == nullptr) {
            struct passwd* pw = getpwuid(getuid());
            homeDir = pw->pw_dir;
        }
        tokenFilePath = std::string(homeDir) + "/.c4g_tokens";
    } else {
        tokenFilePath = configPath;
    }
}

TokenManager::~TokenManager()
{
    // Save tokens before destruction
    saveTokensToFile();
}

bool TokenManager::initialize()
{
    // Check if token authentication should be enabled via environment variable
    const char* authEnabled = getenv("C4G_TOKEN_AUTH_ENABLED");
    if (authEnabled != nullptr && std::string(authEnabled) == "1") {
        tokenAuthEnabled = true;
    }
    
    // Load existing tokens
    return loadTokensFromFile();
}

std::string TokenManager::generateRandomToken()
{
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, 15);
    
    std::string token = "c4g_";
    const char* chars = "0123456789abcdef";
    
    // Generate a 32-character hex string (like UUID without dashes)
    for (int i = 0; i < 32; ++i) {
        token += chars[dis(gen)];
        if (i == 7 || i == 15 || i == 23) {
            token += "_";
        }
    }
    
    return token;
}

std::string TokenManager::generateToken(const std::string& description)
{
    std::string newToken = generateRandomToken();
    
    // Add to valid tokens
    validTokens.insert(newToken);
    
    // Save to file
    if (!saveTokensToFile()) {
        std::cerr << "Warning: Failed to save token to file" << std::endl;
    }
    
    std::cout << "Generated new personal access token: " << newToken << std::endl;
    if (!description.empty()) {
        std::cout << "Description: " << description << std::endl;
    }
    std::cout << "Please store this token securely. It will not be shown again." << std::endl;
    
    return newToken;
}

bool TokenManager::validateToken(const std::string& token)
{
    if (!tokenAuthEnabled) {
        // If token auth is disabled, allow all operations
        return true;
    }
    
    return validTokens.find(token) != validTokens.end();
}

bool TokenManager::revokeToken(const std::string& token)
{
    auto it = validTokens.find(token);
    if (it != validTokens.end()) {
        validTokens.erase(it);
        saveTokensToFile();
        return true;
    }
    return false;
}

std::vector<std::string> TokenManager::listTokens()
{
    std::vector<std::string> tokens;
    for (const auto& token : validTokens) {
        tokens.push_back(token);
    }
    std::sort(tokens.begin(), tokens.end());
    return tokens;
}

void TokenManager::setTokenAuthEnabled(bool enabled)
{
    tokenAuthEnabled = enabled;
}

bool TokenManager::isTokenAuthEnabled() const
{
    return tokenAuthEnabled;
}

void TokenManager::clearAllTokens()
{
    validTokens.clear();
    saveTokensToFile();
}

bool TokenManager::loadTokensFromFile()
{
    std::ifstream file(tokenFilePath);
    if (!file.is_open()) {
        // File doesn't exist yet, create empty token set
        return true;
    }
    
    std::string line;
    while (std::getline(file, line)) {
        // Skip empty lines and comments
        if (line.empty() || line[0] == '#') {
            continue;
        }
        
        // Extract token (first word before any whitespace)
        size_t spacePos = line.find(' ');
        std::string token = (spacePos != std::string::npos) ? 
                           line.substr(0, spacePos) : line;
        
        if (!token.empty()) {
            validTokens.insert(token);
        }
    }
    
    file.close();
    return true;
}

bool TokenManager::saveTokensToFile()
{
    std::ofstream file(tokenFilePath);
    if (!file.is_open()) {
        return false;
    }
    
    file << "# C4G Open Driver Personal Access Tokens" << std::endl;
    file << "# Generated on: " << std::chrono::system_clock::to_time_t(std::chrono::system_clock::now()) << std::endl;
    file << "# Format: token [description]" << std::endl;
    file << std::endl;
    
    for (const auto& token : validTokens) {
        file << token << " # Generated token" << std::endl;
    }
    
    file.close();
    
    // Set file permissions to be readable only by owner
    chmod(tokenFilePath.c_str(), S_IRUSR | S_IWUSR);
    
    return true;
}

} // namespace c4g_open_driver