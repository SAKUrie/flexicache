/* [核心] 运行时库 (搬运工逻辑) */

#include "flexicache.h"

/* ========== 内部数据结构 ========== */

/* 简单的 I-Mem 分配器（模拟缓存管理） */
typedef struct {
    void *start;         /* 当前可用空间起始地址 */
    size_t available;    /* 剩余可用空间 */
} imem_allocator_t;

/* 代码块映射表项 */
#define MAX_CACHED_BLOCKS 64
typedef struct {
    void *dram_addr;     /* DRAM 中的原始地址 */
    void *imem_addr;     /* I-Mem 中的加载地址 */
    size_t size;         /* 代码块大小 */
} block_mapping_t;

static imem_allocator_t g_imem_alloc;
static flexicache_stats_t g_stats;
static block_mapping_t g_block_map[MAX_CACHED_BLOCKS];
static int g_block_count = 0;

/* ========== 辅助函数 ========== */

/* 简单的 memcpy 实现（避免依赖 libc） */
static void* fc_memcpy(void *dst, const void *src, size_t n) {
    uint8_t *d = (uint8_t*)dst;
    const uint8_t *s = (const uint8_t*)src;
    while (n--) {
        *d++ = *s++;
    }
    return dst;
}

/* 简单的 memset 实现 */
static void* fc_memset(void *s, int c, size_t n) {
    uint8_t *p = (uint8_t*)s;
    while (n--) {
        *p++ = (uint8_t)c;
    }
    return s;
}

/* UART 输出（QEMU virt 板的串口地址：0x10000000） */
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

/* 简单的十六进制打印 */
static void uart_puthex(uint32_t val) {
    const char hex[] = "0123456789ABCDEF";
    uart_puts("0x");
    for (int i = 28; i >= 0; i -= 4) {
        uart_putc(hex[(val >> i) & 0xF]);
    }
}

/* ========== API 实现 ========== */

/**
 * 初始化 FlexiCache 系统
 */
void flexicache_init(void) {
    /* 初始化 I-Mem 分配器 */
    /* 注意：运行时库本身在 I-Mem 的开头，我们从偏移 0x10000 开始分配 */
    g_imem_alloc.start = (void*)(IMEM_BASE + 0x10000);
    g_imem_alloc.available = IMEM_SIZE - 0x10000;
    
    /* 清空统计信息 */
    fc_memset(&g_stats, 0, sizeof(g_stats));
    
    /* 清空映射表 */
    g_block_count = 0;
    fc_memset(g_block_map, 0, sizeof(g_block_map));
    
    uart_puts("\n[FlexiCache] 初始化完成\n");
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

/**
 * 加载代码块到 I-Mem
 */
int flexicache_load_block(void *func_addr, size_t size) {
    /* 参数检查 */
    if (func_addr == NULL || size == 0) {
        return -1;
    }
    
    /* 首先检查映射表，看是否已经加载过（命中检测） */
    for (int i = 0; i < g_block_count; i++) {
        if (g_block_map[i].dram_addr == func_addr) {
            /* 找到了！这是一个缓存命中 */
            g_stats.hit_count++;
            uart_puts("[FlexiCache] 缓存命中！\n");
            return 0;  /* 已经在I-Mem中，无需重新加载 */
        }
    }
    
    /* 如果函数不在 DRAM 中，错误 */
    if (!flexicache_is_dram(func_addr)) {
        uart_puts("[FlexiCache] 错误: 无效地址\n");
        return -1;
    }
    
    /* 未命中 - 需要加载 */
    g_stats.miss_count++;
    
    /* 对齐到缓存块大小 */
    size_t aligned_size = (size + CACHE_BLOCK_SIZE - 1) & ~(CACHE_BLOCK_SIZE - 1);
    
    /* 如果 I-Mem 空间不足，需要驱逐旧数据 */
    if (aligned_size > g_imem_alloc.available) {
        uart_puts("[FlexiCache] I-Mem 空间不足，触发驱逐...\n");
        if (flexicache_evict_block(aligned_size) != 0) {
            return -1;
        }
    }
    
    /* 执行搬运操作 */
    void *target = g_imem_alloc.start;
    uart_puts("[FlexiCache] 加载代码块: ");
    uart_puthex((uint32_t)func_addr);
    uart_puts(" -> ");
    uart_puthex((uint32_t)target);
    uart_puts(" (");
    uart_puthex(aligned_size);
    uart_puts(" 字节)\n");
    
    fc_memcpy(target, func_addr, size);
    
    /* 更新分配器状态 */
    g_imem_alloc.start = (void*)((uintptr_t)g_imem_alloc.start + aligned_size);
    g_imem_alloc.available -= aligned_size;
    
    /* 添加到映射表 */
    if (g_block_count < MAX_CACHED_BLOCKS) {
        g_block_map[g_block_count].dram_addr = func_addr;
        g_block_map[g_block_count].imem_addr = target;
        g_block_map[g_block_count].size = aligned_size;
        g_block_count++;
    } else {
        uart_puts("[FlexiCache] 警告: 映射表已满\n");
    }
    
    /* 刷新指令缓存 */
    flexicache_flush_icache();
    
    /* 更新统计 */
    g_stats.load_count++;
    g_stats.total_bytes += aligned_size;
    
    return 0;
}

/**
 * 驱逐 I-Mem 中的代码块
 */
int flexicache_evict_block(size_t size) {
    /* 简单策略：直接重置分配器（相当于清空整个 I-Mem） */
    /* 在真实系统中，这里应该实现 LRU/FIFO 等策略 */
    
    uart_puts("[FlexiCache] 驱逐所有代码块\n");
    
    g_imem_alloc.start = (void*)(IMEM_BASE + 0x10000);
    g_imem_alloc.available = IMEM_SIZE - 0x10000;
    
    /* 清空映射表 */
    g_block_count = 0;
    fc_memset(g_block_map, 0, sizeof(g_block_map));
    
    g_stats.evict_count++;
    
    return 0;
}

/**
 * 刷新指令缓存
 */
void flexicache_flush_icache(void) {
    /* RISC-V fence.i 指令：同步指令流 */
    asm volatile ("fence.i" ::: "memory");
}

/**
 * 获取统计信息
 */
const flexicache_stats_t* flexicache_get_stats(void) {
    return &g_stats;
}

/**
 * 打印统计信息
 */
void flexicache_print_stats(void) {
    uart_puts("\n========== FlexiCache 统计信息 ==========\n");
    uart_puts("加载次数: ");
    uart_puthex(g_stats.load_count);
    uart_putc('\n');
    uart_puts("驱逐次数: ");
    uart_puthex(g_stats.evict_count);
    uart_putc('\n');
    uart_puts("命中次数: ");
    uart_puthex(g_stats.hit_count);
    uart_putc('\n');
    uart_puts("未命中次数: ");
    uart_puthex(g_stats.miss_count);
    uart_putc('\n');
    uart_puts("总搬运字节数: ");
    uart_puthex(g_stats.total_bytes);
    uart_putc('\n');
    uart_puts("=========================================\n");
}

