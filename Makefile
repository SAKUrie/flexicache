# 一键编译和运行脚本

# 工具链配置
CC = riscv64-unknown-elf-gcc
OBJDUMP = riscv64-unknown-elf-objdump
OBJCOPY = riscv64-unknown-elf-objcopy

# 编译选项
ARCH = rv32ima
ABI = ilp32
CFLAGS = -march=$(ARCH) -mabi=$(ABI) -O2 -g -Wall -Wextra
CFLAGS += -static -nostdlib -nostartfiles -ffreestanding
CFLAGS += -I./runtime

# 链接选项
LDFLAGS = -T scripts/flexicache.ld
LDFLAGS += -Wl,-Map=flexicache_demo.map

# 目标文件
RUNTIME_OBJ = runtime/flexicache.o
MAIN_OBJ = src/main.o
TARGET = flexicache_demo.elf
BINARY = flexicache_demo.bin

# QEMU 配置
QEMU = qemu-system-riscv32
QEMU_FLAGS = -machine virt -m 256M -nographic
QEMU_FLAGS += -kernel $(TARGET)

.PHONY: all clean run debug disasm docker-build docker-run

all: $(TARGET) $(BINARY)

# 编译 ELF 文件
$(TARGET): $(RUNTIME_OBJ) $(MAIN_OBJ)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^
	@echo "✓ 编译完成: $(TARGET)"
	@echo "✓ 内存映射: flexicache_demo.map"

# 生成二进制文件
$(BINARY): $(TARGET)
	$(OBJCOPY) -O binary $< $@
	@echo "✓ 二进制文件: $(BINARY)"

# 编译运行时库
$(RUNTIME_OBJ): runtime/flexicache.c runtime/flexicache.h
	$(CC) $(CFLAGS) -c -o $@ runtime/flexicache.c

# 编译主程序
$(MAIN_OBJ): src/main.c runtime/flexicache.h
	$(CC) $(CFLAGS) -c -o $@ src/main.c

# 反汇编（查看生成的代码）
disasm: $(TARGET)
	$(OBJDUMP) -d $(TARGET) > flexicache_demo.asm
	@echo "✓ 反汇编文件: flexicache_demo.asm"

# 在 QEMU 中运行
run: $(TARGET)
	@echo "=== 启动 QEMU (按 Ctrl+A, X 退出) ==="
	$(QEMU) $(QEMU_FLAGS)

# 调试模式（等待 GDB 连接）
debug: $(TARGET)
	@echo "=== QEMU 调试模式 (GDB 端口: 1234) ==="
	@echo "在另一个终端运行: gdb-multiarch $(TARGET)"
	@echo "然后在 GDB 中执行: target remote :1234"
	$(QEMU) $(QEMU_FLAGS) -s -S

# 清理
clean:
	rm -f $(TARGET) $(BINARY) $(RUNTIME_OBJ) $(MAIN_OBJ)
	rm -f flexicache_demo.map flexicache_demo.asm
	@echo "✓ 清理完成"

# Docker 命令
docker-build:
	docker build -t flexicache-env .

docker-run:
	docker run -it --rm -v $$(pwd):/workspace flexicache-env

# 帮助信息
help:
	@echo "FlexiCache Makefile 使用说明"
	@echo "============================"
	@echo "make all      - 编译项目"
	@echo "make run      - 在 QEMU 中运行"
	@echo "make debug    - 启动调试模式"
	@echo "make disasm   - 生成反汇编文件"
	@echo "make clean    - 清理编译文件"
	@echo "make docker-build - 构建 Docker 镜像"
	@echo "make docker-run   - 在 Docker 中运行"

