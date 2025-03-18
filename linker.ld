MEMORY
{
    RAM (rwx) : ORIGIN = 0x00200000, LENGTH = 16M
}

SECTIONS
{
    .text : {
        *(.text.init)
        *(.text)
    } > RAM
    
    .rodata : {
        *(.rodata)
    } > RAM
    
    .data : {
        *(.data)
    } > RAM
    
    .bss : {
        *(.bss)
    } > RAM
}