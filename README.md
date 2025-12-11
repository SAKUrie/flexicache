# FlexiCache - RISC-V Dynamic Code Management System

A flexible cache management system prototype based on the QEMU RISC-V environment, simulating the dynamic code transfer process between fast instruction memory (I-Mem) and slow DRAM.

## 🎯 Core Design Philosophy

FlexiCache addresses a core problem in **heterogeneous memory systems**: how to efficiently manage code between limited fast storage (I-Mem) and large-capacity slow storage (DRAM)?

### Key Features

- ✅ **Linker Script Simulates Hardware**: Uses custom linker scripts to "virtually" partition QEMU's unified memory into I-Mem and DRAM
- ✅ **Runtime Dynamic Loading**: Implements dynamic code block transfer, eviction, and cache management
- ✅ **Source-Level Instrumentation**: Uses `CALL_MANAGED` macro to simulate automatic preprocessor instrumentation
- ✅ **Statistics**: Tracks hit rate, miss rate, transferred bytes, and other key metrics
- ✅ **Fully Containerized**: Docker environment for one-click build, no local toolchain installation required

## 📁 Project Structure

```
flexicache/
├── Dockerfile              # Docker image containing RISC-V toolchain and QEMU
├── Makefile                # One-click compile, run, and debug script
├── README.md               # Project documentation (this file)
├── scripts/
│   └── flexicache.ld       # [Core] Custom linker script (defines I-Mem vs DRAM)
├── runtime/
│   ├── flexicache.c        # [Core] Runtime library (transfer logic)
│   └── flexicache.h        # Header file and API definitions
└── src/
    └── main.c              # User test program (demonstrates FlexiCache functionality)
```

## 🚀 Quick Start

### Method 1: Using Docker (Recommended)

This is the simplest method, works on Windows, macOS, and Linux.

```bash
# 1. Build Docker image (first use or after code updates)
docker build -t flexicache-env .

# 2. Enter container environment
docker run -it --rm -v $(pwd):/workspace flexicache-env

# 3. Compile and run inside container
make          # Compile project
make run      # Run in QEMU

# 4. Exit QEMU: Press Ctrl+A, then X
```

### Method 2: Local Compilation (Requires Toolchain)

If you already have the RISC-V toolchain installed:

```bash
# 1. Compile
make

# 2. Run
make run

# 3. View disassembly (optional)
make disasm

# 4. Clean
make clean
```

## 🔧 Core Components Explained

### 1. Linker Script (`scripts/flexicache.ld`)

**Purpose**: Artificially partitions QEMU virt board's RAM (starting at `0x80000000`) into two regions:

- **I-Mem**: `0x80000000 - 0x800FFFFF` (1MB) - Stores runtime library and critical code
- **DRAM**: `0x80100000 - 0x801FFFFF` (1MB) - Stores user code and data

**Key Code**:

```ld
MEMORY
{
    IMEM (rwx) : ORIGIN = 0x80000000, LENGTH = 1M
    DRAM (rwx) : ORIGIN = 0x80100000, LENGTH = 1M
}
```

### 2. Runtime Library (`runtime/flexicache.c`)

**Purpose**: Implements code block transfer logic, simulating a cache management system.

**Core API**:

```c
void flexicache_init(void);                          // Initialize
int flexicache_load_block(void *func_addr, size_t);  // Load code block
int flexicache_evict_block(size_t size);             // Evict code block
void flexicache_flush_icache(void);                  // Flush instruction cache
```

**Workflow**:
1. Check if function is in I-Mem (hit/miss determination)
2. If miss, copy from DRAM to I-Mem
3. If I-Mem space insufficient, trigger eviction policy
4. Use RISC-V `fence.i` instruction to flush instruction cache

### 3. Instrumentation Macro (`CALL_MANAGED`)

In a complete system, a binary rewriter should automatically insert transfer logic. In this prototype, we use a macro to simulate:

```c
// Usage
int result = CALL_MANAGED(fibonacci, 10);

// Equivalent to
flexicache_load_block((void*)fibonacci, 256);
int result = fibonacci(10);
```

### 4. Test Program (`src/main.c`)

Demo program includes multiple test cases:
- `fibonacci(10)` - First call, triggers load
- `fibonacci(15)` - Second call, should hit
- `factorial(8)` - New function, triggers new load
- `sum_array()` - Array operation test

## 📊 Expected Output

After running `make run`, you should see output similar to:

```
========================================
   FlexiCache Demo
========================================

[FlexiCache] Init complete
[FlexiCache] I-Mem: 0x80000000 - 0x80100000
[FlexiCache] DRAM:  0x80100000 - 0x80200000

[Test 1] Calling fibonacci(10) - First call (miss)
[FlexiCache] Loading block: 0x80100XXX -> 0x80010000 (256 bytes)
Result: 55

[Test 2] Calling fibonacci(15) again - Should hit
Result: 610

========== FlexiCache Statistics ==========
Load count: 0x00000004
Eviction count: 0x00000000
Hit count: 0x00000001
Miss count: 0x00000003
Total bytes transferred: 0x00000400
===========================================

✓ All tests passed!
```

## 🐛 Debug Mode

FlexiCache supports GDB debugging:

```bash
# Terminal 1: Start QEMU in debug mode
make debug

# Terminal 2: Connect GDB
gdb-multiarch flexicache_demo.elf
(gdb) target remote :1234
(gdb) b flexicache_load_block    # Set breakpoint
(gdb) c                          # Continue execution
(gdb) info registers             # View registers
(gdb) x/10i $pc                  # View current instructions
```

## 📝 Technical Details

### Memory Layout

| Region | Start Address | Size | Purpose |
|--------|---------------|------|---------|
| I-Mem (Runtime) | 0x80000000 | ~64KB | Runtime library code |
| I-Mem (Cache) | 0x80010000 | ~960KB | Dynamically loaded code |
| DRAM (Code) | 0x80100000 | Variable | User functions |
| DRAM (Data) | 0x801XXXXX | Variable | Global variables |
| DRAM (Heap) | 0x801YXXXX | 64KB | Dynamic allocation |
| DRAM (Stack) | 0x801FXXXX | 64KB | Function call stack |

### Compilation Options

- **Architecture**: RV32IMA (32-bit RISC-V, Integer+Multiply+Atomic instructions)
- **ABI**: ILP32 (Integers and pointers are both 32-bit)
- **Optimization**: `-O2` (Balance between speed and debug capability)
- **Special Options**: `-nostdlib -ffreestanding` (Bare-metal program)

## 🎓 Use Cases

This prototype is suitable as:

1. **Course Project**: Demonstrates memory management concepts in OS/Computer Architecture
2. **Research Prototype**: Validates dynamic code management algorithms
3. **Learning Material**: Understanding linker scripts, runtime systems, RISC-V assembly
4. **Proof of Concept**: Quick validation before patent/paper applications

## ⚠️ Current Limitations

This is an **MVP (Minimum Viable Product)** with the following simplifications:

1. **Simple Eviction Policy**: Currently clears entire I-Mem, LRU/FIFO not implemented
2. **Static Size Assumption**: `CALL_MANAGED` macro assumes each function is 256 bytes
3. **No Address Translation**: Direct code copy, Position Independent Code (PIC) issues not handled
4. **Single-threaded**: Concurrent access not considered

## 🚧 Future Extension Directions

If you have more time, consider:

- [ ] Implement real LRU cache replacement algorithm
- [ ] Add Python script to automatically analyze ELF files and determine function sizes
- [ ] Support Position Independent Code (PIC), implement real address relocation
- [ ] Add performance analysis tools, generate execution heatmaps
- [ ] Implement Prefetch strategy, preload functions that may be used

## 📚 References

- [RISC-V Instruction Set Manual](https://riscv.org/technical/specifications/)
- [QEMU RISC-V Documentation](https://www.qemu.org/docs/master/system/target-riscv.html)
- [GNU Linker Script Syntax](https://sourceware.org/binutils/docs/ld/Scripts.html)

## 📄 License

MIT License - Free for academic and commercial projects

## 🙋 Troubleshooting

If you encounter issues, please check:
1. Is Docker correctly installed? (`docker --version`)
2. Are you running commands in the project root directory?
3. Check `flexicache_demo.map` file to inspect memory layout

---

**Enjoy using it! If this project helps you, please give it a ⭐️**
