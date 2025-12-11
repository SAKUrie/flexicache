# FlexiCache

A software-managed instruction cache prototype for RISC-V, implementing dynamic code paging between I-Mem and DRAM.

## Project Overview

FlexiCache demonstrates software-managed instruction caching by:
- Partitioning memory into fast I-Mem and slow DRAM using custom linker scripts
- Implementing runtime code block loading with hit/miss detection
- Tracking cache statistics (load count, hit rate, miss rate)

## Project Structure

```
flexicache/
├── runtime/
│   ├── flexicache.c      # Runtime library implementation
│   └── flexicache.h      # API definitions
├── src/
│   └── main.c            # Test program
├── scripts/
│   └── flexicache.ld     # Linker script (memory partitioning)
├── Dockerfile            # Build environment
└── Makefile              # Build script
```

## Build & Run

### Using Docker (Recommended)

```bash
docker build -t flexicache-env .
docker run -it --rm -v $(pwd):/workspace flexicache-env
make run
```

### Local Build (Requires RISC-V Toolchain)

```bash
make        # Compile
make run    # Run in QEMU
make clean  # Clean build files
```

Exit QEMU: `Ctrl+A`, then `X`

## Memory Layout

| Region | Address Range | Size | Purpose |
|--------|---------------|------|---------|
| I-Mem | 0x80000000 - 0x800FFFFF | 1MB | Runtime + cached code |
| DRAM | 0x80100000 - 0x801FFFFF | 1MB | User code + data |

## Core API

```c
void flexicache_init(void);                           // Initialize system
int  flexicache_load_block(void *addr, size_t size);  // Load code block
int  flexicache_evict_block(size_t size);             // Evict blocks
void flexicache_print_stats(void);                    // Print statistics
```

## Test Results

The test program executes 16 function calls across 8 functions:

| Metric | Value |
|--------|-------|
| Load Count | 8 |
| Hit Count | 8 |
| Miss Count | 8 |
| Hit Rate | 50% |
| Total Bytes | 2048 |

## References

- S. Wuytack et al., "Software-based instruction caching for embedded processors," IEEE TVLSI, 1999
- RISC-V ISA Specification: https://riscv.org/specifications/
