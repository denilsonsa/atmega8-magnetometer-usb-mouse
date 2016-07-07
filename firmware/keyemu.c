/* Name: keyemu.c
 * Project: atmega8-magnetometer-usb-mouse
 * Author: Denilson Figueiredo de Sa
 * Creation Date: 2011-10-18
 * Tabsize: 4
 * License: GNU GPL v2 or GNU GPL v3
 */


// For NULL definition
#include <stddef.h>

#include <avr/pgmspace.h>
#include <string.h>
#include <stdlib.h>

#include "common.h"
#include "keyemu.h"


// HID report
KeyboardReport keyboard_report;

// Pointer to RAM for the string being typed.
uchar *string_output_pointer = NULL;

// Shared output buffer, other functions are free to use this as needed.
uchar string_output_buffer[STRING_OUTPUT_BUFFER_SIZE];


// Keyboard usage values  {{{
// See chapter 10 (Keyboard/Keypad Page) from "USB HID Usage Tables"
// (page 53 of Hut1_12v2.pdf)

#define MOD_CONTROL_LEFT    (1<<0)
#define MOD_SHIFT_LEFT      (1<<1)
#define MOD_ALT_LEFT        (1<<2)
#define MOD_GUI_LEFT        (1<<3)
#define MOD_CONTROL_RIGHT   (1<<4)
#define MOD_SHIFT_RIGHT     (1<<5)
#define MOD_ALT_RIGHT       (1<<6)
#define MOD_GUI_RIGHT       (1<<7)

#define KEY_A          4
#define KEY_B          5
#define KEY_C          6
#define KEY_D          7
#define KEY_E          8
#define KEY_F          9
#define KEY_G          10
#define KEY_H          11
#define KEY_I          12
#define KEY_J          13
#define KEY_K          14
#define KEY_L          15
#define KEY_M          16
#define KEY_N          17
#define KEY_O          18
#define KEY_P          19
#define KEY_Q          20
#define KEY_R          21
#define KEY_S          22
#define KEY_T          23
#define KEY_U          24
#define KEY_V          25
#define KEY_W          26
#define KEY_X          27
#define KEY_Y          28
#define KEY_Z          29
#define KEY_1          30
#define KEY_2          31
#define KEY_3          32
#define KEY_4          33
#define KEY_5          34
#define KEY_6          35
#define KEY_7          36
#define KEY_8          37
#define KEY_9          38
#define KEY_0          39
#define KEY_ENTER      40
#define KEY_ESCAPE     41
#define KEY_TAB        43
#define KEY_SPACE      44
#define KEY_MINUS      45
#define KEY_EQUAL      46
#define KEY_SEMICOLON  51
#define KEY_COMMA      54
#define KEY_PERIOD     55
#define KEY_F1         58
#define KEY_F2         59
#define KEY_F3         60
#define KEY_F4         61
#define KEY_F5         62
#define KEY_F6         63
#define KEY_F7         64
#define KEY_F8         65
#define KEY_F9         66
#define KEY_F10        67
#define KEY_F11        68
#define KEY_F12        69

// }}}


// Mapping between chars and their keyboard codes  {{{

typedef struct KeyAndModifier {
	uchar key;
	uchar modifier;
} KeyAndModifier;


// Warning: ';:' key is c-cedilla in BR-ABNT2 layout
// Warning: '/?' key is ';:' in BR-ABNT2 layout
// Other characters not in this lookup table:
//   \t \n A-Z a-z [\] ^_` {|} ~
static const KeyAndModifier char_to_key[] PROGMEM = {  // {{{
	{KEY_SPACE    , 0             }, // SPACE
	{KEY_1        , MOD_SHIFT_LEFT}, // !
	{0            , 0             }, // "
	{KEY_3        , MOD_SHIFT_LEFT}, // #
	{KEY_4        , MOD_SHIFT_LEFT}, // $
	{KEY_5        , MOD_SHIFT_LEFT}, // %
	{KEY_7        , MOD_SHIFT_LEFT}, // &
	{0            , 0             }, // '
	{KEY_9        , MOD_SHIFT_LEFT}, // (
	{KEY_0        , MOD_SHIFT_LEFT}, // )
	{KEY_8        , MOD_SHIFT_LEFT}, // *
	{KEY_EQUAL    , MOD_SHIFT_LEFT}, // +
	{KEY_COMMA    , 0             }, // ,
	{KEY_MINUS    , 0             }, // -
	{KEY_PERIOD   , 0             }, // .
	{0            , 0             }, // /
	{KEY_0        , 0             }, // 0
	{KEY_1        , 0             }, // 1
	{KEY_2        , 0             }, // 2
	{KEY_3        , 0             }, // 3
	{KEY_4        , 0             }, // 4
	{KEY_5        , 0             }, // 5
	{KEY_6        , 0             }, // 6
	{KEY_7        , 0             }, // 7
	{KEY_8        , 0             }, // 8
	{KEY_9        , 0             }, // 9
	{KEY_SEMICOLON, MOD_SHIFT_LEFT}, // :
	{KEY_SEMICOLON, 0             }, // ;
	{KEY_COMMA    , MOD_SHIFT_LEFT}, // <
	{KEY_EQUAL    , 0             }, // =
	{KEY_PERIOD   , MOD_SHIFT_LEFT}, // >
	{0            , 0             }, // ?
	{KEY_2        , MOD_SHIFT_LEFT}  // @
};  // }}}

// }}}


void init_keyboard_emulation() {  // {{{
	// According to avr-libc FAQ, the compiler automatically initializes
	// all variables with zero.
	keyboard_report.report_id = 1;
	//keyboard_report.modifier = 0;
	//keyboard_report.key = 0;
}  // }}}

void build_report_from_char(uchar c) {  // {{{
	// Using a local pointer saves around 6 bytes
	KeyboardReport *repptr = &keyboard_report;
	FIX_POINTER(repptr);

	// For most cases, modifier is zero
	repptr->modifier = 0;

	if (c >= ' ' && c <= '@') {
		repptr->modifier = pgm_read_byte_near(&char_to_key[c - ' '].modifier);
		repptr->key = pgm_read_byte_near(&char_to_key[c - ' '].key);
	} else if (c >= 'A' && c <= 'Z') {
		repptr->modifier = MOD_SHIFT_LEFT;
		repptr->key = KEY_A + c - 'A';
	} else if (c >= 'a' && c <= 'z') {
		//repptr->modifier = 0;
		repptr->key = KEY_A + c - 'a';
	} else {
		switch (c) {
			case '\n':
				//repptr->modifier = 0;
				repptr->key = KEY_ENTER;
				break;
			case '\t':
				//repptr->modifier = 0;
				repptr->key = KEY_TAB;
				break;
			case '_':
				repptr->modifier = MOD_SHIFT_LEFT;
				repptr->key = KEY_MINUS;
				break;

			default:
				//repptr->modifier = 0;
				repptr->key = 0;
		}
	}
}  // }}}


uchar send_next_char() {  // {{{
	// Builds a Report with the char pointed by 'string_output_pointer'.
	//
	// If a valid char is found, builds the report and returns 1.
	// If the pointer is NULL or the char is '\0', builds a "no key being
	// pressed" report and returns 0.
	// (note: the return value is being ignored by main())
	//
	// If the next char uses the same key as the previous one, then sends a
	// "no key" before sending the char.

	// Using a local pointer saves around 2 bytes
	KeyboardReport *repptr = &keyboard_report;
	FIX_POINTER(repptr);

	if (string_output_pointer != NULL && *string_output_pointer != '\0') {
		uchar old_key;

		old_key = repptr->key;
		build_report_from_char(*string_output_pointer);

		if (old_key == repptr->key && repptr->key != 0) {
			// Inserting a key release if the next key would be the same as
			// the previous one
			repptr->modifier = 0;
			repptr->key = 0;
		} else {
			string_output_pointer++;
		}

		return 1;
	} else {
		repptr->modifier = 0;
		repptr->key = 0;
		string_output_pointer = NULL;
		return 0;
	}
}  // }}}


////////////////////////////////////////////////////////////
// String utilities
// Not exactly part of keyboard emulation, but useless outside it.

uchar nibble_to_hex(uchar n) {  // {{{
	// I'm supposing n is already in range 0x00..0x0F
	if (n < 10)
		return '0' + n;
	else
		return 'A' + n - 10;
}  // }}}

void uchar_to_hex(uchar v, uchar *str) {  // {{{
	// NOTE: The NULL terminator is NOT added!
	*str = nibble_to_hex(v >> 4);
	str++;
	*str = nibble_to_hex(v & 0x0F);
}  // }}}

void int_to_hex(int v, uchar *str) {  // {{{
	// I'm supposing int is 16-bit
	// NOTE: The NULL terminator is NOT added!
	uchar_to_hex((uchar)(v >> 8), str);
	uchar_to_hex((uchar) v      , str+2);
}  // }}}

uchar* int_to_dec(int v, uchar *str) {  // {{{
	// Returns a pointer to the '\0' char

	itoa(v, (char*)str, 10);
	while (*str != '\0') {
		str++;
	}
	return str;
}  // }}}

uchar* append_newline_to_str(uchar *str) {  // {{{
	// Returns a pointer to the '\0' char

	while (*str != '\0') {
		str++;
	}
	*str     = '\n';
	*(str+1) = '\0';

	return str+1;
}  // }}}

uchar* array_to_hexdump(uchar *data, uchar len, uchar *str) {  // {{{
	// Builds a string of this form:
	// "DE AD F0 0D"
	// One space between each byte, ending the string with '\0'
	//
	// Returns a pointer to the '\0' char

	// I'm supposing that len > 0
	uchar_to_hex(*data, str);

	while (--len) {
		data++;
		// str[0] and str[1] are the hex digits
		str[2] = ' ';
		str += 3;
		uchar_to_hex(*data, str);
	}

	str[2] = '\0';

	return str+2;
}  // }}}

uchar* XYZVector_to_string(XYZVector* vector, uchar *str) {  // {{{
	// "-1234\t1234\t-1234\n"

	str = int_to_dec(vector->x, str);
	*str = '\t';
	str++;

	str = int_to_dec(vector->y, str);
	*str = '\t';
	str++;

	str = int_to_dec(vector->z, str);
	*str = '\n';
	str++;

	*str = '\0';

	return str;
}  // }}}


// vim:noexpandtab tabstop=4 shiftwidth=4 foldmethod=marker foldmarker={{{,}}}
