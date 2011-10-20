/* Name: main.h
 *
 * USB-HID things defined at main.c
 */

#ifndef __main_h_included__
#define __main_h_included__


#define REPORT_BUFFER_SIZE 6
extern uchar report_buffer[REPORT_BUFFER_SIZE];


void clear_report_buffer();


#endif  // __main_h_included__

// vim:noexpandtab tabstop=4 shiftwidth=4 foldmethod=marker foldmarker={{{,}}}
