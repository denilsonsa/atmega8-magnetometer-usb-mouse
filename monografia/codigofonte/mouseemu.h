/* Name: mouseemu.h
 *
 * See the .c file for more information
 */

#ifndef __mouseemu_h_included__
#define __mouseemu_h_included__

#include "common.h"
#include "sensor.h"


typedef struct MouseReport {
	uchar report_id;
	int x; // 0..32767
	int y; // 0..32767
	uchar buttons;
} MouseReport;

extern MouseReport mouse_report;


void init_mouse_emulation();
uchar mouse_prepare_next_report();


#endif  // __mouseemu_h_included____

// vim:noexpandtab tabstop=4 shiftwidth=4 foldmethod=marker foldmarker={{{,}}}
