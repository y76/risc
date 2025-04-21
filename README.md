## Beagleboard Serial Connection Setup

### Wiring

- **Yellow wire**: Connect to TX
- **Orange wire**: Connect to RX
- **Black wire**: Connect to GND
- Connect Pi board to USB port on your computer

### Connecting to the Serial Console

1. Find the serial port of the Pi board:

    ```bash
    ls /dev/tty*
    ```

2. Connect to the serial console using tio:

    ```bash
    tio /dev/ttyACM0  # Replace with your actual port
    ```


## Fastboot Setup for TH1520

To use fastboot with the TH1520 board without requiring sudo:

1. Install the udev rule:

    ```bash
    sudo cp 99-thead-fastboot.rules /etc/udev/rules.d/
    ```

2. Add your user to the plugdev group:

    ```bash
    # Check if you're already in the group
    groups

    # If you don't see "plugdev" in the output, add yourself to it
    sudo usermod -aG plugdev $USER
    ```

3. Reload the udev rules:

    ```bash
    sudo udevadm control --reload-rules
    sudo udevadm trigger
    ```

4. Log out and log back in for the group changes to take effect


After completing these steps, `make run` target should work properly.

Final output

```bash
Starting download of 144 bytes

downloading of 144 bytes finished
Baremetal Hello, World!
```
