.section .text.init
.globl _start

_start:
    # Set up stack pointer
    li sp, 0x00600000   # Set stack far enough from code
    
    # Jump to main C function
    jal main
    
    # Loop forever if main returns
1:  j 1b