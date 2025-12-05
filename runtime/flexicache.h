#ifndef FLEXICACHE_H
#define FLEXICACHE_H

#include <stdint.h>
#include <stddef.h>

/* Memory layout (QEMU virt platform) */
#define IMEM_BASE    0x80000000   /* I-Mem base (1MB) */
#define IMEM_SIZE    0x00100000
#define DRAM_BASE    0x80100000   /* DRAM base (1MB) */
#define DRAM_SIZE    0x00100000

#define CACHE_BLOCK_SIZE  64

/* Runtime statistics */
typedef struct {
    uint32_t load_count;
    uint32_t evict_count;
    uint32_t hit_count;
    uint32_t miss_count;
    uint32_t total_bytes;
} flexicache_stats_t;

/* Public API */

/* Initialize FlexiCache (must be called first) */
void flexicache_init(void);

/* Load code block from DRAM to I-Mem */
int flexicache_load_block(void *func_addr, size_t size);

/* Evict blocks to make space */
int flexicache_evict_block(size_t size);

/* Flush instruction cache (RISC-V fence.i) */
void flexicache_flush_icache(void);

/* Get runtime statistics */
const flexicache_stats_t* flexicache_get_stats(void);

/* Print statistics for debugging */
void flexicache_print_stats(void);

/* Check if address is in I-Mem */
static inline int flexicache_is_imem(void *addr) {
    uintptr_t a = (uintptr_t)addr;
    return (a >= IMEM_BASE && a < IMEM_BASE + IMEM_SIZE);
}

/* Check if address is in DRAM */
static inline int flexicache_is_dram(void *addr) {
    uintptr_t a = (uintptr_t)addr;
    return (a >= DRAM_BASE && a < DRAM_BASE + DRAM_SIZE);
}

/* Macro to wrap function calls with load logic
 * In a real system, the preprocessor would insert this automatically
 * Usage: int result = CALL_MANAGED(my_function, arg1, arg2);
 */
#define CALL_MANAGED(func, ...) ({ \
    flexicache_load_block((void*)func, 256); \
    func(__VA_ARGS__); \
})

#endif /* FLEXICACHE_H */

