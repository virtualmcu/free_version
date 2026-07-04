/* STM32F103C8T6 — 64K Flash, 20K RAM */
MEMORY {
    FLASH (rx)  : ORIGIN = 0x08000000, LENGTH = 64K
    RAM   (rwx) : ORIGIN = 0x20000000, LENGTH = 20K
}

ENTRY(Reset_Handler)

SECTIONS {
    .isr_vector : { KEEP(*(.isr_vector)) } > FLASH
    .text : { *(.text*) *(.rodata*) _etext = .; } > FLASH
    .data : AT(_etext) { _sdata = .; *(.data*) _edata = .; } > RAM
    .bss  : { _sbss = .; *(.bss*) *(COMMON) _ebss = .; } > RAM
    _stack_top = ORIGIN(RAM) + LENGTH(RAM);
}
