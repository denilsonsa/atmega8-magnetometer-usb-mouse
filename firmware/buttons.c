/* Name: buttons.c
 * Project: atmega8-magnetometer-usb-mouse
 * Author: Denilson Figueiredo de Sa
 * Creation Date: 2011-10-18
 * Tabsize: 4
 * License: GNU GPL v2 or GNU GPL v3
 */


#include <avr/io.h>

#include "buttons.h"


ButtonState button;


void init_button_state() {  // {{{
	// According to avr-libc FAQ, the compiler automatically initializes
	// all variables with zero.
	memset(&button, 0, sizeof(button));
}  // }}}

void update_button_state() { // {{{
	// This function implements debouncing code.
	// It should be called at every iteration of the main loop.
	// It reads the TOV0 flag status, but does not clear it.

	uchar filtered_state;

	filtered_state = button.state;

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
			button.debouncing[i] =
				(button.debouncing[i]<<1)
				| ( ((raw_state & (1<<i)))? 1 : 0 );

			if (button.debouncing[i] == 0) {
				// Releasing this button
				filtered_state &= ~(1<<i);
			} else if (button.debouncing[i] == 0xFF) {
				// Pressing this button
				filtered_state |=  (1<<i);
			}
		}
	}

	// Storing the final, filtered, updated state
	button.changed = button.state ^ filtered_state;
	button.state = filtered_state;
}  // }}}
