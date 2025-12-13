#!/usr/bin/env bash
# Apply clang-format to GnotePad source files
# Uses .clang-format configuration from project root
# Usage: ./clang-format.sh [OPTIONS]
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(dirname "$SCRIPT_DIR")"

VERBOSE=false

usage() {
    cat <<EOF
Usage: $(basename "$0") [OPTIONS]

Apply clang-format to all GnotePad source files (in-place).
Use check-format.sh to check without modifying files.

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

echo "Applying clang-format to source files..."

# Find clang-format
CLANG_FORMAT=""
if command -v clang-format &> /dev/null; then
    CLANG_FORMAT="clang-format"
elif [[ -x "/usr/lib/llvm-21/bin/clang-format" ]]; then
    CLANG_FORMAT="/usr/lib/llvm-21/bin/clang-format"
elif [[ -x "/usr/lib/llvm-20/bin/clang-format" ]]; then
    CLANG_FORMAT="/usr/lib/llvm-20/bin/clang-format"
else
    echo "Error: clang-format not found" >&2
    exit 1
fi

# Count files for progress
FILE_COUNT=$(find "$PROJECT_ROOT/src" "$PROJECT_ROOT/tests" -type f \( -name "*.cpp" -o -name "*.h" \) ! -path "*/build/*" ! -path "*/.git/*" | wc -l)
CHECKED_COUNT=0

while IFS= read -r -d '' file; do
    CHECKED_COUNT=$((CHECKED_COUNT + 1))
    if $VERBOSE; then
        echo "[$CHECKED_COUNT/$FILE_COUNT] Formatting: $(basename "$file")"
    fi
    $CLANG_FORMAT -i "$file"
done < <(find "$PROJECT_ROOT/src" "$PROJECT_ROOT/tests" -type f \( -name "*.cpp" -o -name "*.h" \) ! -path "*/build/*" ! -path "*/.git/*" -print0)

echo "Formatted $FILE_COUNT files."
