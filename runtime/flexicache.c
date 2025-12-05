/* FlexiCache runtime library */

#include "flexicache.h"

/* Internal data structures */

typedef struct {
    void *start;
    size_t available;
} imem_allocator_t;

#define MAX_CACHED_BLOCKS 64
typedef struct {
    void *dram_addr;
    void *imem_addr;
    size_t size;
} block_mapping_t;

static imem_allocator_t g_imem_alloc;
static flexicache_stats_t g_stats;
static block_mapping_t g_block_map[MAX_CACHED_BLOCKS];
static int g_block_count = 0;

/* Helper functions */

static void* fc_memcpy(void *dst, const void *src, size_t n) {
    uint8_t *d = (uint8_t*)dst;
    const uint8_t *s = (const uint8_t*)src;
    while (n--) {
        *d++ = *s++;
    }
    return dst;
}

static void* fc_memset(void *s, int c, size_t n) {
    uint8_t *p = (uint8_t*)s;
    while (n--) {
        *p++ = (uint8_t)c;
    }
    return s;
}

#define UART_BASE 0x10000000
static void uart_putc(char c) {
    volatile uint8_t *uart = (volatile uint8_t*)UART_BASE;
    *uart = c;
}

static void uart_puts(const char *s) {
    while (*s) {
        if (*s == '\n') uart_putc('\r');
        uart_putc(*s++);
    }
}

static void uart_puthex(uint32_t val) {
    const char hex[] = "0123456789ABCDEF";
    uart_puts("0x");
    for (int i = 28; i >= 0; i -= 4) {
        uart_putc(hex[(val >> i) & 0xF]);
    }
}

/* Public API implementation */

void flexicache_init(void) {
    /* Runtime lib is at I-Mem start, allocate from 0x10000 onwards */
    g_imem_alloc.start = (void*)(IMEM_BASE + 0x10000);
    g_imem_alloc.available = IMEM_SIZE - 0x10000;
    
    fc_memset(&g_stats, 0, sizeof(g_stats));
    
    g_block_count = 0;
    fc_memset(g_block_map, 0, sizeof(g_block_map));
    
    uart_puts("\n[FlexiCache] Init complete\n");
    uart_puts("[FlexiCache] I-Mem: ");
    uart_puthex(IMEM_BASE);
    uart_puts(" - ");
    uart_puthex(IMEM_BASE + IMEM_SIZE);
    uart_putc('\n');
    uart_puts("[FlexiCache] DRAM:  ");
    uart_puthex(DRAM_BASE);
    uart_puts(" - ");
    uart_puthex(DRAM_BASE + DRAM_SIZE);
    uart_putc('\n');
}

int flexicache_load_block(void *func_addr, size_t size) {
    if (func_addr == NULL || size == 0) {
        return -1;
    }
    
    /* Check mapping table for cache hit */
    for (int i = 0; i < g_block_count; i++) {
        if (g_block_map[i].dram_addr == func_addr) {
            g_stats.hit_count++;
            uart_puts("[FlexiCache] Cache hit!\n");
            return 0;
        }
    }
    
    if (!flexicache_is_dram(func_addr)) {
        uart_puts("[FlexiCache] Error: invalid address\n");
        return -1;
    }
    
    g_stats.miss_count++;
    
    size_t aligned_size = (size + CACHE_BLOCK_SIZE - 1) & ~(CACHE_BLOCK_SIZE - 1);
    
    if (aligned_size > g_imem_alloc.available) {
        uart_puts("[FlexiCache] Out of space, evicting...\n");
        if (flexicache_evict_block(aligned_size) != 0) {
            return -1;
        }
    }
    
    void *target = g_imem_alloc.start;
    uart_puts("[FlexiCache] Loading block: ");
    uart_puthex((uint32_t)func_addr);
    uart_puts(" -> ");
    uart_puthex((uint32_t)target);
    uart_puts(" (");
    uart_puthex(aligned_size);
    uart_puts(" bytes)\n");
    
    fc_memcpy(target, func_addr, size);
    
    g_imem_alloc.start = (void*)((uintptr_t)g_imem_alloc.start + aligned_size);
    g_imem_alloc.available -= aligned_size;
    
    if (g_block_count < MAX_CACHED_BLOCKS) {
        g_block_map[g_block_count].dram_addr = func_addr;
        g_block_map[g_block_count].imem_addr = target;
        g_block_map[g_block_count].size = aligned_size;
        g_block_count++;
    } else {
        uart_puts("[FlexiCache] Warning: mapping table full\n");
    }
    
    flexicache_flush_icache();
    
    g_stats.load_count++;
    g_stats.total_bytes += aligned_size;
    
    return 0;
}

int flexicache_evict_block(size_t size) {
    /* Simple policy: reset allocator (flush all)
     * A real system would use LRU/FIFO/etc. */
    
    uart_puts("[FlexiCache] Evicting all blocks\n");
    
    g_imem_alloc.start = (void*)(IMEM_BASE + 0x10000);
    g_imem_alloc.available = IMEM_SIZE - 0x10000;
    
    g_block_count = 0;
    fc_memset(g_block_map, 0, sizeof(g_block_map));
    
    g_stats.evict_count++;
    
    return 0;
}

void flexicache_flush_icache(void) {
    asm volatile ("fence.i" ::: "memory");
}

const flexicache_stats_t* flexicache_get_stats(void) {
    return &g_stats;
}

void flexicache_print_stats(void) {
    uart_puts("\n========== FlexiCache Statistics ==========\n");
    uart_puts("Load count: ");
    uart_puthex(g_stats.load_count);
    uart_putc('\n');
    uart_puts("Eviction count: ");
    uart_puthex(g_stats.evict_count);
    uart_putc('\n');
    uart_puts("Hit count: ");
    uart_puthex(g_stats.hit_count);
    uart_putc('\n');
    uart_puts("Miss count: ");
    uart_puthex(g_stats.miss_count);
    uart_putc('\n');
    uart_puts("Total bytes: ");
    uart_puthex(g_stats.total_bytes);
    uart_putc('\n');
    uart_puts("===========================================\n");
}

