/* Name: keyemu.h
 *
 * See the .c file for more information
 */

#ifndef __keyemu_h_included__
#define __keyemu_h_included__

#include "common.h"
#include "sensor.h"


typedef struct KeyboardReport {
	uchar report_id;
	uchar modifier;
	uchar key;
} KeyboardReport;


extern KeyboardReport keyboard_report;


// Copies a string from PGM to string_output_buffer and also sets
// string_output_pointer.
#define output_pgm_string(str) do { \
		strcpy_P(string_output_buffer, str); \
		string_output_pointer = string_output_buffer; \
	} while(0)


#define STRING_OUTPUT_BUFFER_SIZE 100
extern uchar *string_output_pointer;
extern uchar string_output_buffer[STRING_OUTPUT_BUFFER_SIZE];


void init_keyboard_emulation();
void build_report_from_char(uchar c);
uchar send_next_char();

uchar nibble_to_hex(uchar n);
void uchar_to_hex(uchar v, uchar *str);
void int_to_hex(int v, uchar *str);
uchar* int_to_dec(int v, uchar *str);
uchar* append_newline_to_str(uchar *str);
uchar* array_to_hexdump(uchar *data, uchar len, uchar *str);
uchar* XYZVector_to_string(XYZVector* vector, uchar *str);


#endif  // __keyemu_h_included____

// vim:noexpandtab tabstop=4 shiftwidth=4 foldmethod=marker foldmarker={{{,}}}
