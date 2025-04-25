
ENTRY(_boot)

MEMORY {
    RAM (rwx) : ORIGIN = 0xFFE0000000, LENGTH = 0x180000
}

SECTIONS {
    . = 0xFFE0000000;

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

    .bss : {
        __bss_start = .;
        *(.bss*)
        *(COMMON)
        __bss_end = .;
    } > RAM

    /DISCARD/ : {
        *(.eh_frame)
    }
}
