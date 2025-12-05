/* [核心] 自定义链接脚本 (定义 I-Mem vs DRAM) */
/* 
 * 关键设计思想：
 * QEMU virt 板的 RAM 从 0x80000000 开始
 * 我们将其人为切分成两部分：
 *   - I-Mem:  0x80000000 - 0x800FFFFF (1MB)  存放运行时库和关键代码
 *   - DRAM:   0x80100000 - 0x801FFFFF (1MB)  存放用户代码和数据
 */

OUTPUT_ARCH(riscv)
ENTRY(_start)

/* 内存布局定义 */
MEMORY
{
    /* 模拟指令内存 (I-Mem) - 快速、小容量 */
    IMEM (rwx) : ORIGIN = 0x80000000, LENGTH = 1M
    
    /* 模拟数据内存 (DRAM) - 慢速、大容量 */
    DRAM (rwx) : ORIGIN = 0x80100000, LENGTH = 1M
}

/* 定义关键符号 */
__imem_start = ORIGIN(IMEM);
__imem_end = ORIGIN(IMEM) + LENGTH(IMEM);
__dram_start = ORIGIN(DRAM);
__dram_end = ORIGIN(DRAM) + LENGTH(DRAM);

SECTIONS
{
    /* ===== I-Mem 区域 ===== */
    
    /* 启动代码（必须在最前面） */
    .text.start : ALIGN(4) {
        *(.text.start)
        *(.text.init)
    } > IMEM
    
    /* 运行时库代码（放在 I-Mem，因为它是"搬运工"） */
    .text.runtime : ALIGN(4) {
        runtime/*.o(.text .text.*)
        runtime/*.o(.rodata .rodata.*)
    } > IMEM
    
    /* 其他系统代码 */
    .text.system : ALIGN(4) {
        *(.text.system)
    } > IMEM
    
    /* ===== DRAM 区域 ===== */
    
    /* 用户代码（可以被动态搬运） */
    .text.user : ALIGN(4) {
        __user_code_start = .;
        src/*.o(.text .text.*)
        __user_code_end = .;
    } > DRAM
    
    /* 只读数据 */
    .rodata : ALIGN(4) {
        *(.rodata .rodata.*)
        *(.srodata .srodata.*)
    } > DRAM
    
    /* 数据段 */
    .data : ALIGN(4) {
        __data_start = .;
        *(.data .data.*)
        *(.sdata .sdata.*)
        __data_end = .;
    } > DRAM
    
    /* BSS 段（未初始化数据） */
    .bss : ALIGN(4) {
        __bss_start = .;
        *(.bss .bss.*)
        *(.sbss .sbss.*)
        *(COMMON)
        __bss_end = .;
    } > DRAM
    
    /* 堆（用于动态内存分配） */
    .heap : ALIGN(4) {
        __heap_start = .;
        . = . + 0x10000;  /* 64KB 堆空间 */
        __heap_end = .;
    } > DRAM
    
    /* 栈（从高地址向低地址增长） */
    .stack (NOLOAD) : ALIGN(16) {
        . = . + 0x10000;  /* 64KB 栈空间 */
        __stack_top = .;
    } > DRAM
    
    /* 全局指针（用于访问小数据段） */
    __global_pointer$ = MIN(__data_start + 0x800, MAX(__data_start + 0x800, __bss_end - 0x800));
}

