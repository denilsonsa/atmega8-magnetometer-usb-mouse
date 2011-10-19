/* Name: buttons.h
 *
 * See the .c file for more information
 */

#ifndef __buttons_h_included__
#define __buttons_h_included__


#ifndef uchar
#define uchar  unsigned char
#endif


typedef struct ButtonState {
	// "Public" vars, already filtered for debouncing
	uchar state;
	uchar changed;

	// "Private" button debouncing state
	uchar debouncing[4];  // We have 4 buttons/switches
} ButtonState;

extern ButtonState button;


// Bit masks for each button (in PORTC and PINC)
// (also used in button.state and button.changed vars)
#define BUTTON_1      (1 << 0)
#define BUTTON_2      (1 << 1)
#define BUTTON_3      (1 << 2)
#define BUTTON_SWITCH (1 << 3)
#define ALL_BUTTONS   (BUTTON_1 | BUTTON_2 | BUTTON_3 | BUTTON_SWITCH)


// Handy macros!
// These have the same name/meaning of JavaScript events
#define ON_KEY_DOWN(button_mask) ((button.changed & (button_mask)) &&  (button.state & (button_mask)))
#define ON_KEY_UP(button_mask)   ((button.changed & (button_mask)) && !(button.state & (button_mask)))


// Init does nothing
#define init_button_state() do{ }while(0)

void update_button_state();


#endif  // __buttons_h_included____
