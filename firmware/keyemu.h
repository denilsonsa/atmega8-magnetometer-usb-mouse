/* Name: keyemu.h
 *
 * See the .c file for more information
 */

#ifndef __keyemu_h_included__
#define __keyemu_h_included__

#include "common.h"


// Copies a string from PGM to string_output_buffer and also sets
// string_output_pointer.
#define output_pgm_string(str) do { \
		strcpy_P(string_output_buffer, str); \
		string_output_pointer = string_output_buffer; \
	} while(0)


extern uchar *string_output_pointer;
extern uchar string_output_buffer[80];


// Init does nothing
#define init_keyboard_emulation() do{ }while(0)

void build_report_from_char(uchar c);
uchar send_next_char();

#endif  // __keyemu_h_included____

// vim:noexpandtab tabstop=4 shiftwidth=4 foldmethod=marker foldmarker={{{,}}}
