#ifndef FLEXICACHE_H
#define FLEXICACHE_H

#include <stdint.h>
#include <stddef.h>

/* ========== 内存区域定义 ========== */
/* 基于 QEMU virt 板的内存布局 */
#define IMEM_BASE    0x80000000   /* I-Mem 起始地址 (1MB) */
#define IMEM_SIZE    0x00100000   /* 1MB */
#define DRAM_BASE    0x80100000   /* DRAM 起始地址 (1MB) */
#define DRAM_SIZE    0x00100000   /* 1MB */

/* 缓存块大小（模拟硬件缓存行） */
#define CACHE_BLOCK_SIZE  64

/* ========== 统计信息结构 ========== */
typedef struct {
    uint32_t load_count;      /* 加载次数 */
    uint32_t evict_count;     /* 驱逐次数 */
    uint32_t hit_count;       /* 命中次数 */
    uint32_t miss_count;      /* 未命中次数 */
    uint32_t total_bytes;     /* 搬运的总字节数 */
} flexicache_stats_t;

/* ========== 函数声明 ========== */

/**
 * 初始化 FlexiCache 系统
 * 必须在使用其他 API 前调用
 */
void flexicache_init(void);

/**
 * 加载代码块到 I-Mem（模拟从 DRAM 搬运到快速缓存）
 * @param func_addr: 函数地址（在 DRAM 中）
 * @param size: 函数大小（字节）
 * @return: 0 成功, -1 失败
 */
int flexicache_load_block(void *func_addr, size_t size);

/**
 * 驱逐 I-Mem 中的代码块（为新代码腾出空间）
 * @param size: 需要腾出的空间大小（字节）
 * @return: 0 成功, -1 失败
 */
int flexicache_evict_block(size_t size);

/**
 * 刷新指令缓存（RISC-V fence.i 指令）
 * 确保代码修改对 CPU 可见
 */
void flexicache_flush_icache(void);

/**
 * 获取统计信息
 * @return: 统计结构指针
 */
const flexicache_stats_t* flexicache_get_stats(void);

/**
 * 打印统计信息（调试用）
 */
void flexicache_print_stats(void);

/**
 * 检查地址是否在 I-Mem 范围内
 */
static inline int flexicache_is_imem(void *addr) {
    uintptr_t a = (uintptr_t)addr;
    return (a >= IMEM_BASE && a < IMEM_BASE + IMEM_SIZE);
}

/**
 * 检查地址是否在 DRAM 范围内
 */
static inline int flexicache_is_dram(void *addr) {
    uintptr_t a = (uintptr_t)addr;
    return (a >= DRAM_BASE && a < DRAM_BASE + DRAM_SIZE);
}

/* ========== 宏：模拟预处理器插桩 ========== */
/*
 * 这个宏会在调用函数前，自动触发"加载代码块"逻辑
 * 在实际系统中，这应该由二进制重写器自动插入
 * 
 * 使用方法：
 *   int result = CALL_MANAGED(my_function, arg1, arg2);
 */
#define CALL_MANAGED(func, ...) ({ \
    flexicache_load_block((void*)func, 256); \
    func(__VA_ARGS__); \
})

#endif /* FLEXICACHE_H */

