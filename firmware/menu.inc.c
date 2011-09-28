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

// Error item, for when something goes wrong  {{{
static const char  error_menu_item[] PROGMEM = "Error in menu system!\n";
#define            error_menu_total_items 1
static const PGM_P error_menu_strings[] PROGMEM = {
	error_menu_item
};
// Also other error messages:
static const char  error_sensor_string[] PROGMEM = "Error in sensor code!\n";
// }}}

// Main menu, with all main options  {{{
#define UI_MAIN_MENU 1

static const char  main_menu_1[] PROGMEM = "1. Calibrate zero\n";
static const char  main_menu_2[] PROGMEM = "2. Calibrate corners\n";
static const char  main_menu_3[] PROGMEM = "3. Sensor data\n";
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

// Sensor data menu  {{{
#define UI_SENSOR_MENU 4

static const char sensor_menu_1[] PROGMEM = "3.1. Print sensor identification string\n";
static const char sensor_menu_2[] PROGMEM = "3.2. Print configuration registers\n";
static const char sensor_menu_3[] PROGMEM = "3.3. Print X,Y,Z once\n";
static const char sensor_menu_4[] PROGMEM = "3.4. Print X,Y,Z continually\n";
static const char sensor_menu_5[] PROGMEM = "3.5. << main menu\n";
#define           sensor_menu_total_items 5
static PGM_P      sensor_menu_strings[] PROGMEM = {
	sensor_menu_1,
	sensor_menu_2,
	sensor_menu_3,
	sensor_menu_4,
	sensor_menu_5
};
// }}}


#define UI_MIN_MENU_ID UI_ROOT_MENU
#define UI_MAX_MENU_ID UI_SENSOR_MENU

#define UI_SENSOR_ID_WIDGET 0x10

// }}}

////////////////////////////////////////////////////////////
// UI and Menu variables (state)                         {{{

typedef struct UIState {
	// Current active UI widget
	uchar widget_id;
	// Current menu item, or other state for non-menu widgets
	uchar menu_item;
} UIState;

// Current UI state
UIState ui;

// At most 5 stacked widgets. This value is arbitrary.
UIState ui_stack[5];
uchar ui_stack_top;

// Each menu can have at most 8 menu items. This value is arbitrary.
static PGM_P ui_menu_strings[8];
static uchar ui_menu_total_items;

// Should the current menu item be printed in the next ui_main_code() call?
// If the firmware is busy printing something else, this flag won't be reset,
// and the current menu item will be printed whenever it is appropriate.
uchar ui_should_print_menu_item;

// }}}

////////////////////////////////////////////////////////////
// UI and menu handling functions                        {{{

static void ui_load_menu_items() {  // {{{
	// Loads the menu strings from PROGMEM to RAM

// Ah... A preprocessor macro to avoid copy-pasting
#define case_body(prefix) \
			memcpy_P(ui_menu_strings, prefix##_menu_strings, sizeof(prefix##_menu_strings)); \
			ui_menu_total_items = prefix##_menu_total_items; \
			break;

	switch (ui.widget_id) {
		case UI_ROOT_MENU:    case_body(empty)
		case UI_MAIN_MENU:    case_body(main)
		case UI_ZERO_MENU:    case_body(zero)
		case UI_CORNERS_MENU: case_body(corners)
		case UI_SENSOR_MENU:  case_body(sensor)
		default:              case_body(error)
	}

#undef case_body
}  // }}}

void ui_push_state() {  // {{{
	ui_stack[ui_stack_top] = ui;
	ui_stack_top++;
}  // }}}

void ui_pop_state() {  // {{{
	if (ui_stack_top > 0) {
		ui_stack_top--;
		ui = ui_stack[ui_stack_top];
	} else {
		ui.widget_id = UI_ROOT_MENU;
		ui.menu_item = 0;
	}

	// If the state is a menu, reload the items and print the current item.
	if (ui.widget_id >= UI_MIN_MENU_ID && ui.widget_id <= UI_MAX_MENU_ID) {
		ui_load_menu_items();
		ui_should_print_menu_item = 1;
	}
}  // }}}

static void ui_prev_menu_item() {  // {{{
	if (ui.menu_item == 0) {
		ui.menu_item = ui_menu_total_items;
	}
	ui.menu_item--;
	ui_should_print_menu_item = 1;
}  // }}}

static void ui_next_menu_item() {  // {{{
	ui.menu_item++;
	if (ui.menu_item == ui_menu_total_items) {
		ui.menu_item = 0;
	}
	ui_should_print_menu_item = 1;
}  // }}}

static void ui_enter_widget(uchar new_widget) {  // {{{
	// Pushes the current widget (usually a parent menu), and enters a
	// (sub)menu or widget.

	ui_push_state();

	ui.widget_id = new_widget;
	ui.menu_item = 0;

	// If the new widget is a menu, load the items and print the current one.
	if (ui.widget_id >= UI_MIN_MENU_ID && ui.widget_id <= UI_MAX_MENU_ID) {
		ui_load_menu_items();
		ui_should_print_menu_item = 1;
	}
}  // }}}

// }}}

////////////////////////////////////////////////////////////
// UI and menu public functions                          {{{

static void init_ui_system() {   // {{{
	// Must be called in the main initialization routine

	// Emptying the stack
	ui_stack_top = 0;

	// Calling ui_pop_state() with an empty stack will reload the initial
	// widget (the root/empty menu).
	ui_pop_state();
}  // }}}

static void ui_menu_code() {  // {{{
	// This function is huge, almost a big mess. That's because it handles the
	// actions of all menu items.


	// If the current menu item needs to be printed and the firmware is not
	// busy printing something else
	if (ui_should_print_menu_item && string_output_pointer == NULL) {
		output_pgm_string(ui_menu_strings[ui.menu_item]);
		ui_should_print_menu_item = 0;
	}


	if (ON_KEY_DOWN(BUTTON_PREV)) {
		ui_prev_menu_item();
	} else if (ON_KEY_DOWN(BUTTON_NEXT)) {
		ui_next_menu_item();
	} else if (ON_KEY_DOWN(BUTTON_CONFIRM)) {
		switch (ui.widget_id) {
			////////////////////
			case UI_ROOT_MENU:
				// This is an empty "fake" menu, with only one item.
				ui_enter_widget(UI_MAIN_MENU);
				break;

			////////////////////
			case UI_MAIN_MENU:
				switch (ui.menu_item) {
					case 0:
						ui_enter_widget(UI_ZERO_MENU);
						break;
					case 1:
						ui_enter_widget(UI_CORNERS_MENU);
						break;
					case 2:
						ui_enter_widget(UI_SENSOR_MENU);
						break;
					case 3:  // Quit menu
						ui_pop_state();
						break;
				}
				break;

			////////////////////
			case UI_ZERO_MENU:
				switch (ui.menu_item) {
					case 0:
					case 1:
					case 2:
						string_output_buffer[0] = 'z';
						uchar_to_hex(ui.menu_item, string_output_buffer+1);
						string_output_buffer[3] = '\n';
						string_output_buffer[4] = '\0';
						string_output_pointer = string_output_buffer;
						break;

					case 3:  // Back to main menu
						ui_pop_state();
						break;
				}
				break;

			////////////////////
			case UI_CORNERS_MENU:
				switch (ui.menu_item) {
					case 0:
					case 1:
					case 2:
						string_output_buffer[0] = 'c';
						uchar_to_hex(ui.menu_item, string_output_buffer+1);
						string_output_buffer[3] = '\n';
						string_output_buffer[4] = '\0';
						string_output_pointer = string_output_buffer;
						break;

					case 3:  // Back to main menu
						ui_pop_state();
						break;
				}
				break;

			////////////////////
			case UI_SENSOR_MENU:
				switch (ui.menu_item) {
					case 0:
						ui_enter_widget(UI_SENSOR_ID_WIDGET);
						break;
					case 1:
					case 2:
					case 3:
						string_output_buffer[0] = 's';
						uchar_to_hex(ui.menu_item, string_output_buffer+1);
						string_output_buffer[3] = '\n';
						string_output_buffer[4] = '\0';
						string_output_pointer = string_output_buffer;
						break;

					case 4:  // Back to main menu
						ui_pop_state();
						break;
				}
				break;
		}
	}
}  // }}}

static void ui_main_code() {  // {{{
	// This must be called in the main loop.
	//
	// This function handles the actions of all UI widgets.

	uchar return_code;

	if (ui.widget_id >= UI_MIN_MENU_ID && ui.widget_id <= UI_MAX_MENU_ID) {
		ui_menu_code();
	} else {
		switch (ui.widget_id) {
			////////////////////
			case UI_SENSOR_ID_WIDGET:
				if (string_output_pointer != NULL) {
					// Do nothing, let's wait the previous output...
					break;
				}
				if (ui.menu_item == 0) {
					sensor_func_step = 0;
					ui.menu_item = 1;
				}

				return_code = sensor_read_identification_string(string_output_buffer);

				if (return_code == SENSOR_FUNC_DONE) {
					append_newline_to_str(string_output_buffer);
					string_output_pointer = string_output_buffer;
					ui_pop_state();
				} else if (return_code == SENSOR_FUNC_ERROR) {
					output_pgm_string(error_sensor_string);
					ui_pop_state();
				}
				break;
		}
	}
}  // }}}

// }}}


// vim:noexpandtab tabstop=4 shiftwidth=4 foldmethod=marker foldmarker={{{,}}}
