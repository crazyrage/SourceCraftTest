#!/bin/bash

# SourcePawn 1.6 to 1.7+ Syntax Migration Script
# Automatically migrates old SourcePawn syntax to modern standards

set -e

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

# Counters
total_files=0
migrated_files=0
skipped_files=0

echo "========================================"
echo "SourcePawn Syntax Migration Script"
echo "========================================"
echo ""

# Get directory to process
DIR=${1:-.}

if [ ! -d "$DIR" ]; then
    echo -e "${RED}Error: Directory $DIR does not exist${NC}"
    exit 1
fi

echo "Processing directory: $DIR"
echo ""

# Function to migrate a single file
migrate_file() {
    local file="$1"
    local backup="${file}.bak"

    # Create backup
    cp "$file" "$backup"

    # Check if file has old syntax
    if ! grep -q "new String:\|new Float:\|new bool:\|new Handle:\|decl String:\|public Plugin:\|public Action:\|public bool:" "$file"; then
        # File is already using new syntax or has no declarations
        rm "$backup"
        return 1
    fi

    # Apply migrations using sed

    # 1. Replace "new const String:" with "char " (constants)
    sed -i 's/new const String:/char /g' "$file"

    # 2. Replace "new String:" with "char "
    sed -i 's/new String:/char /g' "$file"

    # 3. Replace "new Float:" with "float "
    sed -i 's/new Float:/float /g' "$file"

    # 4. Replace "new bool:" with "bool "
    sed -i 's/new bool:/bool /g' "$file"

    # 5. Replace "new Handle:" with "Handle "
    sed -i 's/new Handle:/Handle /g' "$file"

    # 6. Replace "decl String:" with "char "
    sed -i 's/decl String:/char /g' "$file"

    # 7. Replace "decl Float:" with "float "
    sed -i 's/decl Float:/float /g' "$file"

    # 8. Replace "decl bool:" with "bool "
    sed -i 's/decl bool:/bool /g' "$file"

    # 9. Replace "public Plugin:" with "public Plugin "
    sed -i 's/public Plugin:/public Plugin /g' "$file"

    # 10. Replace "public Action:" with "public Action "
    sed -i 's/public Action:/public Action /g' "$file"

    # 11. Replace "public bool:" with "public bool "
    sed -i 's/public bool:/public bool /g' "$file"

    # 12. Replace "public Float:" with "public float "
    sed -i 's/public Float:/public float /g' "$file"

    # 13. Replace function parameter "Handle:" with "Handle "
    sed -i 's/\bHandle:\([a-zA-Z_][a-zA-Z0-9_]*\)/Handle \1/g' "$file"

    # 14. Replace function parameter "String:" followed by array with "char "
    sed -i 's/\bString:\([a-zA-Z_][a-zA-Z0-9_]*\[\]/char \1/g' "$file"

    # 15. Replace function parameter "bool:" with "bool "
    sed -i 's/\bbool:\([a-zA-Z_][a-zA-Z0-9_]*\)/bool \1/g' "$file"

    # 16. Replace function parameter "Float:" with "float "
    sed -i 's/\bFloat:\([a-zA-Z_][a-zA-Z0-9_]*\)/float \1/g' "$file"

    # 17. Replace "const String:" with "const char " (in parameters)
    sed -i 's/const String:/const char /g' "$file"

    # 18. Fix Plugin info struct syntax (name= to name =)
    sed -i 's/name="/name = "/g' "$file"
    sed -i 's/author="/author = "/g' "$file"
    sed -i 's/description="/description = "/g' "$file"
    sed -i 's/version="/version = "/g' "$file"
    sed -i 's/url="/url = "/g' "$file"

    # Success
    rm "$backup"
    return 0
}

# Find and process all .sp files
while IFS= read -r -d '' file; do
    ((total_files++))

    filename=$(basename "$file")
    echo -n "Processing: $filename ... "

    if migrate_file "$file"; then
        ((migrated_files++))
        echo -e "${GREEN}✓ Migrated${NC}"
    else
        ((skipped_files++))
        echo -e "${YELLOW}⊘ Skipped (already modern syntax)${NC}"
    fi

done < <(find "$DIR" -name "*.sp" -type f -print0)

echo ""
echo "========================================"
echo "Migration Complete!"
echo "========================================"
echo "Total files processed: $total_files"
echo -e "Files migrated: ${GREEN}$migrated_files${NC}"
echo -e "Files skipped: ${YELLOW}$skipped_files${NC}"
echo ""
echo "Next steps:"
echo "1. Review changes: git diff"
echo "2. Test compilation"
echo "3. Commit changes: git add . && git commit"
echo ""
