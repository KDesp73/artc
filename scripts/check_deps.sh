#!/usr/bin/env bash

# ============================================
#  Dependency Checker Script
# ============================================
# This script checks if required dependencies 
# (executables, shared libraries, static libraries) 
# are available in the system.
#
# Usage:
#   ./check_deps.sh [-m <Makefile>] [-q] [-s] [-h] [-v]
#
# Options:
#   -m <Makefile>  Specify a Makefile for auto-detection (default: "Makefile").
#   -q             Quiet mode (no output, only exit code).
#   -s             Silent mode (only prints errors).
#   -h             Show this help message and exit.
#   -v             Show version information and exit.
#
# Exit Codes:
#   0  - All dependencies found
#   1  - At least one dependency is missing
#
# Example:
#   ./check_deps.sh                   # Check manually defined dependencies + Makefile
#   ./check_deps.sh -m MyMakefile      # Use a custom Makefile for detection
#   ./check_deps.sh -q                 # Quiet mode (no output)
#   ./check_deps.sh -s                 # Silent mode (only errors)
#   ./check_deps.sh -h                 # Show help message
#   ./check_deps.sh -v                 # Show version info
#
# Written by KDesp73 (Konstantinos Despoinidis) @ 26/03/2025
# ============================================

# Default values
MAKEFILE="Makefile"
QUIET=false  # Quiet mode (no output)
SILENT=false # Silent mode (only errors)
VERSION="1.0.0"

# Define dependencies manually 
DEPENDENCIES=(
    "gcc"
    "bear"
    "ffmpeg"
)

# ANSI color codes
GREEN="\e[32m"
RED="\e[31m"
RESET="\e[0m"

# Function to display help message
show_help() {
    echo "Usage: $0 [-m <Makefile>] [-q] [-s] [-h] [-v]"
    echo
    echo "Options:"
    echo "  -m <Makefile>  Specify a Makefile for dependency detection (default: \"Makefile\")."
    echo "  -q             Quiet mode (no output, only exit code)."
    echo "  -s             Silent mode (only prints errors)."
    echo "  -h             Show this help message and exit."
    echo "  -v             Show version information and exit."
    exit 0
}

# Parse command-line options
while getopts "m:qshv" opt; do
    case $opt in
        m) MAKEFILE="$OPTARG" ;;
        q) QUIET=true ;;   # Enable quiet mode (no output at all)
        s) SILENT=true ;;  # Enable silent mode (only errors)
        h) show_help ;;    # Show help message and exit
        v) echo "Dependency Checker v$VERSION" && exit 0 ;; # Show version info
        *) show_help ;;    # Default to help if invalid option
    esac
done

# Function to print messages (suppressed in quiet mode)
print_msg() {
    if ! $QUIET && ! $SILENT; then
        echo -e "$1"
    fi
}

# Function to print errors (only suppressed in quiet mode)
print_error() {
    if ! $QUIET; then
        echo -e "${RED}$1${RESET}" >&2
    fi
}

# Function to check if an executable exists
check_executable() {
    if command -v "$1" &>/dev/null; then
        print_msg "${GREEN}✔ Executable '$1' found${RESET}"
    else
        print_error "✖ Executable '$1' NOT found"
        exit 1
    fi
}

# Function to search for a shared library in $LD_LIBRARY_PATH
check_shared_library() {
    local lib="$1"
    IFS=":" read -ra paths <<< "$LD_LIBRARY_PATH"

    for path in "${paths[@]}"; do
        if [[ -f "$path/$lib" ]]; then
            print_msg "${GREEN}✔ Shared library '$lib' found in $path${RESET}"
            return
        fi
    done

    # Fallback: Check system-wide with ldconfig
    if ldconfig -p | grep -q "$lib"; then
        print_msg "${GREEN}✔ Shared library '$lib' found (system-wide)${RESET}"
    else
        print_error "✖ Shared library '$lib' NOT found"
        exit 1
    fi
}

# Function to check for a static library
check_static_library() {
    local lib="$1"
    IFS=":" read -ra paths <<< "$LD_LIBRARY_PATH"

    for path in "${paths[@]}"; do
        if [[ -f "$path/$lib" ]]; then
            print_msg "${GREEN}✔ Static library '$lib' found in $path${RESET}"
            return
        fi
    done

    print_error "✖ Static library '$lib' NOT found"
    exit 1
}

# Function to detect dependencies from a given Makefile
detect_makefile_deps() {
    if [[ -f "$MAKEFILE" ]]; then
        print_msg "Detecting dependencies from $MAKEFILE..."
        grep -oP "(?<=-l)[a-zA-Z0-9_]+" "$MAKEFILE" | while read -r lib; do
            check_shared_library "lib$lib.so"
        done
    else
        print_error "✖ Makefile '$MAKEFILE' not found"
        exit 1
    fi
}

# Main check function
check_dependencies() {
    print_msg "Detecting manually defined dependencies..."
    for dep in "${DEPENDENCIES[@]}"; do
        if [[ $dep == *.so ]]; then
            check_shared_library "$dep"
        elif [[ $dep == *.a ]]; then
            check_static_library "$dep"
        else
            check_executable "$dep"
        fi
    done
}

# Run checks
check_dependencies
detect_makefile_deps
