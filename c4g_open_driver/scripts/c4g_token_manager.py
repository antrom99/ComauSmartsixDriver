#!/usr/bin/env python3
"""
C4G Token Manager Utility

This script provides a command-line interface for managing personal access tokens
for the C4G Open Driver system.

Usage:
    c4g_token_manager.py generate [--description "token description"]
    c4g_token_manager.py list
    c4g_token_manager.py revoke <token>
    c4g_token_manager.py enable
    c4g_token_manager.py disable
    c4g_token_manager.py status

Copyright (C) 2025 ANTONIO ROMANO
Licensed under MIT License
"""

import argparse
import os
import sys
import subprocess
import tempfile

def main():
    parser = argparse.ArgumentParser(description='C4G Token Manager Utility')
    subparsers = parser.add_subparsers(dest='command', help='Available commands')
    
    # Generate token command
    generate_parser = subparsers.add_parser('generate', help='Generate a new personal access token')
    generate_parser.add_argument('--description', '-d', default='', help='Description for the token')
    
    # List tokens command
    subparsers.add_parser('list', help='List all valid tokens')
    
    # Revoke token command
    revoke_parser = subparsers.add_parser('revoke', help='Revoke a specific token')
    revoke_parser.add_argument('token', help='Token to revoke')
    
    # Enable/disable commands
    subparsers.add_parser('enable', help='Enable token authentication')
    subparsers.add_parser('disable', help='Disable token authentication')
    subparsers.add_parser('status', help='Show authentication status')
    
    args = parser.parse_args()
    
    if not args.command:
        parser.print_help()
        return 1
    
    try:
        if args.command == 'generate':
            print(f"Generating new personal access token...")
            if args.description:
                print(f"Description: {args.description}")
            
            # For demonstration, generate a simple token
            import random
            token = f"c4g_{random.randint(10000000, 99999999):08x}_{random.randint(1000, 9999)}"
            print(f"Generated new personal access token: {token}")
            print("Please store this token securely. It will not be shown again.")
            
            # Save to token file
            home = os.path.expanduser("~")
            token_file = os.path.join(home, ".c4g_tokens")
            with open(token_file, "a") as f:
                f.write(f"{token} # {args.description or 'Generated token'}\n")
            os.chmod(token_file, 0o600)  # Read/write for owner only
            
        elif args.command == 'list':
            home = os.path.expanduser("~")
            token_file = os.path.join(home, ".c4g_tokens")
            
            if os.path.exists(token_file):
                print("Valid tokens:")
                with open(token_file, "r") as f:
                    for line in f:
                        line = line.strip()
                        if line and not line.startswith('#'):
                            parts = line.split(' #', 1)
                            token = parts[0]
                            description = parts[1] if len(parts) > 1 else "No description"
                            print(f"  {token[:12]}... # {description}")
            else:
                print("No tokens found. Use 'generate' to create one.")
                
        elif args.command == 'revoke':
            print(f"Revoking token: {args.token[:12]}...")
            home = os.path.expanduser("~")
            token_file = os.path.join(home, ".c4g_tokens")
            
            if os.path.exists(token_file):
                # Read all tokens except the one to revoke
                lines = []
                with open(token_file, "r") as f:
                    for line in f:
                        if not line.strip().startswith(args.token):
                            lines.append(line)
                
                # Write back the filtered tokens
                with open(token_file, "w") as f:
                    f.writelines(lines)
                
                print("Token revoked successfully.")
            else:
                print("No token file found.")
            
        elif args.command == 'enable':
            print("Enabling token authentication...")
            print("Add this to your shell profile (.bashrc, .bash_profile, etc.):")
            print("export C4G_TOKEN_AUTH_ENABLED=1")
            print("")
            print("Then reload your shell or run: source ~/.bashrc")
            
        elif args.command == 'disable':
            print("Disabling token authentication...")
            print("Remove or comment out this line from your shell profile:")
            print("export C4G_TOKEN_AUTH_ENABLED=1")
            print("")
            print("Or run: unset C4G_TOKEN_AUTH_ENABLED")
            
        elif args.command == 'status':
            auth_enabled = os.getenv('C4G_TOKEN_AUTH_ENABLED')
            if auth_enabled == '1':
                print("Token authentication: ENABLED")
            else:
                print("Token authentication: DISABLED")
                
            home = os.path.expanduser("~")
            token_file = os.path.join(home, ".c4g_tokens")
            if os.path.exists(token_file):
                with open(token_file, "r") as f:
                    lines = [line.strip() for line in f if line.strip() and not line.strip().startswith('#')]
                print(f"Number of valid tokens: {len(lines)}")
            else:
                print("Number of valid tokens: 0")
                
    except Exception as e:
        print(f"Error: {e}", file=sys.stderr)
        return 1
    
    return 0

if __name__ == '__main__':
    sys.exit(main())