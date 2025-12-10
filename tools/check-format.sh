#!/usr/bin/env bash
# Check if all source files adhere to clang-format rules
# Exit with non-zero status if formatting violations are found

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(cd "${SCRIPT_DIR}/.." && pwd)"

# Find clang-format executable
CLANG_FORMAT=""
for cmd in clang-format-15 clang-format-16 clang-format-17 clang-format; do
    if command -v "$cmd" &> /dev/null; then
        CLANG_FORMAT="$cmd"
        break
    fi
done

if [ -z "$CLANG_FORMAT" ]; then
    echo "Error: clang-format not found. Please install clang-format."
    exit 1
fi

echo "Using: $CLANG_FORMAT"
echo "Checking code formatting..."

cd "$PROJECT_ROOT"

# Find all .cpp and .h files in src and tests directories
VIOLATIONS_FOUND=0

find src tests -type f \( -name "*.cpp" -o -name "*.h" \) ! -path "*/build/*" ! -path "*/.git/*" -print0 | while IFS= read -r -d '' file; do
    # Check if file would be modified by clang-format
    if ! "$CLANG_FORMAT" --dry-run -Werror "$file" &> /dev/null; then
        echo "Formatting violation: $file"
        VIOLATIONS_FOUND=1
    fi
done

if [ $VIOLATIONS_FOUND -eq 1 ]; then
    echo ""
    echo "Code formatting violations found!"
    echo "Run 'cmake --build build/debug --target run-clang-format' to fix formatting."
    exit 1
else
    echo "All files are properly formatted."
    exit 0
fi
