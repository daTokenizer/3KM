# 3KM
Three key Madness (3KM) is a short C code based on the Techkeys.us 3-key business card firmware for minimal keyboards.

#### Deploy The Firmware to the chip

1. Create a firmware using `make`
2. Put your keyboard into bootloader mode
3. Program your .hex file (replace device type as necessary, your file should be in the `./bin` dir)

		sudo dfu-programmer atmega16u2 erase
		sleep 10
		sudo dfu-programmer atmega16u2 flash /path/to/firmware.hex
		sudo dfu-programmer atmega16u2 launch

AVR-dude may also be used on Linux, as explained [here](https://geekhack.org/index.php?topic=51252.msg2066099#msg2066099).
