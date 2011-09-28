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

// All available UI widgets
#define UI_ROOT_MENU 0
#define UI_MAIN_MENU 1
#define UI_ZERO_MENU 2
#define UI_CORNERS_MENU 3
#define UI_SENSOR_MENU 4

#define UI_MIN_MENU_ID UI_ROOT_MENU
#define UI_MAX_MENU_ID UI_SENSOR_MENU

#define UI_SENSOR_ID_WIDGET        0x10
#define UI_SENSOR_XYZ_ONCE_WIDGET  0x11
#define UI_SENSOR_XYZ_CONT_WIDGET  0x12


typedef struct MenuItem {
	// The menu text string
	PGM_P text;
	// The widget that will be activated on this menu item
	uchar action;
} MenuItem;


// Empty menu, which is also the root menu  {{{
// Yeah, this is just a fake menu with only one empty item.

static const char     empty_menu_1[] PROGMEM = "";
#define               empty_menu_total_items 1
static const MenuItem empty_menu_items[] PROGMEM = {
	{empty_menu_1, UI_MAIN_MENU}
};
// }}}

// Error item, for when something goes wrong  {{{
static const char     error_menu_1[] PROGMEM = "Error in menu system!\n";
#define               error_menu_total_items 1
static const MenuItem error_menu_items[] PROGMEM = {
	{error_menu_1, 0}
};
// Also other error messages:
static const char  error_sensor_string[] PROGMEM = "Error while reading the sensor!\n";
// }}}

// Main menu, with all main options  {{{

static const char     main_menu_1[] PROGMEM = "1. Calibrate zero\n";
static const char     main_menu_2[] PROGMEM = "2. Calibrate corners\n";
static const char     main_menu_3[] PROGMEM = "3. Sensor data\n";
static const char     main_menu_4[] PROGMEM = "4. << quit menu\n";
#define               main_menu_total_items 4
static const MenuItem main_menu_items[] PROGMEM = {
	{main_menu_1, UI_ZERO_MENU},
	{main_menu_2, UI_CORNERS_MENU},
	{main_menu_3, UI_SENSOR_MENU},
	{main_menu_4, 0}
};
// }}}

// Zero calibration menu  {{{

static const char     zero_menu_1[] PROGMEM = "1.1. Print calibrated zero\n";
static const char     zero_menu_2[] PROGMEM = "1.2. Recalibrate zero\n";
static const char     zero_menu_3[] PROGMEM = "1.3. Toggle zero compensation\n";
static const char     zero_menu_4[] PROGMEM = "1.4. << main menu\n";
#define               zero_menu_total_items 4
static const MenuItem zero_menu_items[] PROGMEM = {
	{zero_menu_1, UI_ROOT_MENU},
	{zero_menu_2, UI_ROOT_MENU},
	{zero_menu_3, UI_ROOT_MENU},
	{zero_menu_4, 0}
};
// }}}

// Corner calibration menu  {{{

static const char     corners_menu_1[] PROGMEM = "2.1. Print calibrated corners\n";
static const char     corners_menu_2[] PROGMEM = "2.2. Recalibrate corners\n";
static const char     corners_menu_3[] PROGMEM = "2.3. TODO: toggle algorithm being used, but here?\n";
static const char     corners_menu_4[] PROGMEM = "2.4. << main menu\n";
#define               corners_menu_total_items 4
static const MenuItem corners_menu_items[] PROGMEM = {
	{corners_menu_1, UI_ROOT_MENU},
	{corners_menu_2, UI_ROOT_MENU},
	{corners_menu_3, UI_ROOT_MENU},
	{corners_menu_4, 0}
};
// }}}

// Sensor data menu  {{{

static const char     sensor_menu_1[] PROGMEM = "3.1. Print sensor identification string\n";
static const char     sensor_menu_2[] PROGMEM = "3.2. Print X,Y,Z once\n";
static const char     sensor_menu_3[] PROGMEM = "3.3. Print X,Y,Z continually\n";
static const char     sensor_menu_4[] PROGMEM = "3.4. << main menu\n";
#define               sensor_menu_total_items 4
static const MenuItem sensor_menu_items[] PROGMEM = {
	{sensor_menu_1, UI_SENSOR_ID_WIDGET},
	{sensor_menu_2, UI_SENSOR_XYZ_ONCE_WIDGET},
	{sensor_menu_3, UI_SENSOR_XYZ_CONT_WIDGET},
	{sensor_menu_4, 0}
};
// }}}


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

// Each menu can have at most 6 menu items. This value is arbitrary.
static MenuItem ui_menu_items[6];
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
			memcpy_P(ui_menu_items, prefix##_menu_items, sizeof(prefix##_menu_items)); \
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

	// If the current menu item needs to be printed and the firmware is not
	// busy printing something else
	if (ui_should_print_menu_item && string_output_pointer == NULL) {
		output_pgm_string(ui_menu_items[ui.menu_item].text);
		ui_should_print_menu_item = 0;
	}


	if (ON_KEY_DOWN(BUTTON_PREV)) {
		ui_prev_menu_item();
	} else if (ON_KEY_DOWN(BUTTON_NEXT)) {
		ui_next_menu_item();
	} else if (ON_KEY_DOWN(BUTTON_CONFIRM)) {
		uchar action;
		action = ui_menu_items[ui.menu_item].action;

		if (action == 0) {
			ui_pop_state();
		} else {
			ui_enter_widget(action);
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
			case UI_SENSOR_ID_WIDGET:  // {{{
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
				break;  // }}}

			////////////////////
			case UI_SENSOR_XYZ_ONCE_WIDGET:  // {{{
				if (ui.menu_item == 0) {
					if (string_output_pointer != NULL) {
						// Do nothing, let's wait the previous output...
						break;
					}
					sensor_start_continuous_reading();
					ui.menu_item = 1;
				} else {
					if (string_output_pointer == NULL) {
						if (sensor_new_data_available) {
							sensor_new_data_available = 0;
							sensor_stop_continuous_reading();
							debug_print_X_Y_Z_to_string_output_buffer();
							string_output_pointer = string_output_buffer;
							ui_pop_state();
						} else if (sensor_error_while_reading) {
							sensor_stop_continuous_reading();
							output_pgm_string(error_sensor_string);
							ui_pop_state();
						}
					}
				}
				break;  // }}}

			////////////////////
			case UI_SENSOR_XYZ_CONT_WIDGET:  // {{{
				if (ui.menu_item == 0) {
					if (string_output_pointer != NULL) {
						// Do nothing, let's wait the previous output...
						break;
					}
					sensor_start_continuous_reading();
					ui.menu_item = 1;
				} else {
					if (string_output_pointer == NULL) {
						if (sensor_new_data_available) {
							sensor_new_data_available = 0;
							debug_print_X_Y_Z_to_string_output_buffer();
							string_output_pointer = string_output_buffer;
						} else if (sensor_error_while_reading) {
							sensor_stop_continuous_reading();
							output_pgm_string(error_sensor_string);
							ui_pop_state();
						}
					}
					if (ON_KEY_DOWN(BUTTON_CONFIRM)) {
						sensor_stop_continuous_reading();
						ui_pop_state();
					}
				}
				break;  // }}}

			default:
				// Fallback in case of errors
				ui_pop_state();
		}
	}
}  // }}}

// }}}


// vim:noexpandtab tabstop=4 shiftwidth=4 foldmethod=marker foldmarker={{{,}}}
