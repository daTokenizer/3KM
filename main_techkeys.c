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


volatile bool should_scan = false;



uint8_t mode_change[] = {KM, KO, KD, KE, KSPACE, KC, KH, KA, KN, KG, KE, KD}; // mode change

// MODE 1 (default)

uint8_t keys0[] = {KH, KT, KT, KP, KSHIFT, KSEMICOLON, KSLASH, KSLASH, KW, KW, KW,
	KPERIOD, KA, KD, KA, KM, KL, KE, KV, KPERIOD, KC, KO, KM}; // http://www.adamlev.com

uint8_t keys1[] = {KG, KI, KT, KSPACE, KS, KT, KA, KT, KU, KS, KSPACE, KMINUS, KV}; // git status -v

uint8_t keys2[] = {KG, KI, KT, KSPACE, KP, KU, KL, KL}; // git pull

uint8_t keys3[] = {KSHIFT, KTILDE, KSLASH, KR, KU, KN, KSHIFT, KMINUS, KA, 
	KL, KL, KSHIFT, KMINUS, KT, KE, KS, KT, KS}; // ~/run_all_tests

uint8_t keys4[] = {KG, KI, KT, KSPACE, KC, KH, KE, KC, KK, KO, KU, KT, KSPACE, KR, KE, KL, KE, KA, KS, KE, KSLASH, KEYPAD_7, KPERIOD, KEYPAD_0, KPERIOD, KEYPAD_1, KEYPAD_0, KEYPAD_0};// git checkout release/7.0.100

uint8_t keys5[] = {KG, KI, KT, KSPACE, KR, KE, KB, KA, KS, KE, KSPACE, KO, KR, KI, KG, KI, KN, KSLASH, KR, KE, KL, KE, KA, KS, KE, KSLASH, KEYPAD_7, KPERIOD, KEYPAD_0, KPERIOD, KEYPAD_1, KEYPAD_0, KEYPAD_0}; // git rebase origin/release/7.0.100

uint8_t keys6[] = {KG, KI, KT, KSPACE, KP, KU, KS, KH, KSPACE, KMINUS, KMINUS, KF, KO, KR, KC, KE}; // git push --force



uint8_t *keys[] = {mode_change, keys1, keys2, keys3, keys4, keys5, keys6};
uint8_t keys_s[] = {ARR_SZ(mode_change), ARR_SZ(keys1), ARR_SZ(keys2), ARR_SZ(keys3), ARR_SZ(keys4), ARR_SZ(keys5), ARR_SZ(keys6)};


uint8_t keys1_b[] = {KG, KI, KT, KSPACE, KS, KT, KA, KT, KU, KS, KSPACE, KMINUS, KV}; // git status -v

uint8_t keys2_b[] = {KG, KI, KT, KSPACE, KP, KU, KL, KL}; // git pull

uint8_t keys3_b[] = {KSHIFT, KTILDE, KSLASH, KR, KU, KN, KSHIFT, KMINUS, KA, 
	KL, KL, KSHIFT, KMINUS, KT, KE, KS, KT, KS}; // ~/run_all_tests

uint8_t keys4_b[] = {KG, KI, KT, KSPACE, KC, KH, KE, KC, KK, KO, KU, KT, KSPACE, KR, KE, KL, KE, KA, KS, KE, KSLASH, KEYPAD_7, KPERIOD, KEYPAD_0, KPERIOD, KEYPAD_1, KEYPAD_0, KEYPAD_0};// git checkout release/7.0.100

uint8_t keys5_b[] = {KG, KI, KT, KSPACE, KR, KE, KB, KA, KS, KE, KSPACE, KO, KR, KI, KG, KI, KN, KSLASH, KR, KE, KL, KE, KA, KS, KE, KSLASH, KEYPAD_7, KPERIOD, KEYPAD_0, KPERIOD, KEYPAD_1, KEYPAD_0, KEYPAD_0}; // git rebase origin/release/7.0.100

uint8_t keys6_b[] = {KG, KI, KT, KSPACE, KP, KU, KS, KH, KSPACE, KMINUS, KMINUS, KF, KO, KR, KC, KE}; // git push --force




// MODE 2

uint8_t *keys_b[] = {mode_change, keys1_, keys2, keys3, keys4, keys5, keys6};
uint8_t keys_s_b[] = {ARR_SZ(mode_change), ARR_SZ(keys0), ARR_SZ(keys0), ARR_SZ(keys0), ARR_SZ(keys0), ARR_SZ(keys0), ARR_SZ(keys0)};



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

void _hold_shift(bool is_holding)
{
	_delay_ms(3);
	HID_set_scancode_state(KSHIFT, is_holding);
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
	while (true)
	{
		int count = 0;
		int pressed_i = 1;
		for (int i = 1; i <= 3; ++i) {
			if (_is_button_pressed(i))
			{
				count++;
				if (_is_button_long_pressed(i))
				{
					pressed_i = -i;
				}
				else
				{
					pressed_i = i;
				}
			}
		}
		
		if (count > 0)
		{
			if (count > 2)
			{
				return 0;
			}
			else
			{
				return pressed_i;	
			}

		}
	}
}

void _click_key(uint8_t key)
{
	IO_set(0, false);
	HID_set_scancode_state(key, true);
	HID_commit_state();
	_delay_ms(2);
	IO_set(0, true);
	_delay_ms(18);
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

	while (true) {
		uint8_t *k = NULL;
		uint8_t s = 0;
		// cycle through buttons for click

		int btn_index = _wait_buttons_press();

		if (btn_index == 0)
		{
			mode = mode?0:1; //flip mode
		}

		if (btn_index < 0)
		{
			btn_index = 3 - btn_index;
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
				_hold_shift(true);
				holding_shift = true;
				continue;
			}

			_click_key(k[i]);

			if (holding_shift) {
				_hold_shift(false);
				holding_shift = false;
			}
			_delay_ms(20);
		}
		_hit_enter();
	}

}

void MAIN_timer_handler()
{
	for (int i = 0; i < 3; ++i)
		IO_set(1+i, IO_get(4+i));
}

void MAIN_handle_sof()
{
}
