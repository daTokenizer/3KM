# 3KM
Three key Madness (3KM) is a short C code based on the Techkeys.us 3-key business card firmware for minimal keyboards.

## main features 

1. short/long press deffrentiation - two different macros per key per mode (see below)
2. two modes - holding all three keys will change modes -  two entire sets of macros ("mode changed" will be printed to context)
3. mode LED support - connect LEDs from PD4 to ground and from PD5 to ground to see what mode you are in (you may then also want to disable the mode change print)

## Configurimg with your macros

1. clone/download the code
2. in the file `main_techkeys.c` you'll find arrays of keys named `keys1`..`keys3` - 1,2&3 is the index of the button (they are numberd left to right) and without any suffix it will denote what happens on a short press in the default mode. The `_long` suffix is used to mark the macro for long presses and `_b` marks that this macro belongs to the second mode.
3. the key consts are listed in the file `usb_keyboard.h`
4. to hold shift on a certian charecter (for capital letters, '~', ':', etc.) add KSHIFT as the key previus to the key you want capitalized - so `K2` will produce '2', but `KSHIFT, K2` will produce '@'

### example
`uint8_t keys3_b_long[] = {KC, KD, KSPACE, KSHIFT, KTILDE, KSLASH, KS, KR, KC}; ` will produce `cd ~/src` when holding the rightmost button in mode 2 



## Deploy The Firmware to the chip

1. Create a firmware using `make`
2. Put your keyboard into bootloader mode
3. Program your .hex file (replace device type as necessary, your file should be in the `./bin` dir)

		sudo dfu-programmer atmega16u2 erase
		sleep 10
		sudo dfu-programmer atmega16u2 flash /path/to/firmware.hex

AVR-dude may also be used on Linux, as explained [here](https://geekhack.org/index.php?topic=51252.msg2066099#msg2066099).
