
ENTRY(_start)

MEMORY {
    RAM (rwx) : ORIGIN = 0xFFE0000000, LENGTH = 0x180000
}

SECTIONS {
    . = ORIGIN(RAM);

    .text : {
        *(.text.boot)
        *(.text.start)
        *(.text*)
    } > RAM

    .rodata : {
        *(.rodata*)
    } > RAM

    .pmp_data : ALIGN(4) {
        __pmp_data_start = .;
        KEEP(*(.pmp_data))
    } > RAM

    .data : {
        *(.data*)
    } > RAM

    .bss : ALIGN(8) {
        __bss_start = .;
        *(.bss*)
        *(COMMON)
        __bss_end = .;
    } > RAM

    .stack : ALIGN(16) {
        _stack_top = . + 0x1000; /* 4KB stack grows down from here */
    } > RAM

    /DISCARD/ : {
        *(.eh_frame)
    }
}
