#!/usr/bin/env bash
# Check include order in source files according to project guidelines
# Exit with non-zero status if violations are found


SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(cd "${SCRIPT_DIR}/.." && pwd)"

echo "Checking include order in source files..."

cd "$PROJECT_ROOT"

# Function to get the expected matching header for a .cpp file
get_matching_header() {
    local cpp_file="$1"
    local base_name=$(basename "$cpp_file" .cpp)
    echo "${base_name}.h"
}

# Function to categorize an include
# Returns: 1=MatchingHeader, 2=Project, 3=ThirdParty, 4=Qt, 5=Std, 6=Other
categorize_include() {
    local include_line="$1"
    local matching_header="$2"
    
    # Extract include path
    if [[ $include_line =~ \#include[[:space:]]+[\<\"]([^\>\"]*)[\>\"] ]]; then
        local include_path="${BASH_REMATCH[1]}"
        
        # 1. Matching header
        if [[ -n "$matching_header" && ( "$include_path" == "$matching_header" || "$include_path" == */"$matching_header" ) ]]; then
            echo 1
            return
        fi
        
        # 2. Project headers
        if [[ "$include_path" =~ ^(src/|include/|tests/|ui/|app/) ]]; then
            echo 2
            return
        fi
        
        # 3. Third-party non-Qt
        if [[ "$include_path" =~ ^(spdlog/|fmt/|boost/) ]]; then
            echo 3
            return
        fi
        
        # 4. Qt headers
        if [[ "$include_path" =~ ^(Qt|q) || "$include_path" =~ /q ]]; then
            echo 4
            return
        fi
        
        # 5. C++ standard library (no slashes)
        if [[ "$include_path" != *"/"* ]]; then
            echo 5
            return
        fi
        
        # 6. Other
        echo 6
    else
        echo 6
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
    
    while IFS= read -r line; do
        ((line_num++))
        
        # Skip empty lines and comments
        if [[ -z "${line// /}" || "$line" =~ ^[[:space:]]*// ]]; then
            continue
        fi
        
        # Check if this is an include line
        if [[ "$line" =~ ^[[:space:]]*\#include ]]; then
            local category=$(categorize_include "$line" "$matching_header")
            
            # Check if category order is violated
            if [ "$category" -lt "$last_category" ]; then
                echo "  Line $line_num: Include order violation"
                echo "    Category $category comes after category $last_category"
                echo "    $line"
                violations=1
            fi
            
            last_category=$category
        fi
    done < "$file"
    
    return $violations
}

# Find all .cpp files in src directory
FILES=$(find src -type f -name "*.cpp" ! -path "*/build/*" ! -path "*/.git/*")

VIOLATIONS_FOUND=0

for file in $FILES; do
    if ! check_file_includes "$file"; then
        echo "Include order violation in: $file"
        VIOLATIONS_FOUND=1
    fi
done

if [ $VIOLATIONS_FOUND -eq 1 ]; then
    echo ""
    echo "Include order violations found!"
    echo "Please review the include order guidelines in docs/STATIC_ANALYSIS.md"
    exit 1
else
    echo "All files follow include order guidelines."
    exit 0
fi
