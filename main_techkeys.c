#include "platforms.h"
#include <avr/io.h>
#include <avr/pgmspace.h>
#include <avr/interrupt.h>
#include <avr/power.h>
#include <util/delay.h>
#include <stdbool.h>

#include "main.h"
#include "usb_keyboard.h"
#include "io.h"
#include "hid.h"
#include "timer.h"
#include "auxiliary.h"

#define BUTTON_NUM 3

volatile bool should_scan = false;



uint8_t mode_change[] = {KM, KO, KD, KE, KSPACE, KC, KH, KA, KN, KG, KE, KD}; // mode change

// MODE 1 (default)

uint8_t keys0[] = {KH, KT, KT, KP, KSHIFT, KSEMICOLON, KSLASH, KSLASH, KW, KW, KW, KPERIOD, KA, KD, KA, KM, KL, KE, KV, KPERIOD, KC, KO, KM};

uint8_t keys1[] = {KG, KI, KT, KSPACE, KS, KT, KA, KT, KU, KS, KSPACE, KMINUS, KV}; // git status -v

uint8_t keys2[] = {KG, KI, KT, KSPACE, KP, KU, KL, KL}; // git pull

uint8_t keys3[] = {KSHIFT, KTILDE, KSLASH, KR, KU, KN, KSHIFT, KMINUS, KA, KL, KL, KSHIFT, KMINUS, KT, KE, KS, KT, KS}; // ~/run_all_tests

uint8_t keys1_long[] = {KG, KI, KT, KSPACE, KC, KH, KE, KC, KK, KO, KU, KT, KSPACE, KR, KE, KL, KE, KA, KS, KE, KSLASH, KEYPAD_7, KPERIOD, KEYPAD_0, KPERIOD, KEYPAD_1, KEYPAD_0, KEYPAD_0};// git checkout release/7.0.100

uint8_t keys2_long[] = {KG, KI, KT, KSPACE, KR, KE, KB, KA, KS, KE, KSPACE, KO, KR, KI, KG, KI, KN, KSLASH, KR, KE, KL, KE, KA, KS, KE, KSLASH, KEYPAD_7, KPERIOD, KEYPAD_0, KPERIOD, KEYPAD_1, KEYPAD_0, KEYPAD_0}; // git rebase origin/release/7.0.100

uint8_t keys3_long[] = {KG, KI, KT, KSPACE, KP, KU, KS, KH, KSPACE, KMINUS, KMINUS, KF, KO, KR, KC, KE}; // git push --force



uint8_t *keys[] = {mode_change, keys1, keys2, keys3, keys1_long, keys2_long, keys3_long};
uint8_t keys_s[] = {ARR_SZ(mode_change), ARR_SZ(keys1), ARR_SZ(keys2), ARR_SZ(keys3), ARR_SZ(keys1_long), ARR_SZ(keys2_long), ARR_SZ(keys3_long)};


uint8_t keys1_b[] = {KX, KC, KA, KL, KC}; // xcalc

uint8_t keys2_b[] = {KG, KI, KT, KSPACE, KP, KU, KL, KL}; // git pull

uint8_t keys3_b[] = {KC, KD, KSPACE, KSHIFT, KTILDE, KSLASH, KS, KR, KC}; // cd ~/src

uint8_t keys1_b_long[] = {KPERIOD, KSLASH, KR, KU, KN}; // ./run

uint8_t keys2_b_long[] = {KG, KI, KT, KSPACE, KP, KU, KS, KH}; // git push

uint8_t keys3_b_long[] = {KC, KD, KSPACE, KSHIFT, KTILDE, KSLASH, KS, KR, KC, KSLASH, KG, KI, KT, KSHIFT, KH, KU, KB}; // cd ~/src/gitHub




// MODE 2

uint8_t *keys_b[] = {mode_change, keys1_b, keys2_b, keys3_b, keys1_b_long, keys2_b_long, keys3_b_long};
uint8_t keys_s_b[] = {ARR_SZ(mode_change), ARR_SZ(keys1_b), ARR_SZ(keys2_b), ARR_SZ(keys3_b), ARR_SZ(keys1_b_long), ARR_SZ(keys2_b_long), ARR_SZ(keys3_b_long)};


// // GPIO pins
// GPIO_PINS[6] = {
// 	_BV(PC2), // GPIO 1
// 	_BV(PD0), // GPIO 2
// 	_BV(PD3), // GPIO 4
// 	_BV(PD6), // GPIO 3
// 	_BV(PD4), // GPIO 5
// 	_BV(PD5)  // GPIO 6
// };

void _toggle_mode_lights(int mode)
{
	if (mode==1)
	{
		PORTD |= _BV(PD5); // GPIO 5
		PORTD &= ~(_BV(PD4));  // GPIO 6
	}
	else //mode = 0
	{
		PORTD |= _BV(PD4); // GPIO 5
		PORTD &= ~(_BV(PD5));  // GPIO 6
	}
}


void _clear_context(void)
{
	HID_set_scancode_state(KCTRL, true);
	HID_commit_state();
	_delay_ms(3);
	HID_set_scancode_state(KL, true);
	HID_commit_state();
	_delay_ms(3);
	HID_set_scancode_state(KL, false);
	HID_commit_state();
	_delay_ms(3);
	HID_set_scancode_state(KCTRL, false);
	HID_commit_state();
	_delay_ms(100);
}

void _toggle_shift(bool should_hold)
{
	_delay_ms(3);
	HID_set_scancode_state(KSHIFT, should_hold);
	HID_commit_state();
	_delay_ms(3);
}

void _hit_enter(void)
{
	HID_set_scancode_state(KENTER, true);
	HID_commit_state();
	_delay_ms(3);
	HID_set_scancode_state(KENTER, false);
	HID_commit_state();
	_delay_ms(3);
}

bool _is_button_pressed(int btn_index)
{
	return (!IO_get(3+btn_index));
}

bool _is_button_long_pressed(int btn_index)
{
	_delay_ms(50);
	if (_is_button_pressed(btn_index))
	{
		_delay_ms(300);
		return _is_button_pressed(btn_index);
	}
	return false;
}

int _wait_buttons_press(void)
{

	int count = 0;
	int pressed_i = 1;
	int first_count = 0;
	int second_count = 0;
	while (first_count == 0)
	{
		for (int i = 1; i <= BUTTON_NUM; ++i) {
			if (_is_button_pressed(i))
			{
				pressed_i = i;
				count++;
			}
		}
		first_count = count;
	}
	_delay_ms(50);
	count = 0;
	for (int i = 1; i <= BUTTON_NUM; ++i) {
		if (_is_button_pressed(i))
		{
			count++;
		}
	}
	second_count = count;

	if ((first_count > 1) || (second_count > 1))
	{
		return 0;
	}
	else if(_is_button_long_pressed(pressed_i))
	{
		return -pressed_i;
	}
	else
	{
		return pressed_i;
	}
}

void _click_key(uint8_t key)
{
	IO_set(0, false);
	HID_set_scancode_state(key, true);
	HID_commit_state();
	_delay_ms(2);
	IO_set(0, true);
	_delay_ms(10);
	HID_set_scancode_state(key, false);
	HID_commit_state();
}

int main(void)
{
	int mode = 1;
	clock_prescale_set(clock_div_1);

	USB_init();
	HID_commit_state();

	for (int i = 0; i < 4; ++i) {
		IO_set(i, true);
		IO_config(i, OUTPUT);
	}
	IO_config(4, INPUT);
	IO_config(5, INPUT);
	IO_config(6, INPUT);
	IO_set(4, true);
	IO_set(5, true);
	IO_set(6, true);

	TIMER_init();

	IO_set(0, false);
	_delay_ms(10);
	IO_set(0, true);

	_toggle_mode_lights(mode);
	while (true) {
		uint8_t *k = NULL;
		uint8_t s = 0;
		// cycle through buttons for click

		int btn_index = _wait_buttons_press();

		if (btn_index == 0)
		{
			mode = mode?0:1; //flip mode
			_toggle_mode_lights(mode);
		}

		if (btn_index < 0)
		{
			btn_index = BUTTON_NUM - btn_index;
		}
		
		if (mode)
		{
			k = keys[btn_index];
			s = keys_s[btn_index];
		}
		else
		{
			k = keys_b[btn_index];
			s = keys_s_b[btn_index];
		}

		if (!k)
			continue;

		//_clear_context(); 

		bool holding_shift = false;
		for (int i = 0; i < s; ++i) {
			if (k[i] == KSHIFT) {
				_toggle_shift(true);
				holding_shift = true;
				continue;
			}

			_click_key(k[i]);

			if (holding_shift) {
				_toggle_shift(false);
				holding_shift = false;
			}
			_delay_ms(20);
		}
		_hit_enter();
	}

}

void MAIN_timer_handler()
{
	for (int i = 0; i < BUTTON_NUM; ++i)
		IO_set(1+i, IO_get(4+i));
}

void MAIN_handle_sof()
{}
