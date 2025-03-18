# Makefile for BeagleV-Ahead bare metal application

# Toolchain
CROSS_COMPILE = riscv64-linux-gnu-
CC = $(CROSS_COMPILE)gcc
AS = $(CROSS_COMPILE)gcc
LD = $(CROSS_COMPILE)gcc
OBJCOPY = $(CROSS_COMPILE)objcopy
OBJDUMP = $(CROSS_COMPILE)objdump

# Flags
ARCH_FLAGS = -march=rv64gc -mabi=lp64d
CFLAGS = $(ARCH_FLAGS) -ffreestanding -O2 -Wall -Wextra
ASFLAGS = $(ARCH_FLAGS)
LDFLAGS = -nostdlib -nostartfiles $(ARCH_FLAGS) -T linker.ld

# Source files
SRCS_C = main.c uart.c
SRCS_ASM = startup.S
OBJS = $(SRCS_C:.c=.o) $(SRCS_ASM:.S=.o)

# Output files
TARGET = bare_app
ELF = $(TARGET).elf
BIN = $(TARGET).bin
MAP = $(TARGET).map
LST = $(TARGET).lst

# Default target
all: $(BIN) $(LST)

# Compile C files
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Compile assembly files
%.o: %.S
	$(AS) $(ASFLAGS) -c $< -o $@

# Link
$(ELF): $(OBJS) linker.ld
	$(LD) $(LDFLAGS) -Wl,-Map=$(MAP) $(OBJS) -o $@

# Generate binary
$(BIN): $(ELF)
	$(OBJCOPY) -O binary $< $@

# Generate listing for debugging
$(LST): $(ELF)
	$(OBJDUMP) -D $< > $@

# Clean
clean:
	rm -f $(OBJS) $(ELF) $(BIN) $(MAP) $(LST)

# Phony targets
.PHONY: all clean

# Helpful debug target to dump UART address from u-boot
uart-probe:
	@echo "To find UART base address in U-Boot, use:"
	@echo "  => md 0x10000000 10  # Try various addresses to locate UART registers"

# Install to SD card (adjust path as needed)
install: $(BIN)
	@echo "Copying $(BIN) to SD card..."
	cp $(BIN) /media/$(USER)/BOOT/

# Run directly from U-Boot
uboot-guide:
	@echo "To run from U-Boot:"
	@echo "  => fatload mmc 0:1 0x80000000 $(BIN)"
	@echo "  => go 0x80000000"