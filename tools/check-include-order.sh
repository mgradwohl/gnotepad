#!/usr/bin/env bash
# Check include order in source files according to project guidelines
# See CONTRIBUTING.md#include-guidelines for the authoritative order:
#   1. Matching header (.cpp files only)
#   2. Platform-specific headers in #ifdef guards
#   3. Project headers (src/, ui/, app/, tests/)
#   4. Third-party libraries (spdlog, fmt, boost)
#   5. Qt headers (QtCore/, QSignalBlocker, etc.)
#   6. C++ standard library
#   7. Other
#
# Platform-specific includes within #ifdef guards are allowed at any point
# after the matching header and do not affect category tracking.
#
# Exit with non-zero status if violations are found

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(cd "${SCRIPT_DIR}/.." && pwd)"

VERBOSE=false

# Parse arguments
while [[ $# -gt 0 ]]; do
    case "$1" in
        -v|--verbose) VERBOSE=true; shift ;;
        -h|--help)
            echo "Usage: $0 [-v|--verbose]"
            echo "  -v, --verbose   Show which files are being checked"
            exit 0
            ;;
        *) shift ;;
    esac
done

echo "Checking include order in source files..."

cd "$PROJECT_ROOT"

# Function to get the expected matching header for a .cpp file
get_matching_header() {
    local cpp_file="$1"
    local base_name=$(basename "$cpp_file" .cpp)
    echo "${base_name}.h"
}

# Check if an include path is platform-specific
is_platform_specific_include() {
    local include_path="$1"
    # Windows headers
    if [[ "$include_path" =~ ^(windows\.h|windef\.h|winbase\.h|winuser\.h|wingdi\.h|wincon\.h|shellapi\.h|shlobj\.h|shlwapi\.h|commctrl\.h|commdlg\.h|objbase\.h|ole2\.h|oleauto\.h|oaidl\.h|unknwn\.h|winsock2\.h|ws2tcpip\.h|wincrypt\.h|wininet\.h|winhttp\.h|dbghelp\.h|psapi\.h|tlhelp32\.h)$ ]]; then
        return 0
    fi
    # Unix/POSIX headers
    if [[ "$include_path" =~ ^(unistd\.h|sys/|pthread\.h|dlfcn\.h|fcntl\.h|termios\.h|signal\.h|pwd\.h|grp\.h|dirent\.h|poll\.h|mman\.h)$ ]]; then
        return 0
    fi
    # Platform-specific spdlog sinks
    if [[ "$include_path" =~ ^spdlog/sinks/(msvc_sink|wincolor_sink|syslog_sink|systemd_sink|android_sink) ]]; then
        return 0
    fi
    return 1
}

# Function to categorize an include
# Returns: 1=MatchingHeader, 2=PlatformSpecific, 3=Project, 4=ThirdParty, 5=Qt, 6=Std, 7=Other
categorize_include() {
    local include_line="$1"
    local matching_header="$2"
    
    # Check if it's a quoted include (local header)
    local is_quoted=false
    if [[ $include_line =~ \#include[[:space:]]+\" ]]; then
        is_quoted=true
    fi
    
    # Extract include path
    if [[ $include_line =~ \#include[[:space:]]+[\<\"]([^\>\"]*)[\>\"] ]]; then
        local include_path="${BASH_REMATCH[1]}"
        
        # 1. Matching header
        if [[ -n "$matching_header" && ( "$include_path" == "$matching_header" || "$include_path" == */"$matching_header" ) ]]; then
            echo 1
            return
        fi
        
        # 2. Platform-specific headers (detected separately, returned for reference)
        if is_platform_specific_include "$include_path"; then
            echo 2
            return
        fi
        
        # 3. Project headers - explicit paths OR quoted includes (local headers)
        if [[ "$include_path" =~ ^(src/|include/|tests/|ui/|app/) ]] || $is_quoted; then
            echo 3
            return
        fi
        
        # 4. Third-party non-Qt
        if [[ "$include_path" =~ ^(spdlog/|fmt/|boost/) ]]; then
            echo 4
            return
        fi
        
        # 5. Qt headers (QtCore/qfile.h, QSignalBlocker, etc.)
        # ^Q[A-Z] catches Qt classes, ^Qt catches module paths, /q catches paths with Qt headers
        if [[ "$include_path" =~ ^(Qt|Q[A-Z]) || "$include_path" =~ /q ]]; then
            echo 5
            return
        fi
        
        # 6. C++ standard library (no slashes, common headers)
        if [[ "$include_path" != *"/"* ]]; then
            echo 6
            return
        fi
        
        # 7. Other
        echo 7
    else
        echo 7
    fi
}

# Check a single file's include order
check_file_includes() {
    local file="$1"
    local matching_header=""
    
    # Determine matching header for .cpp files
    if [[ "$file" == *.cpp ]]; then
        matching_header=$(get_matching_header "$file")
    fi
    
    local last_category=0
    local line_num=0
    local violations=0
    local in_ifdef_block=0
    local ifdef_depth=0
    local seen_matching_header=0
    
    while IFS= read -r line; do
        ((line_num++))
        
        # Skip empty lines and comments
        if [[ -z "${line// /}" || "$line" =~ ^[[:space:]]*// ]]; then
            continue
        fi
        
        # Track #ifdef/#endif blocks for platform-specific detection
        if [[ "$line" =~ ^[[:space:]]*\#if ]]; then
            ((ifdef_depth++))
            # Check if this is a platform-specific ifdef
            if [[ "$line" =~ ^[[:space:]]*\#if(def)?[[:space:]]+((_WIN32|WIN32|_MSC_VER|__linux__|__APPLE__|__unix__|__GNUC__|__clang__|NDEBUG|_DEBUG)) ]]; then
                in_ifdef_block=$ifdef_depth
            fi
        elif [[ "$line" =~ ^[[:space:]]*\#endif ]]; then
            if [[ $ifdef_depth -eq $in_ifdef_block ]]; then
                in_ifdef_block=0
            fi
            ((ifdef_depth--))
            if [[ $ifdef_depth -lt 0 ]]; then ifdef_depth=0; fi
        elif [[ "$line" =~ ^[[:space:]]*\#else || "$line" =~ ^[[:space:]]*\#elif ]]; then
            # Stay in ifdef block context
            :
        fi
        
        # Check if this is an include line
        if [[ "$line" =~ ^[[:space:]]*\#include ]]; then
            local category=$(categorize_include "$line" "$matching_header")
            
            # Track if we've seen the matching header
            if [[ $category -eq 1 ]]; then
                seen_matching_header=1
            fi
            
            # Platform-specific includes (category 2) in #ifdef blocks are always OK after matching header
            if [[ $category -eq 2 ]]; then
                if [[ $in_ifdef_block -gt 0 || $ifdef_depth -gt 0 ]]; then
                    # Platform-specific include in #ifdef block - OK, skip order check
                    continue
                else
                    # Platform-specific include NOT in #ifdef block - violation!
                    echo "  Line $line_num: Platform-specific include outside #ifdef guard"
                    echo "    $line"
                    violations=1
                    continue
                fi
            fi
            
            # For non-platform-specific includes, check category order
            # Skip order check if we're in an #ifdef block (allows flexibility for conditional includes)
            if [[ $in_ifdef_block -eq 0 && $ifdef_depth -eq 0 ]]; then
                if [[ $category -lt $last_category ]]; then
                    echo "  Line $line_num: Include order violation"
                    echo "    Category $category comes after category $last_category"
                    echo "    $line"
                    violations=1
                fi
                last_category=$category
            fi
        fi
    done < "$file"
    
    return $violations
}

# Find all .cpp and .h files in src and tests directories and check include order robustly
VIOLATIONS_FOUND=0

# Count files for progress
FILE_COUNT=$(find src tests -type f \( -name "*.cpp" -o -name "*.h" \) ! -path "*/build/*" ! -path "*/.git/*" | wc -l)
CHECKED_COUNT=0

while IFS= read -r -d '' file; do
    CHECKED_COUNT=$((CHECKED_COUNT + 1))
    if $VERBOSE; then
        echo "[$CHECKED_COUNT/$FILE_COUNT] Checking: $(basename "$file")"
    fi
    if ! check_file_includes "$file"; then
        echo "Include order violation in: $file"
        echo ""
        VIOLATIONS_FOUND=1
    fi
done < <(find src tests -type f \( -name "*.cpp" -o -name "*.h" \) ! -path "*/build/*" ! -path "*/.git/*" -print0)

if [ $VIOLATIONS_FOUND -eq 1 ]; then
    echo "Include order violations found!"
    echo "Please review the include order guidelines in CONTRIBUTING.md#include-guidelines"
    exit 1
else
    echo "All $FILE_COUNT files follow include order guidelines."
    exit 0
fi
