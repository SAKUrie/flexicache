# FlexiCache Architecture Design Document

## 📐 System Architecture Overview

FlexiCache is a dynamic code management system prototype running in the RISC-V QEMU environment, simulating the code transfer mechanism in heterogeneous memory systems.

```
┌─────────────────────────────────────────────────────────────┐
│                    QEMU RISC-V (virt board)                  │
│                                                               │
│  ┌───────────────────────────────────────────────────────┐  │
│  │            Physical Memory (Start: 0x80000000)         │  │
│  │                                                         │  │
│  │  ┌──────────────────────────────────────────────────┐ │  │
│  │  │  I-Mem (Fast Instruction Memory)                 │ │  │
│  │  │  0x80000000 - 0x800FFFFF (1MB)                   │ │  │
│  │  │  ┌────────────────────────────────────────────┐  │ │  │
│  │  │  │  Runtime Library Code (flexicache.c)      │  │ │  │
│  │  │  │  - flexicache_init()                       │  │ │  │
│  │  │  │  - flexicache_load_block()                 │  │ │  │
│  │  │  │  - flexicache_evict_block()                │  │ │  │
│  │  │  └────────────────────────────────────────────┘  │ │  │
│  │  │  ┌────────────────────────────────────────────┐  │ │  │
│  │  │  │  Dynamic Cache Area (from 0x80010000)     │  │ │  │
│  │  │  │  - Runtime loaded user code               │  │ │  │
│  │  │  └────────────────────────────────────────────┘  │ │  │
│  │  └──────────────────────────────────────────────────┘ │  │
│  │                                                         │  │
│  │  ┌──────────────────────────────────────────────────┐ │  │
│  │  │  DRAM (Slow Main Memory)                         │ │  │
│  │  │  0x80100000 - 0x801FFFFF (1MB)                   │ │  │
│  │  │  ┌────────────────────────────────────────────┐  │ │  │
│  │  │  │  User Code (.text.user)                    │  │ │  │
│  │  │  │  - fibonacci()                             │  │ │  │
│  │  │  │  - factorial()                             │  │ │  │
│  │  │  │  - sum_array()                             │  │ │  │
│  │  │  └────────────────────────────────────────────┘  │ │  │
│  │  │  ┌────────────────────────────────────────────┐  │ │  │
│  │  │  │  Data Segment (.data, .bss)                │  │ │  │
│  │  │  └────────────────────────────────────────────┘  │ │  │
│  │  │  ┌────────────────────────────────────────────┐  │ │  │
│  │  │  │  Heap (64KB)                               │  │ │  │
│  │  │  └────────────────────────────────────────────┘  │ │  │
│  │  │  ┌────────────────────────────────────────────┐  │ │  │
│  │  │  │  Stack (64KB) ↓ Grows downward             │  │ │  │
│  │  │  └────────────────────────────────────────────┘  │ │  │
│  │  └──────────────────────────────────────────────────┘ │  │
│  └───────────────────────────────────────────────────────┘  │
└─────────────────────────────────────────────────────────────┘
```

## 🔑 Core Design Decisions

### 1. Why Use Linker Script Instead of Real Hardware?

**Problem**: Real heterogeneous memory systems (like I-Mem + DRAM) are difficult to simulate in QEMU.

**Solution**: Use linker script `flexicache.ld` to "virtually" partition unified physical memory into two regions:

```ld
MEMORY {
    IMEM (rwx) : ORIGIN = 0x80000000, LENGTH = 1M
    DRAM (rwx) : ORIGIN = 0x80100000, LENGTH = 1M
}
```

**Advantages**:
- ✅ No need to modify QEMU source code
- ✅ Can compile with standard toolchain
- ✅ Sufficient to prove "runtime transfer logic" feasibility in reports

### 2. Why Use Macros Instead of Binary Rewriter?

**Problem**: A complete system needs a preprocessor to automatically insert `flexicache_load_block()` before function calls.

**Time Constraint**: 3-4 days of development time cannot implement a complete ELF binary parser and modifier.

**MVP Solution**: Use C macro `CALL_MANAGED` to simulate:

```c
#define CALL_MANAGED(func, ...) ({ \
    flexicache_load_block((void*)func, 256); \
    func(__VA_ARGS__); \
})
```

**Usage**:
```c
// Original call
int result = fibonacci(10);

// FlexiCache managed call
int result = CALL_MANAGED(fibonacci, 10);
```

**Future Extension**: Can implement real binary rewriter using Python + `pyelftools`.

### 3. Runtime Library Responsibilities

`runtime/flexicache.c` is the core of the entire system, responsible for:

#### 3.1 Initialization (`flexicache_init`)
- Set up I-Mem allocator (available from `0x80010000`)
- Initialize statistics counters
- Output debug information via UART

#### 3.2 Code Loading (`flexicache_load_block`)

**Flowchart**:
```
Start
  ↓
Is function address in I-Mem?
  ├─ Yes → Hit ✓ → Return 0
  └─ No → Miss ✗
           ↓
      Is address in DRAM?
           ├─ No → Error → Return -1
           └─ Yes ↓
           Is I-Mem space sufficient?
                ├─ No → Evict old code (evict_block)
                └─ Yes ↓
           memcpy(target, src, size)
                ↓
           Update allocator pointer
                ↓
           fence.i (Flush instruction cache)
                ↓
           Update statistics
                ↓
           Return 0
```

#### 3.3 Eviction Policy (`flexicache_evict_block`)

**Current Implementation**: Simply clears entire I-Mem cache area (except runtime library)

**Rationale**: MVP phase prioritizes validating "transfer logic" feasibility

**Future Improvements**:
- LRU (Least Recently Used)
- FIFO (First In First Out)
- Intelligent prefetch based on function call frequency

## 📊 Key Data Structures

### Statistics (`flexicache_stats_t`)

```c
typedef struct {
    uint32_t load_count;      // Load count
    uint32_t evict_count;     // Eviction count
    uint32_t hit_count;       // Hit count
    uint32_t miss_count;      // Miss count
    uint32_t total_bytes;     // Total transferred bytes
} flexicache_stats_t;
```

### I-Mem Allocator (`imem_allocator_t`)

```c
typedef struct {
    void *start;              // Current available space start address
    size_t available;         // Remaining available space
} imem_allocator_t;
```

## 🔄 Complete Execution Flow

### Startup Phase

```
1. QEMU loads flexicache_demo.elf
2. CPU jumps to _start (defined in main.c)
3. _start initializes stack pointer: sp = 0x801FXXXX
4. _start clears BSS segment
5. Call flexicache_init()
   ├─ Initialize I-Mem allocator
   ├─ Clear statistics
   └─ Print initialization message
```

### Function Call Phase

```
User code: int result = CALL_MANAGED(fibonacci, 10);

Macro expansion:
  flexicache_load_block((void*)fibonacci, 256);
  ↓
  Check if fibonacci address (0x801XXXXX) is in I-Mem?
  ↓ No (first call)
  ↓
  Allocate I-Mem space: target = 0x80010000
  ↓
  memcpy(0x80010000, 0x801XXXXX, 256)
  ↓
  fence.i (Synchronize instruction stream)
  ↓
  miss_count++, load_count++, total_bytes += 256
  ↓
  Return 0
  
  fibonacci(10);  // Execute function
```

### Calling Same Function Again

```
Second time: int result = CALL_MANAGED(fibonacci, 15);

flexicache_load_block((void*)fibonacci, 256);
  ↓
  Check if fibonacci address is in I-Mem?
  ↓ Yes (already loaded)
  ↓
  hit_count++
  ↓
  Return 0 directly (no transfer needed)

fibonacci(15);  // Execute function
```

## 🛠️ Compilation Toolchain

### GCC Compilation Options Explained

```makefile
CFLAGS = -march=rv32ima -mabi=ilp32 -O2 -g -Wall -Wextra
```

| Option | Meaning |
|--------|---------|
| `-march=rv32ima` | Target architecture: 32-bit RISC-V, supports Integer(I), Multiply(M), Atomic(A) instructions |
| `-mabi=ilp32` | ABI: Integer, Long, Pointer are all 32-bit |
| `-O2` | Optimization level: Balance performance and debug capability |
| `-g` | Include debug information |
| `-static` | Static linking |
| `-nostdlib` | Don't link standard library |
| `-nostartfiles` | Don't use standard startup files |
| `-ffreestanding` | Bare-metal environment |

### Link Options

```makefile
LDFLAGS = -T scripts/flexicache.ld -Wl,-Map=flexicache_demo.map
```

- `-T scripts/flexicache.ld`: Use custom linker script
- `-Wl,-Map=...`: Generate memory map file (for debugging)

## 📏 Memory Layout Details

### I-Mem Region (0x80000000 - 0x800FFFFF)

| Offset | Size | Content | Description |
|--------|------|---------|-------------|
| 0x00000000 | ~64KB | Runtime library code | Fixed, will not be evicted |
| 0x00010000 | ~960KB | Dynamic cache area | Temporary copy of user code |

### DRAM Region (0x80100000 - 0x801FFFFF)

| Segment | Size | Purpose |
|---------|------|---------|
| .text.user | Variable | Original code of user functions |
| .rodata | Variable | Read-only data (string constants, etc.) |
| .data | Variable | Initialized global variables |
| .bss | Variable | Uninitialized global variables |
| .heap | 64KB | Dynamic memory allocation |
| .stack | 64KB | Function call stack |

## 🧪 Test Coverage

### Test Cases

| Test | Purpose | Expected Behavior |
|------|---------|-------------------|
| `fibonacci(10)` | First call | Miss → Load to I-Mem |
| `fibonacci(15)` | Repeat call | Hit → Return directly |
| `factorial(8)` | New function | Miss → Load to I-Mem |
| `sum_array()` | Function with parameters | Miss → Load to I-Mem |

### Validation Logic

```c
if (result1 == 55 && result2 == 610 && 
    result3 == 40320 && result4 == 55) {
    puts("✓ All tests passed!");
}
```

## 🚀 Performance Metrics

### Theoretical Analysis

Assumptions:
- I-Mem access latency: 1 cycle
- DRAM access latency: 100 cycles
- Code transfer cost: N bytes × 100 cycles

**Hit Scenario** (function in I-Mem):
```
Total latency = Function execution time (using I-Mem speed)
```

**Miss Scenario** (first call):
```
Total latency = Transfer time + Function execution time
              = (256 bytes × 100 cycles) + Function execution time
              = 25,600 cycles + Function execution time
```

### Practical Measurement Method

Can count instructions via QEMU's `-icount` option:

```bash
qemu-system-riscv32 -icount shift=0 -d exec,nochain ...
```

## 🔮 Future Extensions

### Short-term (1-2 weeks)

- [ ] Implement real LRU cache replacement
- [ ] Add Python script to automatically extract function sizes
- [ ] Support multi-level cache (L1-I, L2)

### Medium-term (1-2 months)

- [ ] Implement binary rewriter (Python + pyelftools)
- [ ] Support Position Independent Code (PIC)
- [ ] Add hardware performance counter simulation

### Long-term (3+ months)

- [ ] Validate on real FPGA board
- [ ] Support multi-threading/multi-core
- [ ] Machine learning driven intelligent prefetch

## 📖 Related Concepts

### RISC-V `fence.i` Instruction

**Purpose**: Synchronize instruction stream and data stream

**Scenario**: After we modify the code segment (e.g., dynamic loading), we must execute `fence.i`, otherwise CPU may execute old cached instructions.

```c
void flexicache_flush_icache(void) {
    asm volatile ("fence.i" ::: "memory");
}
```

### `ALIGN` in Linker Script

```ld
.text.user : ALIGN(4) { ... }
```

Ensures segment start address is 4-byte aligned, RISC-V requires instruction alignment.

### QEMU virt Board

This is a general-purpose virtual development board provided by QEMU, features:
- RAM start address: `0x80000000`
- UART address: `0x10000000`
- Supports Device Tree

## 🎓 Suggested Learning Path

1. **Beginner**: Run examples → Modify test functions → Observe statistics
2. **Intermediate**: Read linker script → Understand memory layout → Modify memory sizes
3. **Advanced**: Study runtime library → Implement LRU algorithm → Add performance analysis
4. **Expert**: GDB debugging → Disassembly analysis → Modify compilation options for optimization

---

**Design Philosophy**: Validate core ideas in the simplest way, avoid premature optimization.
