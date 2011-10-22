/* Name: mouseemu.c
 * Project: atmega8-magnetometer-usb-mouse
 * Author: Denilson Figueiredo de Sa
 * Creation Date: 2011-10-18
 * Tabsize: 4
 * License: GNU GPL v2 or GNU GPL v3
 */


#include "buttons.h"
#include "common.h"
#include "mouseemu.h"


// HID report
MouseReport mouse_report;


void init_mouse_emulation() {  // {{{
	// According to avr-libc FAQ, the compiler automatically initializes
	// all variables with zero.
	mouse_report.report_id = 2;
	mouse_report.x = -1;
	mouse_report.y = -1;
	//mouse_report.buttons = 0;
}  // }}}


static uchar mouse_update_buttons() {  // {{{
	// Return 1 if the button state has been updated (and thus the report
	// should be sent to the computer).

	// Since the 3 buttons are already at the 3 least significant bits, no
	// complicated conversion is need.
	// 0x07 = 0000 0111
	uchar new_state = button.state & 0x07;
	uchar modified = (new_state != mouse_report.buttons);

	// The code below is being ignored, for debugging purposes
	//mouse_report.buttons = new_state;
	//return modified;
	return 0;
}  // }}}


static void mouse_axes_no_conversion() {  // {{{
	SensorData *sens = &sensor;
	FIX_POINTER(sens);

	mouse_report.x = sens->data.x + 2048;
	mouse_report.y = sens->data.y + 2048;
}  // }}}


static uchar mouse_update_axes() {  // {{{
	SensorData *sens = &sensor;
	FIX_POINTER(sens);

	if (ON_KEY_DOWN(BUTTON_1)) {
		mouse_report.x += 4096;
		mouse_report.y += 4096;
		return 1;
	} else if (ON_KEY_DOWN(BUTTON_2)) {
		mouse_report.x -= 4096;
		mouse_report.y -= 4096;
		return 1;
	} else if (ON_KEY_DOWN(BUTTON_3)) {
		mouse_report.x = 0;
		mouse_report.y = 0;
		return 1;
	}

	return 0;

	// The code below is being ignored, for debugging purposes
	/*
	if (sens->new_data_available && !sens->overflow) {
		mouse_axes_no_conversion();
		sens->new_data_available = 0;
		return 1;
	} else {
		// Clearing the x, y to invalid values.
		// Invalid values should be ignored by USB host.
		//mouse_report.x = -1;
		//mouse_report.y = -1;

		return 0;
	}
	*/

// Peculiar (or buggy) behavior under Linux...
//
// For debugging:
// cat /sys/kernel/debug/usb/usbmon/5u | awk --posix '/ [0-9a-f]{4}$/ {if(old != $9) {print ; old=$9} }'
// ffff88004b49fe40 3155151289 C Ii:5:086:1 0:8 6 = 02620779 0800
// ffff88004b49fe40 3158927311 C Ii:5:086:1 0:8 6 = 02ffffff ff04
// ffff88004b49fe40 3158935300 C Ii:5:086:1 0:8 6 = 02610780 0804
// ffff88004b49fe40 3161743331 C Ii:5:086:1 0:8 6 = 02ffffff ff00
// ffff88004b49fe40 3161751326 C Ii:5:086:1 0:8 6 = 02aa07c2 0800
//
// Look here --------------------------------------------------^^
//
// The last byte is the "buttons" (either 00 for no button or 04 for
// middle-button). The preceding 4 bytes are the X,Y position (or ffff meaning
// "null value").
//
// So, should be the behavior if the system receives a mouse click at an
// invalid position?
//
// Linux appears to "move" the pointer to the "ffff,ffff" position (that is,
// the bottom-right corner of the screen, which is 1279,799 for my laptop) for
// one moment, and registers the mouse click there. This is wrong, in my
// opinion, as this value is clearly out of the logical minimum and logical
// maximum described in the HID Descriptor, and thus should be ignored.
//
// Windows appears to do the right thing, ignoring the invalid X,Y and
// registering the click wherever the pointer currently is.

}  // }}}


uchar mouse_prepare_next_report() {  // {{{
	// Return 1 if a new report is available and should be sent to the
	// computer.

	// I'm using a bitwise OR here because a boolean OR would short-circuit
	// the expression and wouldn't run the second function. It's ugly, but
	// it's simple and works.
	return mouse_update_buttons() | mouse_update_axes();
}  // }}}


// vim:noexpandtab tabstop=4 shiftwidth=4 foldmethod=marker foldmarker={{{,}}}
