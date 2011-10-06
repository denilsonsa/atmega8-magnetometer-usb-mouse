/* Name: sensor.h
 *
 * See the .c file for more information
 */

#ifndef __sensor_h_included__
#define __sensor_h_included__


#ifndef uchar
#define uchar  unsigned char
#endif


// Return codes for the non-blocking functions
#define SENSOR_FUNC_STILL_WORKING 0
#define SENSOR_FUNC_DONE          1
#define SENSOR_FUNC_ERROR         2


typedef struct XYZVector {
	int x, y, z;
} XYZVector;


// All vars...
// TODO: refactor this, maybe
extern XYZVector sensor_data;
extern uchar sensor_overflow;
#define SENSOR_DATA_OVERFLOW -4096
extern uchar sensor_new_data_available;
extern uchar sensor_error_while_reading;
extern uchar sensor_func_step;
extern uchar sensor_continuous_reading;
extern XYZVector sensor_zero;
extern uchar sensor_zero_compensation;

// EEPROM addresses
#define EEPROM_SENSOR_ZERO_ENABLE ((void*) 1)
#define EEPROM_SENSOR_ZERO_VECTOR ((void*) 2)


void sensor_set_address_pointer(uchar reg);
void sensor_set_register_value(uchar reg, uchar value);

uchar sensor_read_data_registers();

void sensor_start_continuous_reading();
void sensor_stop_continuous_reading();

uchar sensor_read_identification_string(uchar *s);

void sensor_init_configuration();


#endif  // __sensor_h_included____

// vim:noexpandtab tabstop=4 shiftwidth=4 foldmethod=marker foldmarker={{{,}}}
