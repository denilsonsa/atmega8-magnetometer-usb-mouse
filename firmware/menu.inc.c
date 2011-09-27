/* Name: menu.inc.c
 * Project: atmega8-magnetometer-usb-mouse
 * Author: Denilson Figueiredo de Sa
 * Creation Date: 2011-09-27
 * Tabsize: 4
 * License: GNU GPL v2 or GNU GPL v3
 */


#define BUTTON_PREV    BUTTON_1
#define BUTTON_NEXT    BUTTON_2
#define BUTTON_CONFIRM BUTTON_3


static const char main_menu_zero   [] PROGMEM = "1. Calibrate zero\n";
static const char main_menu_corners[] PROGMEM = "2. Calibrate corners\n";
static const char main_menu_sensor [] PROGMEM = "3. Read sensor\n";

static PGM_P main_menu_strings[] = {
	main_menu_zero,
	main_menu_corners,
	main_menu_sensor,
	NULL
};

static uchar current_item;


static void init_menu_ui() {
	current_item = 0;
	output_pgm_string(main_menu_strings[current_item]);
}

void body_menu_ui() {
	if (ON_KEY_DOWN(BUTTON_PREV)) {
		if (current_item == 0) {
			// Find the NULL terminator...
			while (main_menu_strings[current_item] != NULL)
				current_item++;
			// Now go back to the last item.
			current_item--;
		} else {
			current_item--;
		}
		output_pgm_string(main_menu_strings[current_item]);
	} else if (ON_KEY_DOWN(BUTTON_NEXT)) {
		current_item++;
		if (main_menu_strings[current_item] == NULL)
			current_item = 0;
		output_pgm_string(main_menu_strings[current_item]);
	} else if (ON_KEY_DOWN(BUTTON_CONFIRM)) {
		uchar_to_hex(current_item, string_output_buffer);
		string_output_buffer[2] = '\n';
		string_output_buffer[3] = '\0';
		string_output_pointer = string_output_buffer;
	}
}


// vim:noexpandtab tabstop=4 shiftwidth=4 foldmethod=marker foldmarker={{{,}}}
