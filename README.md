# 3KM
Three key Madness (3KM) is a short C code based on the Techkeys.us 3-key business card firmware for minimal keyboards.

#### main features 

1. short/long press deffrentiation - two different macros per key per mode (see below)
2. two modes - holding all three keys will change modes -  two entire sets of macros ("mode changed" will be printed to context)
3. mode LED support - connect LEDs from PD4 to ground and from PD5 to ground to see what mode you are in)

#### Deploy The Firmware to the chip

1. Create a firmware using `make`
2. Put your keyboard into bootloader mode
3. Program your .hex file (replace device type as necessary, your file should be in the `./bin` dir)

		sudo dfu-programmer atmega16u2 erase
		sleep 10
		sudo dfu-programmer atmega16u2 flash /path/to/firmware.hex

AVR-dude may also be used on Linux, as explained [here](https://geekhack.org/index.php?topic=51252.msg2066099#msg2066099).
