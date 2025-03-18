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











When running 
Make clean
Make
sudo fastboot flash boot bare_app.bin


Get this output 


pavel@pavel:~/apps/beagledev$ sudo fastboot flash boot bare_app.bin
Sending 'boot' (0 KB)                              OKAY [  0.007s]
Writing 'boot'                                     OKAY [  0.002s]
Finished. Total time: 0.014s



On other side: 



Starting download of 960 bytes

downloading of 960 bytes finished



And nothing happens. Probably something wrong with my linker/program/uart/who knows...




Within u-boot:
C910 Light# mw.l 0xffe7014000 0x41 1
AC910 Light# 
^^ prints out the "A"
![Screenshot_20250317_183834](https://github.com/user-attachments/assets/446b647b-9b5f-4c24-ba8a-f8e490491e0f)
