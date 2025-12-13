#!/usr/bin/env bash
# Check if all source files adhere to clang-format rules
# Uses .clang-format configuration from project root
# Exit with non-zero status if formatting violations are found
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(cd "${SCRIPT_DIR}/.." && pwd)"

VERBOSE=false

usage() {
    cat <<EOF
Usage: $(basename "$0") [OPTIONS]

Check if all source files adhere to clang-format rules.
Use clang-format.sh to fix violations.

Options:
  -v, --verbose   Show per-file progress
  -h, --help      Show this help
EOF
    exit 0
}

# Parse arguments
while [[ $# -gt 0 ]]; do
    case "$1" in
        -v|--verbose) VERBOSE=true; shift ;;
        -h|--help) usage ;;
        *) echo "Error: Unknown argument: $1" >&2; usage ;;
    esac
done

# Find clang-format executable
CLANG_FORMAT=""
for cmd in /usr/lib/llvm-21/bin/clang-format clang-format-21 clang-format-18 clang-format-17 clang-format; do
    if command -v "$cmd" &> /dev/null; then
        CLANG_FORMAT="$cmd"
        break
    fi
done

if [ -z "$CLANG_FORMAT" ]; then
    echo "Error: clang-format not found. Please install clang-format."
    exit 1
fi

if $VERBOSE; then
    echo "Using: $CLANG_FORMAT"
fi
echo "Checking code formatting..."

cd "$PROJECT_ROOT"

# Find all .cpp and .h files in src and tests directories
VIOLATIONS_FOUND=0

# Count files for progress
FILE_COUNT=$(find src tests -type f \( -name "*.cpp" -o -name "*.h" \) ! -path "*/build/*" ! -path "*/.git/*" 2>/dev/null | wc -l)
CHECKED_COUNT=0

while IFS= read -r -d '' file; do
    CHECKED_COUNT=$((CHECKED_COUNT + 1))
    if $VERBOSE; then
        echo "[$CHECKED_COUNT/$FILE_COUNT] Checking: $(basename "$file")"
    fi
    # Check if file would be modified by clang-format
    if ! "$CLANG_FORMAT" --dry-run -Werror "$file" &> /dev/null; then
        echo "  Formatting violation: $file"
        VIOLATIONS_FOUND=1
    fi
done < <(find src tests -type f \( -name "*.cpp" -o -name "*.h" \) ! -path "*/build/*" ! -path "*/.git/*" -print0 2>/dev/null)

if [ $VIOLATIONS_FOUND -eq 1 ]; then
    echo ""
    echo "Code formatting violations found!"
    echo "Run './tools/clang-format.sh' to fix formatting."
    exit 1
else
    echo "All $FILE_COUNT files are properly formatted."
    exit 0
fi
