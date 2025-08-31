# Personal Access Token Authentication

This document describes the personal access token authentication system for the C4G Open Driver.

## Overview

The C4G Open Driver now supports optional token-based authentication to secure robot operations. When enabled, the system requires a valid personal access token to perform critical robot functions such as starting communication or changing operation modes.

## Features

- **Token Generation**: Create unique personal access tokens with optional descriptions
- **Token Validation**: Authenticate robot operations using tokens
- **Token Management**: List, revoke, and manage tokens
- **Optional Authentication**: Can be enabled/disabled via environment variables
- **Secure Storage**: Tokens are stored in user-specific configuration files with restricted permissions

## Token Format

Tokens use the format: `c4g_XXXXXXXX_XXXX` where X represents hexadecimal characters.

Example: `c4g_a1b2c3d4_5678`

## Configuration

### Enable Token Authentication

Set the environment variable to enable token authentication:

```bash
export C4G_TOKEN_AUTH_ENABLED=1
```

Add this to your shell profile (`.bashrc`, `.bash_profile`, etc.) to make it permanent.

### Disable Token Authentication

Unset the environment variable or set it to 0:

```bash
unset C4G_TOKEN_AUTH_ENABLED
# or
export C4G_TOKEN_AUTH_ENABLED=0
```

## Token Management

### Using the Token Manager Script

The `c4g_token_manager.py` script provides command-line token management:

```bash
# Generate a new token
./scripts/c4g_token_manager.py generate --description "Robot control token"

# List all tokens
./scripts/c4g_token_manager.py list

# Check authentication status
./scripts/c4g_token_manager.py status

# Revoke a token
./scripts/c4g_token_manager.py revoke c4g_a1b2c3d4_5678

# Enable/disable authentication
./scripts/c4g_token_manager.py enable
./scripts/c4g_token_manager.py disable
```

### Using the C++ API

```cpp
#include <c4g_open_driver/C4gOpen.hpp>

// Create C4gOpen instance
C4gOpen robot(1001);

// Check if token authentication is enabled
if (robot.isTokenAuthEnabled()) {
    // Generate a new token
    std::string token = robot.generateToken("My robot control app");
    
    // Set the token for authentication
    robot.setAuthToken(token);
}

// Now perform robot operations
if (!robot.start()) {
    if (robot.getLastError() == TOKEN_AUTHENTICATION_FAILED) {
        std::cerr << "Authentication failed: Invalid or missing token" << std::endl;
    }
}
```

## Token Storage

Tokens are stored in the file `~/.c4g_tokens` with the format:

```
# C4G Open Driver Personal Access Tokens
# Generated on: 1672531200
# Format: token [description]

c4g_a1b2c3d4_5678 # Robot control token
c4g_e9f0a1b2_3456 # Test application token
```

The file has restricted permissions (600) to prevent unauthorized access.

## Security Considerations

1. **Token Protection**: Store tokens securely and never commit them to version control
2. **Token Rotation**: Regularly generate new tokens and revoke old ones
3. **Environment Isolation**: Use different tokens for different environments (development, production)
4. **Principle of Least Privilege**: Create tokens with specific purposes and descriptions

## API Reference

### C4gOpen Class Methods

- `bool setAuthToken(const std::string& token)` - Set authentication token
- `bool isTokenAuthEnabled() const` - Check if token authentication is enabled
- `std::string generateToken(const std::string& description = "")` - Generate new token
- `bool revokeToken(const std::string& token)` - Revoke a token
- `std::vector<std::string> listTokens()` - List all valid tokens

### TokenManager Class

The `TokenManager` class provides the underlying token management functionality:

- `bool initialize()` - Initialize the token manager
- `std::string generateToken(const std::string& description)` - Generate new token
- `bool validateToken(const std::string& token)` - Validate a token
- `bool revokeToken(const std::string& token)` - Revoke a token
- `std::vector<std::string> listTokens()` - List all tokens
- `void setTokenAuthEnabled(bool enabled)` - Enable/disable authentication
- `bool isTokenAuthEnabled() const` - Check authentication status

### Error Codes

- `TOKEN_AUTHENTICATION_FAILED` - Returned when token authentication fails

## Testing

Run the token authentication test:

```bash
cd build
./TestTokenAuth
```

This test validates token generation, validation, and integration with the C4gOpen class.

## Backward Compatibility

The token authentication system is fully backward compatible. When token authentication is disabled (default), all operations work as before without requiring tokens.

## Migration Guide

For existing applications:

1. **No Changes Required**: Applications continue to work without modification when token authentication is disabled
2. **Gradual Migration**: Enable token authentication and update applications to use tokens as needed
3. **Environment-Based Control**: Use environment variables to control authentication per deployment

## Troubleshooting

### Authentication Errors

If you receive `TOKEN_AUTHENTICATION_FAILED` errors:

1. Check if token authentication is enabled: `./scripts/c4g_token_manager.py status`
2. Verify your token is valid: `./scripts/c4g_token_manager.py list`
3. Ensure you've set the token in your application: `robot.setAuthToken(token)`

### Token File Issues

If tokens aren't persisting:

1. Check file permissions on `~/.c4g_tokens`
2. Verify the home directory is writable
3. Ensure the token manager has proper initialization

### Environment Variable Issues

If authentication isn't enabling/disabling:

1. Check the environment variable: `echo $C4G_TOKEN_AUTH_ENABLED`
2. Reload your shell or restart the application
3. Verify the variable is set in the correct shell profile