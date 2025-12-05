/* Custom linker script for FlexiCache
 * Splits physical RAM into two logical regions:
 *   - I-Mem:  0x80000000 - 0x800FFFFF (1MB) - runtime lib
 *   - DRAM:   0x80100000 - 0x801FFFFF (1MB) - user code/data
 */

OUTPUT_ARCH(riscv)
ENTRY(_start)

MEMORY
{
    IMEM (rwx) : ORIGIN = 0x80000000, LENGTH = 1M
    DRAM (rwx) : ORIGIN = 0x80100000, LENGTH = 1M
}

__imem_start = ORIGIN(IMEM);
__imem_end = ORIGIN(IMEM) + LENGTH(IMEM);
__dram_start = ORIGIN(DRAM);
__dram_end = ORIGIN(DRAM) + LENGTH(DRAM);

SECTIONS
{
    /* I-Mem region */
    
    .text.start : ALIGN(4) {
        *(.text.start)
        *(.text.init)
    } > IMEM
    
    .text.runtime : ALIGN(4) {
        runtime/*.o(.text .text.*)
        runtime/*.o(.rodata .rodata.*)
    } > IMEM
    
    .text.system : ALIGN(4) {
        *(.text.system)
    } > IMEM
    
    /* DRAM region */
    
    .text.user : ALIGN(4) {
        __user_code_start = .;
        src/*.o(.text .text.*)
        __user_code_end = .;
    } > DRAM
    
    .rodata : ALIGN(4) {
        *(.rodata .rodata.*)
        *(.srodata .srodata.*)
    } > DRAM
    
    .data : ALIGN(4) {
        __data_start = .;
        *(.data .data.*)
        *(.sdata .sdata.*)
        __data_end = .;
    } > DRAM
    
    .bss : ALIGN(4) {
        __bss_start = .;
        *(.bss .bss.*)
        *(.sbss .sbss.*)
        *(COMMON)
        __bss_end = .;
    } > DRAM
    
    .heap : ALIGN(4) {
        __heap_start = .;
        . = . + 0x10000;  /* 64KB heap */
        __heap_end = .;
    } > DRAM
    
    .stack (NOLOAD) : ALIGN(16) {
        . = . + 0x10000;  /* 64KB stack */
        __stack_top = .;
    } > DRAM
    
    __global_pointer$ = MIN(__data_start + 0x800, MAX(__data_start + 0x800, __bss_end - 0x800));
}

