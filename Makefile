# One-click compile and run script

# Toolchain configuration
CC = riscv64-unknown-elf-gcc
OBJDUMP = riscv64-unknown-elf-objdump
OBJCOPY = riscv64-unknown-elf-objcopy

# Compilation options
ARCH = rv32ima
ABI = ilp32
CFLAGS = -march=$(ARCH) -mabi=$(ABI) -O2 -g -Wall -Wextra
CFLAGS += -static -nostdlib -nostartfiles -ffreestanding
CFLAGS += -I./runtime

# Link options
LDFLAGS = -T scripts/flexicache.ld
LDFLAGS += -Wl,-Map=flexicache_demo.map

# Object files
RUNTIME_OBJ = runtime/flexicache.o
MAIN_OBJ = src/main.o
TARGET = flexicache_demo.elf
BINARY = flexicache_demo.bin

# QEMU configuration
QEMU = qemu-system-riscv32
QEMU_FLAGS = -machine virt -m 256M -nographic
QEMU_FLAGS += -kernel $(TARGET)

.PHONY: all clean run debug disasm docker-build docker-run

all: $(TARGET) $(BINARY)

# Compile ELF file
$(TARGET): $(RUNTIME_OBJ) $(MAIN_OBJ)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^
	@echo "✓ Compilation complete: $(TARGET)"
	@echo "✓ Memory map: flexicache_demo.map"

# Generate binary file
$(BINARY): $(TARGET)
	$(OBJCOPY) -O binary $< $@
	@echo "✓ Binary file: $(BINARY)"

# Compile runtime library
$(RUNTIME_OBJ): runtime/flexicache.c runtime/flexicache.h
	$(CC) $(CFLAGS) -c -o $@ runtime/flexicache.c

# Compile main program
$(MAIN_OBJ): src/main.c runtime/flexicache.h
	$(CC) $(CFLAGS) -c -o $@ src/main.c

# Disassembly (view generated code)
disasm: $(TARGET)
	$(OBJDUMP) -d $(TARGET) > flexicache_demo.asm
	@echo "✓ Disassembly file: flexicache_demo.asm"

# Run in QEMU
run: $(TARGET)
	@echo "=== Starting QEMU (Press Ctrl+A, X to exit) ==="
	$(QEMU) $(QEMU_FLAGS)

# Debug mode (wait for GDB connection)
debug: $(TARGET)
	@echo "=== QEMU Debug Mode (GDB port: 1234) ==="
	@echo "In another terminal run: gdb-multiarch $(TARGET)"
	@echo "Then in GDB execute: target remote :1234"
	$(QEMU) $(QEMU_FLAGS) -s -S

# Clean
clean:
	rm -f $(TARGET) $(BINARY) $(RUNTIME_OBJ) $(MAIN_OBJ)
	rm -f flexicache_demo.map flexicache_demo.asm
	@echo "✓ Cleanup complete"

# Docker commands
docker-build:
	docker build -t flexicache-env .

docker-run:
	docker run -it --rm -v $$(pwd):/workspace flexicache-env

# Help information
help:
	@echo "FlexiCache Makefile Usage"
	@echo "========================="
	@echo "make all      - Compile project"
	@echo "make run      - Run in QEMU"
	@echo "make debug    - Start debug mode"
	@echo "make disasm   - Generate disassembly file"
	@echo "make clean    - Clean compiled files"
	@echo "make docker-build - Build Docker image"
	@echo "make docker-run   - Run in Docker"
