/* FlexiCache test program */

#include "flexicache.h"

/* External symbols from linker script */
extern char __stack_top;
extern char __bss_start, __bss_end;

/* Test functions (placed in DRAM, loaded on demand) */

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

/* Helper functions */

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

/* Bootstrap code */

void main(void);

void _start(void) __attribute__((section(".text.start"), naked));

void _start(void) {
    /* Must setup stack pointer before any C code runs */
    asm volatile (
        ".option push\n"
        ".option norelax\n"
        "la gp, __global_pointer$\n"
        ".option pop\n"
        "la sp, __stack_top\n"
        "call main\n"
        "1: j 1b\n"
        ::: "memory"
    );
}

void main(void) {
    /* Clear BSS */
    char *bss = &__bss_start;
    char *bss_end = &__bss_end;
    while (bss < bss_end) {
        *bss++ = 0;
    }
    
    /* Initialize FlexiCache */
    puts("\n========================================\n");
    puts("   FlexiCache Demo\n");
    puts("   Cache hit/miss behavior\n");
    puts("========================================\n");
    
    flexicache_init();
    
    /* Round 1: Initial loads (all miss) */
    puts("\n=== Round 1: Initial loads ===\n");
    
    puts("[1.1] fibonacci(10) - expect miss\n");
    int r1 = CALL_MANAGED(fibonacci, 10);
    puts("Result: "); putdec(r1); puts("\n");
    
    puts("[1.2] factorial(5) - expect miss\n");
    int r2 = CALL_MANAGED(factorial, 5);
    puts("Result: "); putdec(r2); puts("\n");
    
    puts("[1.3] power(2, 8) - expect miss\n");
    int r3 = CALL_MANAGED(power, 2, 8);
    puts("Result: "); putdec(r3); puts("\n");
    
    /* Round 2: Repeat calls (should hit) */
    puts("\n=== Round 2: Repeat calls ===\n");
    
    puts("[2.1] fibonacci(12) - expect hit\n");
    int r4 = CALL_MANAGED(fibonacci, 12);
    puts("Result: "); putdec(r4); puts("\n");
    
    puts("[2.2] factorial(7) - expect hit\n");
    int r5 = CALL_MANAGED(factorial, 7);
    puts("Result: "); putdec(r5); puts("\n");
    
    puts("[2.3] power(3, 4) - expect hit\n");
    int r6 = CALL_MANAGED(power, 3, 4);
    puts("Result: "); putdec(r6); puts("\n");
    
    /* Round 3: Load more functions */
    puts("\n=== Round 3: New functions ===\n");
    
    int arr[] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    puts("[3.1] sum_array(10) - expect miss\n");
    int r7 = CALL_MANAGED(sum_array, arr, 10);
    puts("Result: "); putdec(r7); puts("\n");
    
    puts("[3.2] gcd(48, 18) - expect miss\n");
    int r8 = CALL_MANAGED(gcd, 48, 18);
    puts("Result: "); putdec(r8); puts("\n");
    
    puts("[3.3] is_prime(17) - expect miss\n");
    int r9 = CALL_MANAGED(is_prime, 17);
    puts("Result: "); putdec(r9); puts("\n");
    
    /* Round 4: Mixed calls */
    puts("\n=== Round 4: Mixed calls ===\n");
    
    puts("[4.1] fibonacci(8) - expect hit\n");
    int r10 = CALL_MANAGED(fibonacci, 8);
    puts("Result: "); putdec(r10); puts("\n");
    
    puts("[4.2] multiply(12, 7) - expect miss\n");
    int r11 = CALL_MANAGED(multiply, 12, 7);
    puts("Result: "); putdec(r11); puts("\n");
    
    puts("[4.3] gcd(100, 35) - expect hit\n");
    int r12 = CALL_MANAGED(gcd, 100, 35);
    puts("Result: "); putdec(r12); puts("\n");
    
    puts("[4.4] max_of_three(15, 42, 28) - expect miss\n");
    int r13 = CALL_MANAGED(max_of_three, 15, 42, 28);
    puts("Result: "); putdec(r13); puts("\n");
    
    /* Round 5: Verify cache hits */
    puts("\n=== Round 5: Verify hits ===\n");
    
    puts("[5.1] sum_array - expect hit\n");
    int r14 = CALL_MANAGED(sum_array, arr, 10);
    puts("Result: "); putdec(r14); puts("\n");
    
    puts("[5.2] power(5, 3) - expect hit\n");
    int r15 = CALL_MANAGED(power, 5, 3);
    puts("Result: "); putdec(r15); puts("\n");
    
    puts("[5.3] is_prime(23) - expect hit\n");
    int r16 = CALL_MANAGED(is_prime, 23);
    puts("Result: "); putdec(r16); puts("\n");
    
    puts("\n");
    flexicache_print_stats();
    
    /* Verify results */
    puts("\n========== Test Results ==========\n");
    int all_correct = 1;
    
    if (r1 != 55) { puts("X fibonacci(10) failed\n"); all_correct = 0; }
    if (r2 != 120) { puts("X factorial(5) failed\n"); all_correct = 0; }
    if (r3 != 256) { puts("X power(2,8) failed\n"); all_correct = 0; }
    if (r4 != 144) { puts("X fibonacci(12) failed\n"); all_correct = 0; }
    if (r5 != 5040) { puts("X factorial(7) failed\n"); all_correct = 0; }
    if (r6 != 81) { puts("X power(3,4) failed\n"); all_correct = 0; }
    if (r7 != 55) { puts("X sum_array failed\n"); all_correct = 0; }
    if (r8 != 6) { puts("X gcd(48,18) failed\n"); all_correct = 0; }
    if (r9 != 1) { puts("X is_prime(17) failed\n"); all_correct = 0; }
    if (r10 != 21) { puts("X fibonacci(8) failed\n"); all_correct = 0; }
    if (r11 != 84) { puts("X multiply(12,7) failed\n"); all_correct = 0; }
    if (r12 != 5) { puts("X gcd(100,35) failed\n"); all_correct = 0; }
    if (r13 != 42) { puts("X max_of_three failed\n"); all_correct = 0; }
    if (r14 != 55) { puts("X sum_array(2) failed\n"); all_correct = 0; }
    if (r15 != 125) { puts("X power(5,3) failed\n"); all_correct = 0; }
    if (r16 != 1) { puts("X is_prime(23) failed\n"); all_correct = 0; }
    
    if (all_correct) {
        puts("All 16 tests passed!\n");
    } else {
        puts("Some tests failed!\n");
    }
    
    puts("\nDone. Press Ctrl+A, X to exit QEMU.\n");
}

