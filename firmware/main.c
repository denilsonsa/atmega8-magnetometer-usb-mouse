/* Name: main.c
 * Project: atmega8-magnetometer-usb-mouse
 * Author: Denilson Figueiredo de Sa
 * Creation Date: 2011-08-29
 * Tabsize: 4
 * License: GNU GPL v2 or GNU GPL v3
 *
 * Includes third-party code:
 * - V-USB from OBJECTIVE DEVELOPMENT Software GmbH
 *   http://www.obdev.at/products/vusb/index.html
 * - USBaspLoader from OBJECTIVE DEVELOPMENT Software GmbH
 *   http://www.obdev.at/products/vusb/usbasploader.html
 * - AVR315 TWI Master Implementation from Atmel
 *   http://www.atmel.com/dyn/products/documents.asp?category_id=163&family_id=607&subfamily_id=760
 */

// Headers from AVR-Libc
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <avr/wdt.h>
#include <util/delay.h>
#include <stdlib.h>

// V-USB driver from http://www.obdev.at/products/vusb/
#include "usbdrv.h"

// It's also possible to include "usbdrv.c" directly, if we also add
// this definition at the top of this file:
// #define USB_PUBLIC static
// However, this only saved 10 bytes.

// I'm not using serial-line debugging
//#include "oddebug.h"

// AVR315 Using the TWI module as I2C master
#include "avr315/TWI_Master.h"


////////////////////////////////////////////////////////////
// Hardware description                                  {{{

/* ATmega8 pin assignments:
 * PB0: (not used)
 * PB1: (not used)
 * PB2: (not used)
 * PB3: (not used - MOSI)
 * PB4: (not used - MISO)
 * PB5: (not used - SCK)
 * PB6: 12MHz crystal
 * PB7: 12MHz crystal
 *
 * PC0: Button 1
 * PC1: Button 2
 * PC2: Button 3
 * PC3: Switch button (if held to GND during power-on, starts the bootloader)
 * PC4: I2C - SDA
 * PC5: I2C - SCL
 * PC6: Reset pin (with an external 10K pull-up to VCC)
 *
 * PD0: USB-
 * PD1: (not used - debug tx)
 * PD2: USB+ (int0)
 * PD3: (not used)
 * PD4: (not used)
 * PD5: red debug LED
 * PD6: yellow debug LED
 * PD7: green debug LED
 *
 * If you change the ports, remember to update these functions:
 * - hardware_init()
 * - update_key_state()
 */

#define LED_TURN_ON(led)  do { PORTD |=  (led); } while(0)
#define LED_TURN_OFF(led) do { PORTD &= ~(led); } while(0)
#define LED_TOGGLE(led)   do { PORTD ^=  (led); } while(0)

// Bit masks for each LED (in PORTD)
#define RED_LED    (1 << 5)
#define YELLOW_LED (1 << 6)
#define GREEN_LED  (1 << 7)
#define ALL_LEDS   (GREEN_LED | YELLOW_LED | RED_LED)

// Bit masks for each button (in PORTC and PINC)
// (also used in key_state and key_changed vars)
#define BUTTON_1      (1 << 0)
#define BUTTON_2      (1 << 1)
#define BUTTON_3      (1 << 2)
#define BUTTON_SWITCH (1 << 3)
#define ALL_BUTTONS   (BUTTON_1 | BUTTON_2 | BUTTON_3 | BUTTON_SWITCH)

// }}}

////////////////////////////////////////////////////////////
// Button handling code                                  {{{

// "Public" vars, already filtered for debouncing
static uchar key_state = 0;
static uchar key_changed = 0;

// "Private" button debouncing state
static uchar button_debouncing[4];  // We have 4 buttons/switches

// Handy macros!
// These have the same name/meaning of JavaScript events
#define ON_KEY_DOWN(button_mask) ((key_changed & (button_mask)) &&  (key_state & (button_mask)))
#define ON_KEY_UP(button_mask)   ((key_changed & (button_mask)) && !(key_state & (button_mask)))

static void init_key_state() {  // {{{
	key_state = 0;
	key_changed = 0;
	button_debouncing[0] = 0;
	button_debouncing[1] = 0;
	button_debouncing[2] = 0;
	button_debouncing[3] = 0;
}  // }}}

static void update_key_state() { // {{{
	// This function implements debouncing code.
	// It should be called at every iteration of the main loop.
	// It reads the TOV0 flag status, but does not clear it.

	uchar filtered_state;

	filtered_state = key_state;

	// Timer is set to 1.365ms
	if (TIFR & (1<<TOV0)) {
		uchar raw_state;
		uchar i;

		// Buttons are on PC0, PC1, PC2, PC3
		// Buttons are ON when connected to GND, and read as zero
		// Buttons are OFF when open, internal pull-ups make them read as one

		// The low nibble of PINC maps to the 4 buttons
		raw_state = (~PINC) & 0x0F;
		// "raw_state" has the button state, with 1 for pressed and 0 for released.
		// Still needs debouncing...

		// This debouncing solution is inspired by tiltstick-20080207 firmware.
		//
		// Poll the buttons into a shift register for de-bouncing.
		// A button is considered pressed when the register reaches 0xFF.
		// A button is considered released when the register reaches 0x00.
		// Any intermediate value does not change the button state.
		//
		// 8 * 1.365ms = ~11ms without interruption
		for (i=0; i<4; i++) {
			button_debouncing[i] =
				(button_debouncing[i]<<1)
				| ( ((raw_state & (1<<i)))? 1 : 0 );

			if (button_debouncing[i] == 0) {
				// Releasing this button
				filtered_state &= ~(1<<i);
			} else if (button_debouncing[i] == 0xFF) {
				// Pressing this button
				filtered_state |=  (1<<i);
			}
		}
	}

	// Storing the final, filtered, updated state
	key_changed = key_state ^ filtered_state;
	key_state = filtered_state;
}  // }}}

// }}}

////////////////////////////////////////////////////////////
// USB HID Report Descriptor                             {{{

static uchar    reportBuffer[2];    /* buffer for HID reports */

// XXX: If this HID report descriptor is changed, remember to update
//      USB_CFG_HID_REPORT_DESCRIPTOR_LENGTH from usbconfig.h
PROGMEM char usbHidReportDescriptor[USB_CFG_HID_REPORT_DESCRIPTOR_LENGTH] = {
    0x05, 0x01,                    // USAGE_PAGE (Generic Desktop)
    0x09, 0x06,                    // USAGE (Keyboard)
    0xa1, 0x01,                    // COLLECTION (Application)
    0x05, 0x07,                    //   USAGE_PAGE (Keyboard)
    0x19, 0xe0,                    //   USAGE_MINIMUM (Keyboard LeftControl)
    0x29, 0xe7,                    //   USAGE_MAXIMUM (Keyboard Right GUI)
    0x15, 0x00,                    //   LOGICAL_MINIMUM (0)
    0x25, 0x01,                    //   LOGICAL_MAXIMUM (1)
    0x75, 0x01,                    //   REPORT_SIZE (1)
    0x95, 0x08,                    //   REPORT_COUNT (8)
    0x81, 0x02,                    //   INPUT (Data,Var,Abs)
    0x95, 0x01,                    //   REPORT_COUNT (1)
    0x75, 0x08,                    //   REPORT_SIZE (8)
    0x25, 0x65,                    //   LOGICAL_MAXIMUM (101)
    0x19, 0x00,                    //   USAGE_MINIMUM (Reserved (no event indicated))
    0x29, 0x65,                    //   USAGE_MAXIMUM (Keyboard Application)
    0x81, 0x00,                    //   INPUT (Data,Ary,Abs)
    0xc0                           // END_COLLECTION
};
/* We use a simplifed keyboard report descriptor which does not support the
 * boot protocol. We don't allow setting status LEDs and we only allow one
 * simultaneous key press (except modifiers). We can therefore use short
 * 2 byte input reports.
 * The report descriptor has been created with usb.org's "HID Descriptor Tool"
 * which can be downloaded from http://www.usb.org/developers/hidpage/.
 * Redundant entries (such as LOGICAL_MINIMUM and USAGE_PAGE) have been omitted
 * for the second INPUT item.
 */

// }}}

////////////////////////////////////////////////////////////
// Keyboard emulation code                               {{{

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

// Last char sent
static uchar last_char = '\0';

// Pointer to RAM for the string being typed.
static uchar *string_output_pointer = NULL;

// Shared output buffer, other functions are free to use this as needed.
static uchar string_output_buffer[80];

// Copies a string from PGM to string_output_buffer and also sets
// string_output_pointer.
#define output_pgm_string(str) do { \
		strcpy_P(string_output_buffer, str); \
		string_output_pointer = string_output_buffer; \
	} while(0);

static void build_report_from_char(uchar c) {  // {{{
	last_char = c;

	if (c >= '1' && c <= '9') {
		reportBuffer[0] = 0;
		reportBuffer[1] = KEY_1 + c - '1';
	}
	else if (c == '0') {
		reportBuffer[0] = 0;
		reportBuffer[1] = KEY_0;
	}
	else if (c >= 'a' && c <= 'z') {
		reportBuffer[0] = 0;
		reportBuffer[1] = KEY_A + c - 'a';
	}
	else if (c >= 'A' && c <= 'Z') {
		reportBuffer[0] = MOD_SHIFT_LEFT;
		reportBuffer[1] = KEY_A + c - 'A';
	}
	else {
		switch (c) {
			case '\n': reportBuffer[1] = KEY_ENTER;     break;

			case '\t': reportBuffer[1] = KEY_TAB;       break;

			case ' ':  reportBuffer[1] = KEY_SPACE;     break;

			case '_':
			case '-':  reportBuffer[1] = KEY_MINUS;     break;

			case '+':
			case '=':  reportBuffer[1] = KEY_EQUAL;     break;

			case ':':
			case ';':  reportBuffer[1] = KEY_SEMICOLON; break;

			case '<':
			case ',':  reportBuffer[1] = KEY_COMMA;     break;

			case '>':
			case '.':  reportBuffer[1] = KEY_PERIOD;    break;

			default:
				reportBuffer[1] = 0;
		}

		switch (c) {
			case '_':
			case '+':
			case ':':
			case '<':
			case '>':
				reportBuffer[0] = MOD_SHIFT_LEFT;
				break;
			default:
				reportBuffer[0] = 0;
		}
	}
}  // }}}

static uchar send_next_char() {  // {{{
	// Builds a Report with the char pointed by 'string_output_pointer'.
	//
	// If a valid char is found, builds the report and returns 1.
	// If the pointer is NULL or the char is '\0', builds a "no key being
	// pressed" report and returns 0.
	//
	// If the next char is equal to the last char, then sends a "no key" before
	// sending the char.

	if (string_output_pointer != NULL && *string_output_pointer != '\0') {
		if (*string_output_pointer == last_char) {
			build_report_from_char('\0');
		} else {
			build_report_from_char(*string_output_pointer);
			string_output_pointer++;
		}
		return 1;
	} else {
		build_report_from_char('\0');
		string_output_pointer = NULL;
		return 0;
	}
}  // }}}

static void init_keyboard_emulation() {  // {{{
	last_char = '\0';
	string_output_pointer = NULL;
}  // }}}

// }}}

////////////////////////////////////////////////////////////
// String utilities                                      {{{

static uchar nibble_to_hex(uchar n) {  // {{{
	// I'm supposing n is already in range 0x00..0x0F
	if (n < 10)
		return '0' + n;
	else
		return 'A' + n - 10;
}  // }}}

static void uchar_to_hex(uchar v, uchar *str) {  // {{{
	// XXX: The NULL terminator is NOT added!
	*str = nibble_to_hex(v >> 4);
	str++;
	*str = nibble_to_hex(v & 0x0F);
}  // }}}

static void int_to_hex(int v, uchar *str) {  // {{{
	// I'm supposing int is 16-bit
	// XXX: The NULL terminator is NOT added!
	uchar_to_hex((uchar)(v >> 8), str);
	uchar_to_hex((uchar) v      , str+2);
}  // }}}

static uchar* int_to_dec(int v, uchar *str) {  // {{{
	// Returns a pointer to the '\0' char

	itoa(v, (char*)str, 10);
	while (*str != '\0') {
		str++;
	}
	return str;
}  // }}}

static uchar* append_newline_to_str(uchar *str) {  // {{{
	// Returns a pointer to the '\0' char

	while (*str != '\0') {
		str++;
	}
	*str     = '\n';
	*(str+1) = '\0';

	return str+1;
}  // }}}

static uchar* array_to_hexdump(uchar *data, uchar len, uchar *str) {  // {{{
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

// }}}

////////////////////////////////////////////////////////////
// Sensor communication over I2C (TWI)                   {{{
// This sensor has "L883 2105" written on the chip.
// It is known as HMC5883L or HMC5883.

#define SENSOR_I2C_READ_ADDRESS  0x3D
#define SENSOR_I2C_WRITE_ADDRESS 0x3C

// HMC5883L register definitions  {{{
// See page 11 of HMC5883L.pdf
// Read/write registers:
#define SENSOR_REG_CONF_A       0
#define SENSOR_REG_CONF_B       1
#define SENSOR_REG_MODE         2
// Read-only registers:
#define SENSOR_REG_DATA_START   3
#define SENSOR_REG_DATA_X_MSB   3
#define SENSOR_REG_DATA_X_LSB   4
#define SENSOR_REG_DATA_Z_MSB   5
#define SENSOR_REG_DATA_Z_LSB   6
#define SENSOR_REG_DATA_Y_MSB   7
#define SENSOR_REG_DATA_Y_LSB   8
#define SENSOR_REG_STATUS       9
#define SENSOR_REG_ID_A        10
#define SENSOR_REG_ID_B        11
#define SENSOR_REG_ID_C        12

// }}}

// HMC5883L status definitions  {{{
// See page 16 of HMC5883L.pdf

#define SENSOR_STATUS_LOCK  2
#define SENSOR_STATUS_RDY   1
// }}}

// HMC5883L configuration definitions  {{{
// See pages 12, 13, 14 of HMC5883L.pdf

// The Mode Register has 2 possible values for Idle mode.
#define SENSOR_MODE_CONTINUOUS  0
#define SENSOR_MODE_SINGLE      1
#define SENSOR_MODE_IDLE_A      2
#define SENSOR_MODE_IDLE_B      3
#define SENSOR_MODE_MASK        3

// How many samples averaged? Default=1
#define SENSOR_CONF_A_SAMPLES_1    0x00
#define SENSOR_CONF_A_SAMPLES_2    0x20
#define SENSOR_CONF_A_SAMPLES_4    0x40
#define SENSOR_CONF_A_SAMPLES_8    0x60
#define SENSOR_CONF_A_SAMPLES_MASK 0x60

// Data output rate for continuous mode. Default=15Hz
#define SENSOR_CONF_A_RATE_0_75     0x00
#define SENSOR_CONF_A_RATE_1_5      0x04
#define SENSOR_CONF_A_RATE_3        0x08
#define SENSOR_CONF_A_RATE_7_5      0x0C
#define SENSOR_CONF_A_RATE_15       0x10
#define SENSOR_CONF_A_RATE_30       0x14
#define SENSOR_CONF_A_RATE_75       0x18
#define SENSOR_CONF_A_RATE_RESERVED 0x1C
#define SENSOR_CONF_A_RATE_MASK     0x1C

// Measurement configuration, whether to apply bias. Default=Normal
#define SENSOR_CONF_A_BIAS_NORMAL   0x00
#define SENSOR_CONF_A_BIAS_POSITIVE 0x01
#define SENSOR_CONF_A_BIAS_NEGATIVE 0x02
#define SENSOR_CONF_A_BIAS_RESERVED 0x03
#define SENSOR_CONF_A_BIAS_MASK     0x03

// Gain configuration. Default=1.3Ga
#define SENSOR_CONF_B_GAIN_0_88 0x00
#define SENSOR_CONF_B_GAIN_1_3  0x20
#define SENSOR_CONF_B_GAIN_1_9  0x40
#define SENSOR_CONF_B_GAIN_2_5  0x60
#define SENSOR_CONF_B_GAIN_4_0  0x80
#define SENSOR_CONF_B_GAIN_4_7  0xA0
#define SENSOR_CONF_B_GAIN_5_6  0xC0
#define SENSOR_CONF_B_GAIN_8_1  0xE0
#define SENSOR_CONF_B_GAIN_MASK 0xE0

// Digital resolution (mG/LSb) for each gain
#define SENSOR_GAIN_SCALE_0_88  0.73
#define SENSOR_GAIN_SCALE_1_3   0.92
#define SENSOR_GAIN_SCALE_1_9   1.22
#define SENSOR_GAIN_SCALE_2_5   1.52
#define SENSOR_GAIN_SCALE_4_0   2.27
#define SENSOR_GAIN_SCALE_4_7   2.56
#define SENSOR_GAIN_SCALE_5_6   3.03
#define SENSOR_GAIN_SCALE_8_1   4.35

// }}}

// The X,Y,Z data from the sensor
int sensor_X;
int sensor_Y;
int sensor_Z;

// Boolean that detects if the sensor have reported an overflow
uchar sensor_overflow;
#define SENSOR_DATA_OVERFLOW -4096

static void sensor_set_address_pointer(uchar reg) {  // {{{
	// Sets the sensor internal register pointer.
	// This is required before reading registers.

	uchar msg[2];
	msg[0] = SENSOR_I2C_WRITE_ADDRESS;
	msg[1] = reg;
	TWI_Start_Transceiver_With_Data(msg, 2);
}  // }}}

static void sensor_set_register_value(uchar reg, uchar value) {  // {{{
	// Sets one of those 3 writable registers to a value.
	// Only useful for configuration.

	uchar msg[3];
	msg[0] = SENSOR_I2C_WRITE_ADDRESS;
	msg[1] = reg;
	msg[2] = value;
	TWI_Start_Transceiver_With_Data(msg, 3);
}  // }}}

static uchar sensor_read_status_register() {  // {{{
	// Returns the value of the STATUS register.
	// No error handling is done in this code.  Please test
	// "TWI_statusReg.lastTransOK" in order to detect errors.

	uchar msg[2];

	sensor_set_address_pointer(SENSOR_REG_STATUS);

	msg[0] = SENSOR_I2C_READ_ADDRESS;
	TWI_Start_Transceiver_With_Data(msg, 2);
	TWI_Get_Data_From_Transceiver(msg, 2);

	return msg[1];
}  // }}}

static uchar sensor_read_data_registers() {  // {{{
	// Reads the X,Y,Z data registers and store them at global vars.
	// In case of a transmission error, the previous values are not changed.
	//
	// Returns the same as "TWI_statusReg.lastTransOK".

	// 1 address byte + 6 data bytes = 7 bytes
	uchar msg[7];

	uchar lastTransOK;

	sensor_set_address_pointer(SENSOR_REG_DATA_START);

	msg[0] = SENSOR_I2C_READ_ADDRESS;
	TWI_Start_Transceiver_With_Data(msg, 7);
	lastTransOK = TWI_Get_Data_From_Transceiver(msg, 7);

	if (lastTransOK) {
		#define OFFSET (1 - SENSOR_REG_DATA_START)
		sensor_X = (msg[OFFSET+SENSOR_REG_DATA_X_MSB] << 8) | (msg[OFFSET+SENSOR_REG_DATA_X_LSB]);
		sensor_Y = (msg[OFFSET+SENSOR_REG_DATA_Y_MSB] << 8) | (msg[OFFSET+SENSOR_REG_DATA_Y_LSB]);
		sensor_Z = (msg[OFFSET+SENSOR_REG_DATA_Z_MSB] << 8) | (msg[OFFSET+SENSOR_REG_DATA_Z_LSB]);
		#undef OFFSET

		sensor_overflow =
			(sensor_X == SENSOR_DATA_OVERFLOW)
			|| (sensor_Y == SENSOR_DATA_OVERFLOW)
			|| (sensor_Z == SENSOR_DATA_OVERFLOW);
	}

	return lastTransOK;
}  // }}}

static uchar sensor_read_identification_string(uchar *s) {  // {{{
	// Reads the 3 identification registers from the sensor.
	// They should read as ASCII "H43".
	//
	// Receives a pointer to a string with at least 4 chars of size.
	// After reading the registers, stores them at *s, followed by '\0'.
	// In case of a transmission error, the passed string is not touched.
	//
	// Returns the same as "TWI_statusReg.lastTransOK".

	// 1 address byte + 3 chars
	uchar msg[4];

	uchar lastTransOK;

	sensor_set_address_pointer(SENSOR_REG_ID_A);
	msg[0] = SENSOR_I2C_READ_ADDRESS;
	TWI_Start_Transceiver_With_Data(msg, 4);
	lastTransOK = TWI_Get_Data_From_Transceiver(msg, 4);

	if (lastTransOK) {
		s[0] = msg[1];
		s[1] = msg[2];
		s[2] = msg[3];
		s[3] = '\0';
	}

	return lastTransOK;
}  // }}}

static void init_sensor_configuration() {  // {{{
	sensor_set_register_value(
		SENSOR_REG_CONF_A,
		SENSOR_CONF_A_SAMPLES_8
		| SENSOR_CONF_A_RATE_75
		| SENSOR_CONF_A_BIAS_NORMAL
	);
	sensor_set_register_value(
		SENSOR_REG_CONF_B,
		SENSOR_CONF_B_GAIN_1_3
	);
	sensor_set_register_value(
		SENSOR_REG_MODE,
		SENSOR_MODE_CONTINUOUS
	);
}  // }}}

// }}}

////////////////////////////////////////////////////////////
// Menu user interface for configuring the device        {{{

#include "menu.inc.c"

// }}}

////////////////////////////////////////////////////////////
// Main code                                             {{{

static const char hello_world[] PROGMEM = "Hello, YouTube.\n";

static const char twi_error_string[] PROGMEM = "TWI_statusReg.lastTransOK was FALSE.\n";

// As defined in section 7.2.4 Set_Idle Request
// of Device Class Definition for Human Interface Devices (HID) version 1.11
// pages 52 and 53 (or 62 and 63) of HID1_11.pdf
//
// Set/Get IDLE defines how long the device should keep "quiet" if the state
// has not changed.
// Recommended default value for keyboard is 500ms, and infinity for joystick
// and mice.
//
// This value is measured in multiples of 4ms.
// A value of zero means indefinite/infinity.
static uchar idle_rate;


static uchar* debug_print_X_Y_Z_to_string_buffer() {  // {{{
	// "-1234\t1234\t-1234\n"

	uchar *str = string_output_buffer;

	str = int_to_dec(sensor_X, str);
	*str = '\t';
	str++;

	str = int_to_dec(sensor_Y, str);
	*str = '\t';
	str++;

	str = int_to_dec(sensor_Z, str);
	*str = '\n';
	str++;

	*str = '\0';

	return str;
}  // }}}


static void hardware_init(void) {  // {{{
	// Configuring Watchdog to about 2 seconds
	// See pages 43 and 44 from ATmega8 datasheet
	// See also http://www.nongnu.org/avr-libc/user-manual/group__avr__watchdog.html
	wdt_enable(WDTO_2S);

	PORTB = 0xff;  // activate all pull-ups
	DDRB = 0;      // all pins input
	PORTC = 0xff;  // activate all pull-ups
	DDRC = 0;      // all pins input

	// From usbdrv.h:
	//#define USBMASK ((1<<USB_CFG_DPLUS_BIT) | (1<<USB_CFG_DMINUS_BIT))

	// activate pull-ups, except on USB lines and LED pins
	PORTD = 0xFF ^ (USBMASK | ALL_LEDS);
	// LED pins as output, the other pins as input
	DDRD = 0 | ALL_LEDS;

	// Doing a USB reset
	// This is done here because the device might have been reset
	// by the watchdog or some condition other than power-up.
	//
	// A reset is done by holding both D+ and D- low (setting the
	// pins as output with value zero) for longer than 10ms.
	//
	// See page 145 of usb_20.pdf
	// See also http://www.beyondlogic.org/usbnutshell/usb2.shtml

	DDRD |= USBMASK;    // Setting as output
	PORTD &= ~USBMASK;  // Setting as zero

	_delay_ms(15);  // Holding this state for at least 10ms

	DDRD &= ~USBMASK;   // Setting as input
	//PORTD &= ~USBMASK;  // Pull-ups are already disabled

	// End of USB reset


	// Disabling Timer0 Interrupt
	// It's disabled by default, anyway, so this shouldn't be needed
	TIMSK &= ~(TOIE0);

	// Configuring Timer0 (with main clock at 12MHz)
	// 0 = No clock (timer stopped)
	// 1 = Prescaler = 1     =>   0.0213333ms
	// 2 = Prescaler = 8     =>   0.1706666ms
	// 3 = Prescaler = 64    =>   1.3653333ms
	// 4 = Prescaler = 256   =>   5.4613333ms
	// 5 = Prescaler = 1024  =>  21.8453333ms
	// 6 = External clock source on T0 pin (falling edge)
	// 7 = External clock source on T0 pin (rising edge)
	//
	// See page 72 from ATmega8 datasheet.
	// Also thanks to http://frank.circleofcurrent.com/cache/avrtimercalc.htm
	TCCR0 = 3;

	// I'm using Timer0 as a 1.365ms ticker. Every time it overflows, the TOV0
	// flag in TIFR is set. The main loop clears this flag near the end of each
	// iteration.

	// I'm not using serial-line debugging
	//odDebugInit();

	LED_TURN_ON(YELLOW_LED);
}  // }}}


uchar usbFunctionSetup(uchar data[8]) {  // {{{
	usbRequest_t *rq = (void *)data;

	usbMsgPtr = reportBuffer;
	if ((rq->bmRequestType & USBRQ_TYPE_MASK) == USBRQ_TYPE_CLASS) {    /* class request type */
		if (rq->bRequest == USBRQ_HID_GET_REPORT){  /* wValue: ReportType (highbyte), ReportID (lowbyte) */
			/* we only have one report type, so don't look at wValue */

			// XXX: Ainda não entendi quando isto é chamado...
			LED_TOGGLE(GREEN_LED);
			build_report_from_char('\0');

			usbMsgPtr = reportBuffer;
			return sizeof(reportBuffer);
		} else if (rq->bRequest == USBRQ_HID_GET_IDLE) {
			usbMsgPtr = &idle_rate;
			return 1;
		} else if (rq->bRequest == USBRQ_HID_SET_IDLE) {
			idle_rate = rq->wValue.bytes[1];
		}
	} else {
		/* no vendor specific requests implemented */
	}
	return 0;
}  // }}}


int	main(void) {  // {{{
	uchar should_send_report = 1;
	int idle_counter = 0;

	uchar sensor_probe_counter = 0;
	uchar new_data_received = 0;

	cli();

	hardware_init();
	TWI_Master_Initialise();
	usbInit();

	wdt_reset();
	sei();

	init_key_state();
	init_keyboard_emulation();
	// Sensor initialization must be done with interrupts enabled!
	init_sensor_configuration();

	LED_TURN_ON(GREEN_LED);

	for (;;) {	// main event loop
		wdt_reset();
		usbPoll();

		update_key_state();

		// Red LED lights up if there is any kind of error in I2C communication
		if ( TWI_statusReg.lastTransOK ) {
			LED_TURN_OFF(RED_LED);
		} else {
			LED_TURN_ON(RED_LED);
		}

		// The main switch can be used to "freeze" the sensor values.
		new_data_received = 0;
		if (key_state & BUTTON_SWITCH) {
			// Timer is set to 1.365ms
			if (TIFR & (1<<TOV0)) {
				if (sensor_probe_counter > 0){
					sensor_probe_counter--;
				} else {
					uchar lastTransOK;

					// The sensor is configured for 75Hz measurements.
					// I'm using this timer to read the values at twice that rate.
					// 5 * 1.365ms = 6.827ms ~= 146Hz
					sensor_probe_counter = 5;

					lastTransOK = sensor_read_data_registers();
					if (lastTransOK) {
						new_data_received = 1;
					}
				}
			}

			if (key_state & BUTTON_3) {
				if (!should_send_report) {
					debug_print_X_Y_Z_to_string_buffer();
					string_output_pointer = string_output_buffer;
				}
			}
		}


		if (ON_KEY_UP(BUTTON_SWITCH)) {
			init_menu_ui();
		}

		if (!(key_state & BUTTON_SWITCH)) {
			body_menu_ui();
		}

		/*
		if (ON_KEY_DOWN(BUTTON_1)) {
			if (key_state & BUTTON_SWITCH) {
				if (!should_send_report) {
					// And the firmware is not sending anything

					// Printing "Hello, world"
					output_pgm_string(hello_world);
				}
			} else {
				if (!should_send_report) {
					// And the firmware is not sending anything

					// Printing X,Y,Z

					uchar lastTransOK;

					lastTransOK = sensor_read_data_registers();

					if (lastTransOK) {
						debug_print_X_Y_Z_to_string_buffer();
						string_output_pointer = string_output_buffer;
					} else {
						output_pgm_string(twi_error_string);
					}
				}
			}
		}
		*/


		// Timer is set to 1.365ms
		if (TIFR & (1<<TOV0)) {
			// Implementing the idle rate...
			if (idle_rate != 0) {
				if (idle_counter > 0){
					idle_counter--;
				} else {
					// idle_counter counts how many Timer0 overflows are
					// required before sending another report.
					// The exact formula is:
					// idle_counter = (idle_rate * 4)/1.365;
					// But it's better to avoid floating point math.
					// 4/1.365 = 2.93, so let's just multiply it by 3.
					idle_counter = idle_rate * 3;

					//keyDidChange = 1;
					LED_TOGGLE(YELLOW_LED);
					// TODO: implement this... should re-send the current status
					// Either that, or the idle_rate support should be removed.
				}
			}
		}

		// Resetting the Timer0
		if (TIFR & (1<<TOV0)) {
			// Setting this bit to one will clear it.
			// Yeah, weird, but that's how it works.
			TIFR = 1<<TOV0;
		}

		// Automatically send report if there is something in the buffer.
		if (string_output_pointer != NULL) {
			// TODO: refactor this "should_send_report" var. Currently, its
			// meaning is the same as comparing string_output_buffer to NULL.
			// So, it's redundant.
			should_send_report = 1;
		}

		// Sending USB Interrupt-in report
		if(should_send_report && usbInterruptIsReady()){
			// TODO: no need to return a value here...
			should_send_report = send_next_char();
			usbSetInterrupt(reportBuffer, sizeof(reportBuffer));
		}
	}
	return 0;
}  // }}}

// }}}

// vim:noexpandtab tabstop=4 shiftwidth=4 foldmethod=marker foldmarker={{{,}}}
