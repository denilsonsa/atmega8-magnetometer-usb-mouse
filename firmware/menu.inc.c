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

////////////////////////////////////////////////////////////
// Menu definitions (constants in progmem)               {{{

// Empty menu, which is also the root menu
#define UI_ROOT_MENU 0

static const char empty_menu_item  [] PROGMEM = "";

static PGM_P empty_menu_strings[] PROGMEM = {
	empty_menu_item
};
#define empty_menu_total_items 1


// Main menu, with all main options
#define UI_MAIN_MENU 1

static const char main_menu_zero   [] PROGMEM = "1. Calibrate zero\n";
static const char main_menu_corners[] PROGMEM = "2. Calibrate corners\n";
static const char main_menu_sensor [] PROGMEM = "3. Read sensor\n";
static const char main_menu_return [] PROGMEM = "4. << quit menu\n";

static PGM_P main_menu_strings[] PROGMEM = {
	main_menu_zero,
	main_menu_corners,
	main_menu_sensor,
	main_menu_return
};
#define main_menu_total_items 4

// }}}

////////////////////////////////////////////////////////////
// UI and Menu variables (state)                         {{{

// Current active UI screen/widget
static uchar ui_active_widget;

// Each menu can have at most 8 menu items. This value is arbitrary.
static PGM_P menu_strings[8];
static uchar menu_current_item;
static uchar menu_total_items;

// }}}

////////////////////////////////////////////////////////////
// Generic menu handling functions                       {{{

static void load_menu_items(uchar which_menu) {  // {{{
	switch (which_menu) {
		case UI_ROOT_MENU:
			memcpy_P(menu_strings, empty_menu_strings, sizeof(empty_menu_strings));
			menu_total_items = empty_menu_total_items;
			break;

		case UI_MAIN_MENU:
			memcpy_P(menu_strings, main_menu_strings, sizeof(main_menu_strings));
			menu_total_items = main_menu_total_items;
			break;
	}
}  // }}}

#define print_menu_item() output_pgm_string(menu_strings[menu_current_item])

static void prev_menu_item() {  // {{{
	if (menu_current_item == 0) {
		menu_current_item = menu_total_items;
	}
	menu_current_item--;
	print_menu_item();
}  // }}}

static void next_menu_item() {  // {{{
	menu_current_item++;
	if (menu_current_item == menu_total_items) {
		menu_current_item = 0;
	}
	print_menu_item();
}  // }}}

static void init_menu_ui() {  // {{{
	// init_ is called upon "entering" (or starting) a UI "screen".

	// Reset menu cursor
	menu_current_item = 0;

	// Reload menu
	load_menu_items(ui_active_widget);
}  // }}}

static void refresh_menu_ui() {  // {{{
	// refresh_ is called right after init_, and also after coming back from a
	// "pop()" (i.e. after a UI "screen" has finished, refresh_ is called for
	// the previous one in the stack).

	// reload_menu...
	load_menu_items(ui_active_widget);

	print_menu_item();
}  // }}}

static void body_menu_ui() {  // {{{
	// main_ is called in the main loop.

	if (ON_KEY_DOWN(BUTTON_PREV)) {
		prev_menu_item();
	} else if (ON_KEY_DOWN(BUTTON_NEXT)) {
		next_menu_item();
	} else if (ON_KEY_DOWN(BUTTON_CONFIRM)) {
		switch (ui_active_widget) {
			case UI_ROOT_MENU:
				// Entering the main menu
				ui_active_widget = UI_MAIN_MENU;
				init_menu_ui();
				refresh_menu_ui();
				break;

			case UI_MAIN_MENU:

				switch (menu_current_item) {
					case 0:
					case 1:
					case 2:
						uchar_to_hex(menu_current_item, string_output_buffer);
						string_output_buffer[2] = '\n';
						string_output_buffer[3] = '\0';
						string_output_pointer = string_output_buffer;
						break;

					case 3:  // Quit menu
						ui_active_widget = UI_ROOT_MENU;
						init_menu_ui();
						refresh_menu_ui();
						break;
				}
				break;
		}
	}
}  // }}}

// }}}



static void init_ui_system() {   // {{{
	// Must be called in the main initialization routine
	ui_active_widget = UI_ROOT_MENU;
}  // }}}


// vim:noexpandtab tabstop=4 shiftwidth=4 foldmethod=marker foldmarker={{{,}}}
