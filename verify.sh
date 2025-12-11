#!/bin/bash
# FlexiCache Project Integrity Verification Script

echo "========================================="
echo "   FlexiCache Project Integrity Check"
echo "========================================="
echo ""

# Color definitions
GREEN='\033[0;32m'
RED='\033[0;31m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

# Check counters
total_checks=0
passed_checks=0

# Check functions
check_file() {
    total_checks=$((total_checks + 1))
    if [ -f "$1" ]; then
        echo -e "${GREEN}✓${NC} $1"
        passed_checks=$((passed_checks + 1))
    else
        echo -e "${RED}✗${NC} $1 (missing)"
    fi
}

check_dir() {
    total_checks=$((total_checks + 1))
    if [ -d "$1" ]; then
        echo -e "${GREEN}✓${NC} $1/"
        passed_checks=$((passed_checks + 1))
    else
        echo -e "${RED}✗${NC} $1/ (missing)"
    fi
}

# 1. Check directory structure
echo "1️⃣  Checking directory structure"
echo "-------------------"
check_dir "scripts"
check_dir "runtime"
check_dir "src"
echo ""

# 2. Check core files
echo "2️⃣  Checking core code files"
echo "-------------------"
check_file "Dockerfile"
check_file "Makefile"
check_file "scripts/flexicache.ld"
check_file "runtime/flexicache.c"
check_file "runtime/flexicache.h"
check_file "src/main.c"
echo ""

# 3. Check documentation
echo "3️⃣  Checking documentation files"
echo "-------------------"
check_file "README.md"
check_file "QUICKSTART.md"
check_file "ARCHITECTURE.md"
check_file "PROJECT_SUMMARY.md"
echo ""

# 4. Check file contents (simple validation)
echo "4️⃣  Checking key code contents"
echo "-------------------"

check_content() {
    total_checks=$((total_checks + 1))
    if grep -q "$2" "$1" 2>/dev/null; then
        echo -e "${GREEN}✓${NC} $3"
        passed_checks=$((passed_checks + 1))
    else
        echo -e "${RED}✗${NC} $3"
    fi
}

check_content "runtime/flexicache.c" "flexicache_init" "flexicache_init() function exists"
check_content "runtime/flexicache.c" "flexicache_load_block" "flexicache_load_block() function exists"
check_content "runtime/flexicache.h" "CALL_MANAGED" "CALL_MANAGED macro defined"
check_content "scripts/flexicache.ld" "IMEM" "Linker script defines IMEM"
check_content "scripts/flexicache.ld" "DRAM" "Linker script defines DRAM"
check_content "src/main.c" "_start" "_start entry function exists"
check_content "src/main.c" "fibonacci" "Test function fibonacci exists"
check_content "Makefile" "QEMU" "Makefile contains QEMU configuration"
echo ""

# 5. Check file sizes (ensure not empty files)
echo "5️⃣  Checking file sizes"
echo "-------------------"

check_size() {
    total_checks=$((total_checks + 1))
    if [ -f "$1" ]; then
        size=$(wc -c < "$1" 2>/dev/null)
        if [ "$size" -gt "$2" ]; then
            echo -e "${GREEN}✓${NC} $1 (${size} bytes)"
            passed_checks=$((passed_checks + 1))
        else
            echo -e "${YELLOW}⚠${NC} $1 (${size} bytes, may be too small)"
        fi
    else
        echo -e "${RED}✗${NC} $1 (does not exist)"
    fi
}

check_size "runtime/flexicache.c" 5000
check_size "runtime/flexicache.h" 2000
check_size "scripts/flexicache.ld" 2000
check_size "src/main.c" 3000
check_size "Makefile" 2000
echo ""

# 6. Summary
echo "========================================="
echo "   Check Complete"
echo "========================================="
echo ""
echo -e "Passed: ${GREEN}${passed_checks}${NC} / ${total_checks}"

if [ $passed_checks -eq $total_checks ]; then
    echo -e "${GREEN}🎉 All checks passed! Project is ready.${NC}"
    echo ""
    echo "Next steps:"
    echo "  1. Build Docker image: docker build -t flexicache-env ."
    echo "  2. Enter container: docker run -it --rm -v \$(pwd):/workspace flexicache-env"
    echo "  3. Compile and run: make run"
    exit 0
else
    echo -e "${RED}⚠️  $((total_checks - passed_checks)) check(s) failed${NC}"
    echo ""
    echo "Please check missing files or content."
    exit 1
fi
