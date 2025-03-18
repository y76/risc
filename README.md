BeagleV-Ahead Bare Metal Development Guide


Set up a cross-compilation toolchain:
sudo apt-get install gcc-riscv64-linux-gnu binutils-riscv64-linux-gnu

Create a minimal project structure:

startup.S - Assembly startup code   (probably not working)
main.c - Main application code      (probably not working)
linker.ld - Linker script           (probably not working)
Makefile - Build script            (probably not working)

Building and Flashing

Compile application:
ymake clean
make

Flash to the board using fastboot:

sudo fastboot flash boot bare_app.bin


UART Testing in U-Boot
To verify the UART functionality directly from U-Boot:


# Write a character ('A') to the UART transmit register

mw.l 0xffe7014000 0x41 1

# Read UART registers

md.l 0xffe7014000 8



Notes

The BeagleV-Ahead UART base address is 0xffe7014000 https://github.com/beagleboard/beaglev-ahead-u-boot/blob/beaglev-v2020.01-1.1.2/arch/riscv/dts/light-beagle.dts
U-Boot loads the kernel at address 0x00200000 https://github.com/beagleboard/beaglev-ahead-u-boot/blob/beaglev-v2020.01-1.1.2/arch/riscv/cpu/start.S
