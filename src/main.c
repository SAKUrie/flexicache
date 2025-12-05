/* 用户测试程序 */

#include "flexicache.h"

/* ========== 外部符号（来自链接脚本） ========== */
extern char __stack_top;
extern char __bss_start, __bss_end;

/* ========== 测试函数（放在 DRAM 中） ========== */

/* 这些函数会被放在 DRAM 中，需要通过 FlexiCache 动态加载 */

int fibonacci(int n) {
    if (n <= 1) return n;
    return fibonacci(n - 1) + fibonacci(n - 2);
}

int factorial(int n) {
    if (n <= 1) return 1;
    return n * factorial(n - 1);
}

int sum_array(const int *arr, int len) {
    int sum = 0;
    for (int i = 0; i < len; i++) {
        sum += arr[i];
    }
    return sum;
}

/* 新增测试函数 - 用于展示更复杂的缓存行为 */

int power(int base, int exp) {
    int result = 1;
    for (int i = 0; i < exp; i++) {
        result *= base;
    }
    return result;
}

int gcd(int a, int b) {
    while (b != 0) {
        int temp = b;
        b = a % b;
        a = temp;
    }
    return a;
}

int is_prime(int n) {
    if (n <= 1) return 0;
    if (n <= 3) return 1;
    if (n % 2 == 0 || n % 3 == 0) return 0;
    for (int i = 5; i * i <= n; i += 6) {
        if (n % i == 0 || n % (i + 2) == 0)
            return 0;
    }
    return 1;
}

int multiply(int a, int b) {
    return a * b;
}

int max_of_three(int a, int b, int c) {
    int max = a;
    if (b > max) max = b;
    if (c > max) max = c;
    return max;
}

/* ========== 辅助函数 ========== */

#define UART_BASE 0x10000000
static void putc(char c) {
    volatile char *uart = (volatile char*)UART_BASE;
    *uart = c;
}

static void puts(const char *s) {
    while (*s) {
        if (*s == '\n') putc('\r');
        putc(*s++);
    }
}

static void puthex(uint32_t val) {
    const char hex[] = "0123456789ABCDEF";
    puts("0x");
    for (int i = 28; i >= 0; i -= 4) {
        putc(hex[(val >> i) & 0xF]);
    }
}

static void putdec(int val) {
    if (val < 0) {
        putc('-');
        val = -val;
    }
    char buf[12];
    int i = 0;
    do {
        buf[i++] = '0' + (val % 10);
        val /= 10;
    } while (val > 0);
    while (i > 0) {
        putc(buf[--i]);
    }
}

/* ========== 启动代码 ========== */

void main(void);

void _start(void) __attribute__((section(".text.start"), naked));

void _start(void) {
    /* 汇编启动代码 - 必须先设置栈指针！ */
    asm volatile (
        ".option push\n"
        ".option norelax\n"
        "la gp, __global_pointer$\n"  /* 设置全局指针 */
        ".option pop\n"
        "la sp, __stack_top\n"         /* 设置栈指针 */
        "call main\n"                  /* 调用main函数 */
        "1: j 1b\n"                    /* 无限循环 */
        ::: "memory"
    );
}

void main(void) {
    /* 1. 清空 BSS 段 */
    char *bss = &__bss_start;
    char *bss_end = &__bss_end;
    while (bss < bss_end) {
        *bss++ = 0;
    }
    
    /* 2. 初始化 FlexiCache */
    puts("\n========================================\n");
    puts("   FlexiCache 高级演示程序\n");
    puts("   展示缓存命中、未命中和驱逐机制\n");
    puts("========================================\n");
    
    flexicache_init();
    
    /* ===== 第一轮：首次调用（全部未命中） ===== */
    puts("\n=== 第一轮：首次加载函数 ===\n");
    
    puts("[1.1] fibonacci(10) - 预期：未命中\n");
    int r1 = CALL_MANAGED(fibonacci, 10);
    puts("结果: "); putdec(r1); puts("\n");
    
    puts("[1.2] factorial(5) - 预期：未命中\n");
    int r2 = CALL_MANAGED(factorial, 5);
    puts("结果: "); putdec(r2); puts("\n");
    
    puts("[1.3] power(2, 8) - 预期：未命中\n");
    int r3 = CALL_MANAGED(power, 2, 8);
    puts("结果: "); putdec(r3); puts("\n");
    
    /* ===== 第二轮：重复调用（应该命中） ===== */
    puts("\n=== 第二轮：重复调用相同函数 ===\n");
    
    puts("[2.1] fibonacci(12) - 预期：命中\n");
    int r4 = CALL_MANAGED(fibonacci, 12);
    puts("结果: "); putdec(r4); puts("\n");
    
    puts("[2.2] factorial(7) - 预期：命中\n");
    int r5 = CALL_MANAGED(factorial, 7);
    puts("结果: "); putdec(r5); puts("\n");
    
    puts("[2.3] power(3, 4) - 预期：命中\n");
    int r6 = CALL_MANAGED(power, 3, 4);
    puts("结果: "); putdec(r6); puts("\n");
    
    /* ===== 第三轮：添加新函数（继续加载） ===== */
    puts("\n=== 第三轮：加载更多新函数 ===\n");
    
    int arr[] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    puts("[3.1] sum_array(10 elements) - 预期：未命中\n");
    int r7 = CALL_MANAGED(sum_array, arr, 10);
    puts("结果: "); putdec(r7); puts("\n");
    
    puts("[3.2] gcd(48, 18) - 预期：未命中\n");
    int r8 = CALL_MANAGED(gcd, 48, 18);
    puts("结果: "); putdec(r8); puts("\n");
    
    puts("[3.3] is_prime(17) - 预期：未命中\n");
    int r9 = CALL_MANAGED(is_prime, 17);
    puts("结果: "); putdec(r9); puts("\n");
    
    /* ===== 第四轮：混合调用（命中和未命中） ===== */
    puts("\n=== 第四轮：混合调用测试 ===\n");
    
    puts("[4.1] fibonacci(8) - 预期：命中\n");
    int r10 = CALL_MANAGED(fibonacci, 8);
    puts("结果: "); putdec(r10); puts("\n");
    
    puts("[4.2] multiply(12, 7) - 预期：未命中\n");
    int r11 = CALL_MANAGED(multiply, 12, 7);
    puts("结果: "); putdec(r11); puts("\n");
    
    puts("[4.3] gcd(100, 35) - 预期：命中\n");
    int r12 = CALL_MANAGED(gcd, 100, 35);
    puts("结果: "); putdec(r12); puts("\n");
    
    puts("[4.4] max_of_three(15, 42, 28) - 预期：未命中\n");
    int r13 = CALL_MANAGED(max_of_three, 15, 42, 28);
    puts("结果: "); putdec(r13); puts("\n");
    
    /* ===== 第五轮：再次调用已加载函数（测试命中率） ===== */
    puts("\n=== 第五轮：验证缓存命中 ===\n");
    
    puts("[5.1] sum_array - 预期：命中\n");
    int r14 = CALL_MANAGED(sum_array, arr, 10);
    puts("结果: "); putdec(r14); puts("\n");
    
    puts("[5.2] power(5, 3) - 预期：命中\n");
    int r15 = CALL_MANAGED(power, 5, 3);
    puts("结果: "); putdec(r15); puts("\n");
    
    puts("[5.3] is_prime(23) - 预期：命中\n");
    int r16 = CALL_MANAGED(is_prime, 23);
    puts("结果: "); putdec(r16); puts("\n");
    
    /* 打印最终统计信息 */
    puts("\n");
    flexicache_print_stats();
    
    /* 验证结果 */
    puts("\n========== 验证测试结果 ==========\n");
    int all_correct = 1;
    
    if (r1 != 55) { puts("✗ fibonacci(10) 错误\n"); all_correct = 0; }
    if (r2 != 120) { puts("✗ factorial(5) 错误\n"); all_correct = 0; }
    if (r3 != 256) { puts("✗ power(2,8) 错误\n"); all_correct = 0; }
    if (r4 != 144) { puts("✗ fibonacci(12) 错误\n"); all_correct = 0; }
    if (r5 != 5040) { puts("✗ factorial(7) 错误\n"); all_correct = 0; }
    if (r6 != 81) { puts("✗ power(3,4) 错误\n"); all_correct = 0; }
    if (r7 != 55) { puts("✗ sum_array 错误\n"); all_correct = 0; }
    if (r8 != 6) { puts("✗ gcd(48,18) 错误\n"); all_correct = 0; }
    if (r9 != 1) { puts("✗ is_prime(17) 错误\n"); all_correct = 0; }
    if (r10 != 21) { puts("✗ fibonacci(8) 错误\n"); all_correct = 0; }
    if (r11 != 84) { puts("✗ multiply(12,7) 错误\n"); all_correct = 0; }
    if (r12 != 5) { puts("✗ gcd(100,35) 错误\n"); all_correct = 0; }
    if (r13 != 42) { puts("✗ max_of_three 错误\n"); all_correct = 0; }
    if (r14 != 55) { puts("✗ sum_array(2) 错误\n"); all_correct = 0; }
    if (r15 != 125) { puts("✗ power(5,3) 错误\n"); all_correct = 0; }
    if (r16 != 1) { puts("✗ is_prime(23) 错误\n"); all_correct = 0; }
    
    if (all_correct) {
        puts("✓ 所有 16 个测试全部通过！\n");
    } else {
        puts("✗ 部分测试失败！\n");
    }
    
    puts("\n程序执行完成。按 Ctrl+A, X 退出 QEMU。\n");
    
    /* 无限循环（在_start中处理） */
}

