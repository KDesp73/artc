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
#   ./check_deps.sh -q                 # Quiet mode (no output, only exit code)
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

# Build toolchain + runtime (bear is only for compile_commands; optional in CI)
OPTIONAL_EXES=(
	"bear"
)
DEPENDENCIES=(
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

# C compiler: accept gcc (often clang on macOS) or cc
check_c_compiler() {
    if command -v gcc &>/dev/null; then
        print_msg "${GREEN}✔ C compiler (gcc) found${RESET}"
        return 0
    elif command -v cc &>/dev/null; then
        print_msg "${GREEN}✔ C compiler (cc) found${RESET}"
        return 0
    fi
    print_error "✖ No C compiler (gcc or cc) found"
    exit 1
}

# True if a shared lib for this -l name exists in directory p (e.g. name SDL2 -> libSDL2*.dylib)
__path_has_lib() {
    local p="$1" name="$2"
    for ext in dylib so; do
        for f in "$p/lib${name}.$ext" "$p"/lib${name}*.$ext; do
            if [[ -f $f ]]; then
                return 0
            fi
        done
    done
    return 1
}

# macOS: search typical Homebrew and env paths
check_shared_library_darwin() {
    local name="$1"
    # Project static build does not preinstall a shared libartc; Lua may come from deps/lib
    if [[ $name = "artc" ]]; then
        print_msg "${GREEN}✔ (artc) built in-tree; not checked as a system library${RESET}"
        return 0
    fi
    BREW_PFX="${BREW_PFX:-}"
    if [[ -z $BREW_PFX ]] && command -v brew &>/dev/null; then
        BREW_PFX=$(brew --prefix 2>/dev/null)
    fi
    BREW_PFX=${BREW_PFX:-/opt/homebrew}
    IFS=':' read -ra DIRS <<< "${DYLD_LIBRARY_PATH:-}"
    for extra in \
        "/opt/homebrew/lib" \
        "/usr/local/lib" \
        "${BREW_PFX}/lib" \
        "${BREW_PFX}/opt/sdl2/lib" \
        "${BREW_PFX}/opt/sdl2_ttf/lib" \
        "${BREW_PFX}/opt/sdl2_image/lib" \
        "${BREW_PFX}/opt/lua/lib" \
        "${BREW_PFX}/opt/libmagic/lib" \
        "${BREW_PFX}/opt/sdl2_image/lib" \
    ; do
        DIRS+=("$extra")
    done
    if [[ -n ${LD_LIBRARY_PATH:-} ]]; then
        IFS=':' read -ra x <<< "$LD_LIBRARY_PATH"
        DIRS+=("${x[@]}")
    fi
    for p in "${DIRS[@]}"; do
        [[ -n $p && -d $p ]] || continue
        if __path_has_lib "$p" "$name"; then
            print_msg "${GREEN}✔ Library 'lib${name}*' found in $p (macOS)${RESET}"
            return 0
        fi
    done
    # libm is in libSystem; libgcc_s may be implicit
    if [[ $name = "m" ]]; then
        print_msg "${GREEN}✔ Math (libm) is provided by the system (macOS)${RESET}"
        return 0
    fi
    print_error "✖ Shared library matching 'lib${name}*' NOT found (install e.g. Homebrew deps)"
    exit 1
}

# Linux: search in LD path + ldconfig
check_shared_library() {
    local lib="$1"
    IFS=":" read -ra paths <<< "$LD_LIBRARY_PATH"
    for path in "${paths[@]}"; do
        if [[ -f "$path/$lib" ]]; then
            print_msg "${GREEN}✔ Shared library '$lib' found in $path${RESET}"
            return
        fi
    done

    if command -v ldconfig &>/dev/null; then
        if ldconfig -p 2>/dev/null | grep -F -q "$lib"; then
            print_msg "${GREEN}✔ Shared library '$lib' found (system-wide)${RESET}"
            return
        fi
    fi
    print_error "✖ Shared library '$lib' NOT found"
    exit 1
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

# Portably extract -l<name> tokens from a Makefile (skip -l:GNU ext)
__flags_from_makefile() {
    tr -s ' \n\t' ' ' < "$MAKEFILE" | tr ' ' '\n' | grep -E -- '^-l[[:alnum:]_]+$'
}

# Known suffix for linked libs
detect_makefile_deps() {
    if [[ ! -f $MAKEFILE ]]; then
        print_error "✖ Makefile '$MAKEFILE' not found"
        exit 1
    fi
    print_msg "Detecting -l flags from $MAKEFILE..."
    if [[ $(uname -s) = "Darwin" ]]; then
        while IFS= read -r flag; do
            libname="${flag#-l}"
            if [[ -z $libname || $libname = m ]]; then
                continue
            fi
            case "$libname" in
                c|pthread) continue ;;
            esac
            check_shared_library_darwin "$libname"
        done < <( __flags_from_makefile )
    else
        while IFS= read -r flag; do
            libname="${flag#-l}"
            if [[ -z $libname || $libname = m ]]; then
                continue
            fi
            case "$libname" in
                c|pthread) continue ;;
            esac
            check_shared_library "lib$libname.so"
        done < <( __flags_from_makefile )
    fi
}

# Main check function
check_dependencies() {
    print_msg "Detecting manually defined dependencies..."
    for dep in "${OPTIONAL_EXES[@]}"; do
        if command -v "$dep" &>/dev/null; then
            print_msg "${GREEN}✔ Optional '$dep' found${RESET}"
        else
            if ! $QUIET && ! $SILENT; then
                print_msg "Optional tool '$dep' not found (ok unless you use that target)"
            fi
        fi
    done
    check_c_compiler
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
