/* Name: sensor.h
 *
 * See the .c file for more information
 */

#ifndef __sensor_h_included__
#define __sensor_h_included__

#include "common.h"


// Return codes for the non-blocking functions
#define SENSOR_FUNC_STILL_WORKING 0
#define SENSOR_FUNC_DONE          1
#define SENSOR_FUNC_ERROR         2

// Value that means "overflow"
#define SENSOR_DATA_OVERFLOW -4096


typedef struct XYZVector {
	int x, y, z;
} XYZVector;


typedef struct SensorData {
	union {
		uchar flags;
		struct {
			// Boolean that detects if the sensor have reported an overflow
			uchar overflow:1;

			// Set to 1 whenever new sensor data has been read.
			uchar new_data_available:1;

			// Almost the same as TWI_statusReg.lastTransOK.
			// Gets set whenever a function returns with SENSOR_FUNC_ERROR.
			// Gets reset whenever a function returns with SENSOR_FUNC_DONE.
			uchar error_while_reading:1;

			// Enable continuous reading of sensor values
			// This variable should be used in main() main loop (together
			// with a timer) to detect when sensor_read_data_registers()
			// should be called.
			uchar continuous_reading:1;

			// Boolean to enable Zero compensation
			uchar zero_compensation:1;
		};
	};

	// The X,Y,Z data from the sensor
	XYZVector data;

	// Zero calibration value
	XYZVector zero;

	// Zero calibration temporary values
	XYZVector zero_min;
	XYZVector zero_max;

	// Used to determine the next step in non-blocking functions.
	// Must be set to zero to ensure each function starts from the beginning.
	uchar func_step;

} SensorData;

extern SensorData sensor;


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
