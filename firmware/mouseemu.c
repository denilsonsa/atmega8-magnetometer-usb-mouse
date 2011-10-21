/* Name: mouseemu.c
 * Project: atmega8-magnetometer-usb-mouse
 * Author: Denilson Figueiredo de Sa
 * Creation Date: 2011-10-18
 * Tabsize: 4
 * License: GNU GPL v2 or GNU GPL v3
 */


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


// vim:noexpandtab tabstop=4 shiftwidth=4 foldmethod=marker foldmarker={{{,}}}
