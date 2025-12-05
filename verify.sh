#!/bin/bash
# FlexiCache 项目完整性验证脚本

echo "========================================="
echo "   FlexiCache 项目完整性检查"
echo "========================================="
echo ""

# 颜色定义
GREEN='\033[0;32m'
RED='\033[0;31m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

# 检查计数
total_checks=0
passed_checks=0

# 检查函数
check_file() {
    total_checks=$((total_checks + 1))
    if [ -f "$1" ]; then
        echo -e "${GREEN}✓${NC} $1"
        passed_checks=$((passed_checks + 1))
    else
        echo -e "${RED}✗${NC} $1 (缺失)"
    fi
}

check_dir() {
    total_checks=$((total_checks + 1))
    if [ -d "$1" ]; then
        echo -e "${GREEN}✓${NC} $1/"
        passed_checks=$((passed_checks + 1))
    else
        echo -e "${RED}✗${NC} $1/ (缺失)"
    fi
}

# 1. 检查目录结构
echo "1️⃣  检查目录结构"
echo "-------------------"
check_dir "scripts"
check_dir "runtime"
check_dir "src"
echo ""

# 2. 检查核心文件
echo "2️⃣  检查核心代码文件"
echo "-------------------"
check_file "Dockerfile"
check_file "Makefile"
check_file "scripts/flexicache.ld"
check_file "runtime/flexicache.c"
check_file "runtime/flexicache.h"
check_file "src/main.c"
echo ""

# 3. 检查文档
echo "3️⃣  检查文档文件"
echo "-------------------"
check_file "README.md"
check_file "QUICKSTART.md"
check_file "ARCHITECTURE.md"
check_file "PROJECT_SUMMARY.md"
echo ""

# 4. 检查文件内容（简单验证）
echo "4️⃣  检查关键代码内容"
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

check_content "runtime/flexicache.c" "flexicache_init" "flexicache_init() 函数存在"
check_content "runtime/flexicache.c" "flexicache_load_block" "flexicache_load_block() 函数存在"
check_content "runtime/flexicache.h" "CALL_MANAGED" "CALL_MANAGED 宏定义存在"
check_content "scripts/flexicache.ld" "IMEM" "链接脚本定义 IMEM"
check_content "scripts/flexicache.ld" "DRAM" "链接脚本定义 DRAM"
check_content "src/main.c" "_start" "_start 启动函数存在"
check_content "src/main.c" "fibonacci" "测试函数 fibonacci 存在"
check_content "Makefile" "QEMU" "Makefile 包含 QEMU 配置"
echo ""

# 5. 检查文件大小（确保不是空文件）
echo "5️⃣  检查文件大小"
echo "-------------------"

check_size() {
    total_checks=$((total_checks + 1))
    if [ -f "$1" ]; then
        size=$(wc -c < "$1" 2>/dev/null)
        if [ "$size" -gt "$2" ]; then
            echo -e "${GREEN}✓${NC} $1 (${size} 字节)"
            passed_checks=$((passed_checks + 1))
        else
            echo -e "${YELLOW}⚠${NC} $1 (${size} 字节，可能过小)"
        fi
    else
        echo -e "${RED}✗${NC} $1 (不存在)"
    fi
}

check_size "runtime/flexicache.c" 5000
check_size "runtime/flexicache.h" 2000
check_size "scripts/flexicache.ld" 2000
check_size "src/main.c" 3000
check_size "Makefile" 2000
echo ""

# 6. 总结
echo "========================================="
echo "   检查完成"
echo "========================================="
echo ""
echo -e "通过: ${GREEN}${passed_checks}${NC} / ${total_checks}"

if [ $passed_checks -eq $total_checks ]; then
    echo -e "${GREEN}🎉 所有检查通过！项目已就绪。${NC}"
    echo ""
    echo "下一步："
    echo "  1. 构建 Docker 镜像: docker build -t flexicache-env ."
    echo "  2. 进入容器: docker run -it --rm -v \$(pwd):/workspace flexicache-env"
    echo "  3. 编译运行: make run"
    exit 0
else
    echo -e "${RED}⚠️  有 $((total_checks - passed_checks)) 项检查失败${NC}"
    echo ""
    echo "请检查缺失的文件或内容。"
    exit 1
fi

