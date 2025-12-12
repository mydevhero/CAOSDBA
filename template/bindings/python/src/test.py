#!/usr/bin/env python3
"""
caos_query.py - Execute CAOS query and print result
"""

import sys
import os

def main():
    # Add module path
    sys.path.insert(0, "../build/release")

    try:
        import my_app
    except ImportError as e:
        print(f"ERROR: Cannot import module: {e}")
        sys.exit(1)

    # Execute query
    token = "ARBJi7cJuOYPXmFPPLVWsGrXmD4SU3LW"
    message = "Test query"

    result = my_app.IQuery_Template_echoString({"token": token}, message)

    # Print only the result
    print(result)

if __name__ == "__main__":
    main()
