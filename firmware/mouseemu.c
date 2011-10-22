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

	mouse_report.buttons = new_state;
	return modified;
}  // }}}


static void mouse_axes_no_conversion() {  // {{{
	SensorData *sens = &sensor;
	FIX_POINTER(sens);

	mouse_report.x = sens->data.x * 8 + 16384;
	mouse_report.y = sens->data.y * 8 + 16384;
}  // }}}


static uchar mouse_update_axes() {  // {{{
	SensorData *sens = &sensor;
	FIX_POINTER(sens);

	if (sens->new_data_available && !sens->overflow) {
		mouse_axes_no_conversion();
		sens->new_data_available = 0;
		return 1;
	} else {
		// Clearing the x, y to invalid values.
		// Invalid values should be ignored by USB host.
		//mouse_report.x = -1;
		//mouse_report.y = -1;

		// Well... Actually, Linux 2.6.38 does not behave that way.
		// Instead, Linux moves the mouse pointer even if the supplied X,Y
		// values are outside the LOGICAL_MINIMUM..LOGICAL_MAXIMUM range.
		//
		// As a workaround:
		// If no data is available, I just leave the previous data in there.
		//
		// Note: Windows correctly ignores the invalid values.

		return 0;
	}
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
