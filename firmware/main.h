/* Name: main.h
 *
 * USB-HID things defined at main.c
 */

#ifndef __main_h_included__
#define __main_h_included__


typedef struct KeyboardReport {
	//uchar report_id;
	uchar modifier;
	uchar key;
} KeyboardReport;

typedef struct MouseReport {
	//uchar report_id;
	int x;
	int y;
	uchar buttons;
	// TODO: update the report descriptor to this struct
} MouseReport;

typedef struct HIDReport {
	// TODO: update the code at main.c and keyemu.c to this struct
	uchar report_id;
	union {
		KeyboardReport keyboard;
		MouseReport mouse;
	};
} HIDReport;


#define REPORT_BUFFER_SIZE 6
extern uchar report_buffer[REPORT_BUFFER_SIZE];


void clear_report_buffer();


#endif  // __main_h_included__

// vim:noexpandtab tabstop=4 shiftwidth=4 foldmethod=marker foldmarker={{{,}}}
