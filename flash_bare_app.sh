#!/bin/bash

# Check if running as root
if ! id | grep -q root; then
    echo "./flash_bare_app.sh must be run as root:"
    echo "sudo ./flash_bare_app.sh"
    exit 1
fi

# Check if binary exists
if [ ! -f bare_app.bin ]; then
    echo "Error: bare_app.bin not found!"
    echo "Please run 'make' first to compile your application."
    exit 1
fi

echo "Flashing bare metal application to BeagleV-Ahead..."
echo "Make sure your board is in USB flash mode (hold USB button while connecting)"

# Flash using fastboot
fastboot flash ram ./u-boot-with-spl.bin
fastboot reboot
sleep 5
fastboot flash uboot ./u-boot-with-spl.bin

# Load your bare application to a specific memory address
# You can adjust the partition or approach based on your needs
fastboot flash boot ./bare_app.bin

# Alternative approach: flash to a raw location 
# fastboot flash:raw 0x1000000 ./bare_app.bin

echo "Flash complete! Rebooting the board..."
fastboot reboot

echo ""
echo "After boot, connect to the serial console and run:"
echo "  => load mmc 0:1 0x80000000 bare_app.bin"
echo "  => go 0x80000000"
echo ""
echo "Or if you flashed to a specific partition, you can load directly from it."