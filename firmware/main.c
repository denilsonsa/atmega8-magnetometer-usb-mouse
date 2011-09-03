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
 * TODO: add AVR315 TWI driver, and USBaspLoader
 */

// V-USB driver from http://www.obdev.at/products/vusb/
#include "usbdrv.h"

// Headers from AVR-Libc
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <avr/wdt.h>
#include <util/delay.h>
#include <stdlib.h>

// I'm not using serial-line debugging
//#include "oddebug.h"

// TODO: try including usbdrv.c directly, just to see how much space it saves.

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
 * PC3: Switch button
 * PC4: I2C - SDA
 * PC5: I2C - SCL
 *
 * PD0: USB-
 * PD1: (not used - debug tx)
 * PD2: USB+ (int0)
 * PD3: (not used)
 * PD4: (not used)
 * PD5: red debug LED
 * PD6: yellow debug LED
 * PD7: green debug LED
 */

#define LED_TURN_ON(led)  do { PORTD |=  (led); } while(0)
#define LED_TURN_OFF(led) do { PORTD &= ~(led); } while(0)
#define LED_TOGGLE(led)   do { PORTD ^=  (led); } while(0)

// Bit masks for each LED (in PORTD)
#define RED_LED    (1 << 5)
#define YELLOW_LED (1 << 6)
#define GREEN_LED  (1 << 7)
#define ALL_LEDS   (GREEN_LED | YELLOW_LED | RED_LED)

// Bit masks for each button (in PORTC)
// (also used in key_state and key_changed vars)
#define BUTTON_1      (1 << 0)
#define BUTTON_2      (1 << 1)
#define BUTTON_3      (1 << 2)
#define BUTTON_SWITCH (1 << 3)
#define ALL_BUTTONS   (BUTTON_1 | BUTTON_2 | BUTTON_3 | BUTTON_SWITCH)

// }}}


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
	DDRD = 0 | ALL_LEDS;

	// Doing a USB reset
	// This is done here because the device might have been reset by the
	// watchdog or some condition other than power-up.
	//
	// A reset is done by holding both D+ and D- low (setting the pins as
	// output with value zero) for longer than 10ms.
	//
	// See page 145 of usb_20.pdf
	// See also http://www.beyondlogic.org/usbnutshell/usb2.shtml

    DDRD |= USBMASK;    // Setting as output
	PORTD &= ~USBMASK;  // Setting as zero

	_delay_ms(15);  // Holding this state for at least 10ms

    DDRD &= ~USBMASK;   // Setting as input
	//PORTD &= ~USBMASK;  // Pull-ups are already disabled

	// End of USB reset
		

	// TODO: Do I need this timer?
    /* configure timer 0 for a rate of 12M/(1024 * 256) = 45.78 Hz (~22ms) */
    TCCR0 = 5;      /* timer 0 prescaler: 1024 */

	// I'm not using serial-line debugging
	//odDebugInit();

	LED_TOGGLE(YELLOW_LED);
}  // }}}

/* ------------------------------------------------------------------------- */


static uchar key_state = 0;
static uchar key_changed = 0;

// Handy macros!
// These have the same name/meaning of JavaScript events
#define ON_KEY_DOWN(button_mask) ((key_changed & (button_mask)) &&  (key_state & (button_mask)))
#define ON_KEY_UP(button_mask)   ((key_changed & (button_mask)) && !(key_state & (button_mask)))
// TODO: add some debouncing code, if needed.

static void update_key_state() {
	uchar state;

	// buttons are on PC0, PC1, PC2, PC3
	// buttons are ON when connected to GND, and thus read as zero
	// buttons are OFF when open, and thus the internal pull-up makes them read as one

	// The low nibble of PINC maps to the 4 buttons
	state = (~PINC) & 0x0F;
	key_changed = key_state ^ state;
	key_state = state;
}

/* ------------------------------------------------------------------------- */
/* ----------------------------- USB interface ----------------------------- */
/* ------------------------------------------------------------------------- */

static uchar    reportBuffer[2];    /* buffer for HID reports */
static uchar    idleRate;           /* in 4 ms units */

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

/* Keyboard usage values, see usb.org's HID-usage-tables document, chapter
 * 10 Keyboard/Keypad Page for more codes.
 */
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

// Last char sent
static uchar last_char = '\0';

static void build_report_from_char(uchar c) {
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
			case '\n':
				reportBuffer[0] = 0;
				reportBuffer[1] = KEY_ENTER;
				break;

			case '\t':
				reportBuffer[0] = 0;
				reportBuffer[1] = KEY_TAB;
				break;

			case ' ':
				reportBuffer[0] = 0;
				reportBuffer[1] = KEY_SPACE;
				break;

			case '-':
				reportBuffer[0] = 0;
				reportBuffer[1] = KEY_MINUS;
				break;
			case '_':
				reportBuffer[0] = MOD_SHIFT_LEFT;
				reportBuffer[1] = KEY_MINUS;
				break;

			case '=':
				reportBuffer[0] = 0;
				reportBuffer[1] = KEY_EQUAL;
				break;
			case '+':
				reportBuffer[0] = MOD_SHIFT_LEFT;
				reportBuffer[1] = KEY_EQUAL;
				break;

			case ';':
				reportBuffer[0] = 0;
				reportBuffer[1] = KEY_SEMICOLON;
				break;
			case ':':
				reportBuffer[0] = MOD_SHIFT_LEFT;
				reportBuffer[1] = KEY_SEMICOLON;
				break;

			case ',':
				reportBuffer[0] = 0;
				reportBuffer[1] = KEY_COMMA;
				break;
			case '<':
				reportBuffer[0] = MOD_SHIFT_LEFT;
				reportBuffer[1] = KEY_COMMA;
				break;

			case '.':
				reportBuffer[0] = 0;
				reportBuffer[1] = KEY_PERIOD;
				break;
			case '>':
				reportBuffer[0] = MOD_SHIFT_LEFT;
				reportBuffer[1] = KEY_PERIOD;
				break;

			default:
				reportBuffer[0] = 0;
				reportBuffer[1] = 0;
		}
	}
}


// Exclamation point is being ignored, though
static uchar hello_world[] = "Hello, world!\n";

// 2**31 has 10 decimal digits, plus 1 for signal, plus 1 for NULL terminator
static uchar number_buffer[12];


// Pointer to RAM... Yeah, for static strings that's a waste of RAM, but it's
// good enough for now.
static uchar *string_pointer = NULL;

static uchar send_next_char() {
	// Builds a Report with the char pointed by 'string_pointer'.
	//
	// If a valid char is found, builds the report and returns 1.
	// If the pointer is NULL or the char is '\0', builds a "no key being
	// pressed" report and returns 0.
	//
	// If the next char is equal to the last char, then sends a "no key" before
	// sending the char.

	if (string_pointer != NULL && *string_pointer != '\0') {
		if (*string_pointer == last_char) {
			build_report_from_char('\0');
		} else {
			build_report_from_char(*string_pointer);
			string_pointer++;
		}
		return 1;
	} else {
		build_report_from_char('\0');
		string_pointer = NULL;
		return 0;
	}
}


static uchar nibble_to_hex(uchar n) {
	// I'm supposing n is already in range 0x00..0x0F
	if (n < 10)
		return '0' + n;
	else
		return 'A' + n - 10;
}

static void uchar_to_hex(uchar v, uchar *str) {
	// XXX: The NULL terminator is NOT added!
	*str = nibble_to_hex(v >> 4);
	str++;
	*str = nibble_to_hex(v & 0x0F);
}

static void int_to_hex(int v, uchar *str) {
	// I'm supposing int is 16-bit
	// XXX: The NULL terminator is NOT added!
	uchar_to_hex((uchar)(v >> 8), str);
	uchar_to_hex((uchar) v      , str+2);
}

static void append_newline_to_str(uchar *str) {
	while(*str != '\0') {
		str++;
	}
	*str     = '\n';
	*(str+1) = '\0';
}

uchar usbFunctionSetup(uchar data[8]) {
	usbRequest_t *rq = (void *)data;

    usbMsgPtr = reportBuffer;
    if ((rq->bmRequestType & USBRQ_TYPE_MASK) == USBRQ_TYPE_CLASS) {    /* class request type */
        if (rq->bRequest == USBRQ_HID_GET_REPORT){  /* wValue: ReportType (highbyte), ReportID (lowbyte) */
            /* we only have one report type, so don't look at wValue */

			// XXX: Ainda não entendi quando isto é chamado...
			// TODO: Fazer ligar um LED quando isto acontecer.
			LED_TOGGLE(GREEN_LED);
			build_report_from_char('\0');

            //buildReport(keyPressed());
			// Achei que isto fosse necessário, mas não é
            // usbMsgPtr = reportBuffer;
            return sizeof(reportBuffer);
        } else if (rq->bRequest == USBRQ_HID_GET_IDLE) {
            usbMsgPtr = &idleRate;
            return 1;
        } else if (rq->bRequest == USBRQ_HID_SET_IDLE) {
			// Set/Get IDLE defines how long the device should keep "quiet" if
			// the state has not changed.
			// Recommended default value for keyboard is 500ms, and infinity
			// for joystick and mice.
			// See pages 52 and 53 from HID1_11.pdf
            idleRate = rq->wValue.bytes[1];
        }
    } else {
        /* no vendor specific requests implemented */
    }
	return 0;
}

/* ------------------------------------------------------------------------- */

int	main(void)
{
	int useless_counter = 0;
	uchar should_send_report = 1;

	uchar idleCounter = 0;

	cli();
    hardware_init();
	usbInit();
	sei();

	for(;;) {	/* main event loop */
		wdt_reset();
		usbPoll();
		update_key_state();

		if (ON_KEY_DOWN(BUTTON_1)) {
			LED_TOGGLE(RED_LED);
			if (!should_send_report) {
				// And this firmware is not sending anything
				string_pointer = hello_world;
				should_send_report = 1;
			}
		}
		if (ON_KEY_DOWN(BUTTON_2)) {
			useless_counter++;
		}
		if (ON_KEY_DOWN(BUTTON_3)) {
			useless_counter--;
		}
		if (ON_KEY_DOWN(BUTTON_2) || ON_KEY_DOWN(BUTTON_3)) {
			if (!should_send_report) {
				if (key_state & BUTTON_SWITCH) {
					int_to_hex(useless_counter, number_buffer);
					number_buffer[4] = '\n';
					number_buffer[5] = '\0';
				} else {
					itoa(useless_counter, (char*)number_buffer, 10);
					append_newline_to_str(number_buffer);
				}
				string_pointer = number_buffer;
				should_send_report = 1;
			}
		}

        if(TIFR & (1<<TOV0)){   /* 22 ms timer */
            TIFR = 1<<TOV0;
            if(idleRate != 0){
                if(idleCounter > 4){
                    idleCounter -= 5;   /* 22 ms in units of 4 ms */
                }else{
                    idleCounter = idleRate;
                    //keyDidChange = 1;
					LED_TOGGLE(YELLOW_LED);
					// This piece of code never runs... because this modified
					// firmware does not implement "idle"
                }
            }
        }
        if(should_send_report && usbInterruptIsReady()){
			should_send_report = send_next_char();
            usbSetInterrupt(reportBuffer, sizeof(reportBuffer));
        }
	}
	return 0;
}
