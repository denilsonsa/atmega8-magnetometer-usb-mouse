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

// Empty menu, which is also the root menu  {{{
// Yeah, this is just a fake menu with only one empty item.
#define UI_ROOT_MENU 0

static const char  empty_menu_item[] PROGMEM = "";
#define            empty_menu_total_items 1
static const PGM_P empty_menu_strings[] PROGMEM = {
	empty_menu_item
};
// }}}


// Main menu, with all main options  {{{
#define UI_MAIN_MENU 1

static const char  main_menu_1[] PROGMEM = "1. Calibrate zero\n";
static const char  main_menu_2[] PROGMEM = "2. Calibrate corners\n";
static const char  main_menu_3[] PROGMEM = "3. Read sensor\n";
static const char  main_menu_4[] PROGMEM = "4. << quit menu\n";
#define            main_menu_total_items 4
static const PGM_P main_menu_strings[] PROGMEM = {
	main_menu_1,
	main_menu_2,
	main_menu_3,
	main_menu_4
};
// }}}


// Zero calibration menu  {{{
#define UI_ZERO_MENU 2

static const char  zero_menu_1[] PROGMEM = "1.1. Print calibrated zero\n";
static const char  zero_menu_2[] PROGMEM = "1.2. Recalibrate zero\n";
static const char  zero_menu_3[] PROGMEM = "1.3. Toggle zero compensation\n";
static const char  zero_menu_4[] PROGMEM = "1.4. << main menu\n";
#define            zero_menu_total_items 4
static const PGM_P zero_menu_strings[] PROGMEM = {
	zero_menu_1,
	zero_menu_2,
	zero_menu_3,
	zero_menu_4
};
// }}}


// Corner calibration menu  {{{
#define UI_CORNERS_MENU 3

static const char corners_menu_1[] PROGMEM = "2.1. Print calibrated corners\n";
static const char corners_menu_2[] PROGMEM = "2.2. Recalibrate corners\n";
static const char corners_menu_3[] PROGMEM = "2.3. TODO: toggle algorithm being used, but here?\n";
static const char corners_menu_4[] PROGMEM = "2.4. << main menu\n";
#define           corners_menu_total_items 4
static PGM_P      corners_menu_strings[] PROGMEM = {
	corners_menu_1,
	corners_menu_2,
	corners_menu_3,
	corners_menu_4
};
// }}}

// }}}

////////////////////////////////////////////////////////////
// UI and Menu variables (state)                         {{{

// Current active UI screen/widget
static uchar ui_active_widget;

// Each menu can have at most 8 menu items. This value is arbitrary.
static PGM_P menu_strings[8];
static uchar menu_total_items;
static uchar menu_current_item;

// }}}

////////////////////////////////////////////////////////////
// Generic menu handling functions                       {{{

static void load_menu_items(uchar which_menu) {  // {{{

// Ah... A preprocessor macro to avoid copy-pasting
#define case_item(number, prefix) \
		case number: \
			memcpy_P(menu_strings, prefix##_menu_strings, sizeof(prefix##_menu_strings)); \
			menu_total_items = prefix##_menu_total_items; \
			break;

	switch (which_menu) {
		case_item(UI_ROOT_MENU, empty)
		case_item(UI_MAIN_MENU, main)
		case_item(UI_ZERO_MENU, zero)
		case_item(UI_CORNERS_MENU, corners)
	}

#undef case_item
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

static void refresh_menu_ui() {  // {{{
	// refresh_ is called right after init_, and also after coming back from a
	// "pop()" (i.e. after a UI "screen" has finished, refresh_ is called for
	// the previous one in the stack).

	// reload_menu...
	load_menu_items(ui_active_widget);

	print_menu_item();
}  // }}}

static void init_menu_ui() {  // {{{
	// init_ is called upon "entering" (or starting) a UI "screen".

	// Reset menu cursor
	menu_current_item = 0;

	refresh_menu_ui();
}  // }}}

static void enter_menu(uchar which_menu) {  // {{{
	ui_active_widget = which_menu;
	init_menu_ui();
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
				enter_menu(UI_MAIN_MENU);
				break;

			case UI_MAIN_MENU:
				switch (menu_current_item) {
					case 0:
						enter_menu(UI_ZERO_MENU);
						break;
					case 1:
						enter_menu(UI_CORNERS_MENU);
						break;
					case 2:
						string_output_buffer[0] = 'm';
						uchar_to_hex(menu_current_item, string_output_buffer+1);
						string_output_buffer[3] = '\n';
						string_output_buffer[4] = '\0';
						string_output_pointer = string_output_buffer;
						break;
					case 3:  // Quit menu
						enter_menu(UI_ROOT_MENU);
						break;
				}
				break;

			case UI_ZERO_MENU:
				switch (menu_current_item) {
					case 0:
					case 1:
					case 2:
						string_output_buffer[0] = 'z';
						uchar_to_hex(menu_current_item, string_output_buffer+1);
						string_output_buffer[3] = '\n';
						string_output_buffer[4] = '\0';
						string_output_pointer = string_output_buffer;
						break;

					case 3:  // Back to main menu
						ui_active_widget = UI_MAIN_MENU;
						init_menu_ui();
						refresh_menu_ui();
						break;
				}
				break;

			case UI_CORNERS_MENU:
				switch (menu_current_item) {
					case 0:
					case 1:
					case 2:
						string_output_buffer[0] = 'c';
						uchar_to_hex(menu_current_item, string_output_buffer+1);
						string_output_buffer[3] = '\n';
						string_output_buffer[4] = '\0';
						string_output_pointer = string_output_buffer;
						break;

					case 3:  // Back to main menu
						ui_active_widget = UI_MAIN_MENU;
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
