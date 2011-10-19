/* Name: sensor.c
 * Project: atmega8-magnetometer-usb-mouse
 * Author: Denilson Figueiredo de Sa
 * Creation Date: 2011-09-28
 * Tabsize: 4
 * License: GNU GPL v2 or GNU GPL v3
 *
 * Communication with HMC5883L or HMC5883 from Honeywell.
 * This sensor has "L883 2105" written on the chip.
 * This sensor is a 3-axis magnetometer with I2C interface.
 */


#include <avr/eeprom.h>

#include "avr315/TWI_Master.h"
#include "sensor.h"


SensorData sensor;


// Avoiding GCC optimizing-out these EEPROM vars
// Maybe I should put them inside a struct?
// http://www.avrfreaks.net/index.php?name=PNphpBB2&file=viewtopic&t=68621
#define X_EEMEM __attribute__((section(".eeprom"), used, externally_visible))

// "Default" EEPROM values:
uchar     X_EEMEM eeprom_sensor_unused = 0;
uchar     X_EEMEM eeprom_sensor_zero_compensation = 1;
XYZVector X_EEMEM eeprom_sensor_zero = {21, -108, 138};


////////////////////////////////////////////////////////////
// Constant definitions                                  {{{

#define SENSOR_I2C_READ_ADDRESS  0x3D
#define SENSOR_I2C_WRITE_ADDRESS 0x3C

// HMC5883L register definitions  {{{
// See page 11 of HMC5883L.pdf

// Read/write registers:
#define SENSOR_REG_CONF_A       0
#define SENSOR_REG_CONF_B       1
#define SENSOR_REG_MODE         2
// Read-only registers:
#define SENSOR_REG_DATA_START   3
#define SENSOR_REG_DATA_X_MSB   3
#define SENSOR_REG_DATA_X_LSB   4
#define SENSOR_REG_DATA_Z_MSB   5
#define SENSOR_REG_DATA_Z_LSB   6
#define SENSOR_REG_DATA_Y_MSB   7
#define SENSOR_REG_DATA_Y_LSB   8
#define SENSOR_REG_STATUS       9
#define SENSOR_REG_ID_A        10
#define SENSOR_REG_ID_B        11
#define SENSOR_REG_ID_C        12

// }}}

// HMC5883L status definitions  {{{
// See page 16 of HMC5883L.pdf

#define SENSOR_STATUS_LOCK  2
#define SENSOR_STATUS_RDY   1

// }}}

// HMC5883L configuration definitions  {{{
// See pages 12, 13, 14 of HMC5883L.pdf

// The Mode Register has 2 possible values for Idle mode.
#define SENSOR_MODE_CONTINUOUS  0
#define SENSOR_MODE_SINGLE      1
#define SENSOR_MODE_IDLE_A      2
#define SENSOR_MODE_IDLE_B      3
#define SENSOR_MODE_MASK        3

// How many samples averaged? Default=1
#define SENSOR_CONF_A_SAMPLES_1    0x00
#define SENSOR_CONF_A_SAMPLES_2    0x20
#define SENSOR_CONF_A_SAMPLES_4    0x40
#define SENSOR_CONF_A_SAMPLES_8    0x60
#define SENSOR_CONF_A_SAMPLES_MASK 0x60

// Data output rate for continuous mode. Default=15Hz
#define SENSOR_CONF_A_RATE_0_75     0x00
#define SENSOR_CONF_A_RATE_1_5      0x04
#define SENSOR_CONF_A_RATE_3        0x08
#define SENSOR_CONF_A_RATE_7_5      0x0C
#define SENSOR_CONF_A_RATE_15       0x10
#define SENSOR_CONF_A_RATE_30       0x14
#define SENSOR_CONF_A_RATE_75       0x18
#define SENSOR_CONF_A_RATE_RESERVED 0x1C
#define SENSOR_CONF_A_RATE_MASK     0x1C

// Measurement configuration, whether to apply bias. Default=Normal
#define SENSOR_CONF_A_BIAS_NORMAL   0x00
#define SENSOR_CONF_A_BIAS_POSITIVE 0x01
#define SENSOR_CONF_A_BIAS_NEGATIVE 0x02
#define SENSOR_CONF_A_BIAS_RESERVED 0x03
#define SENSOR_CONF_A_BIAS_MASK     0x03

// Gain configuration. Default=1.3Ga
#define SENSOR_CONF_B_GAIN_0_88 0x00
#define SENSOR_CONF_B_GAIN_1_3  0x20
#define SENSOR_CONF_B_GAIN_1_9  0x40
#define SENSOR_CONF_B_GAIN_2_5  0x60
#define SENSOR_CONF_B_GAIN_4_0  0x80
#define SENSOR_CONF_B_GAIN_4_7  0xA0
#define SENSOR_CONF_B_GAIN_5_6  0xC0
#define SENSOR_CONF_B_GAIN_8_1  0xE0
#define SENSOR_CONF_B_GAIN_MASK 0xE0

// Digital resolution (mG/LSb) for each gain
#define SENSOR_GAIN_SCALE_0_88  0.73
#define SENSOR_GAIN_SCALE_1_3   0.92
#define SENSOR_GAIN_SCALE_1_9   1.22
#define SENSOR_GAIN_SCALE_2_5   1.52
#define SENSOR_GAIN_SCALE_4_0   2.27
#define SENSOR_GAIN_SCALE_4_7   2.56
#define SENSOR_GAIN_SCALE_5_6   3.03
#define SENSOR_GAIN_SCALE_8_1   4.35

// }}}

// }}}


void sensor_set_address_pointer(uchar reg) {  // {{{
	// Sets the sensor internal register pointer.
	// This is required before reading registers.
	//
	// This function is non-blocking (except if TWI is already busy).

	uchar msg[2];
	msg[0] = SENSOR_I2C_WRITE_ADDRESS;
	msg[1] = reg;
	TWI_Start_Transceiver_With_Data(msg, 2);
}  // }}}

void sensor_set_register_value(uchar reg, uchar value) {  // {{{
	// Sets one of those 3 writable registers to a value.
	// Only useful for configuration.
	//
	// This function is non-blocking (except if TWI is already busy).

	uchar msg[3];
	msg[0] = SENSOR_I2C_WRITE_ADDRESS;
	msg[1] = reg;
	msg[2] = value;
	TWI_Start_Transceiver_With_Data(msg, 3);
}  // }}}


uchar sensor_read_data_registers() {  // {{{
	// Reads the X,Y,Z data registers and store them at global vars.
	// In case of a transmission error, the previous values are not changed.
	//
	// This function is non-blocking.

	// 1 address byte + 6 data bytes = 7 bytes
	uchar msg[7];

	uchar lastTransOK;

	SensorData *sens = &sensor;
	FIX_POINTER(sens);

	switch(sens->func_step) {
		case 0:  // Set address pointer
			if (TWI_Transceiver_Busy()) return SENSOR_FUNC_STILL_WORKING;

			sensor_set_address_pointer(SENSOR_REG_DATA_START);
			sens->func_step = 1;
		case 1:  // Start reading operation
			if (TWI_Transceiver_Busy()) return SENSOR_FUNC_STILL_WORKING;

			msg[0] = SENSOR_I2C_READ_ADDRESS;
			TWI_Start_Transceiver_With_Data(msg, 7);

			sens->func_step = 2;
		case 2:  // Finished reading operation
			if (TWI_Transceiver_Busy()) return SENSOR_FUNC_STILL_WORKING;

			lastTransOK = TWI_Get_Data_From_Transceiver(msg, 7);
			sens->func_step = 0;

			if (lastTransOK) {
				#define OFFSET (1 - SENSOR_REG_DATA_START)
				sens->data.x = (msg[OFFSET+SENSOR_REG_DATA_X_MSB] << 8) | (msg[OFFSET+SENSOR_REG_DATA_X_LSB]);
				sens->data.y = (msg[OFFSET+SENSOR_REG_DATA_Y_MSB] << 8) | (msg[OFFSET+SENSOR_REG_DATA_Y_LSB]);
				sens->data.z = (msg[OFFSET+SENSOR_REG_DATA_Z_MSB] << 8) | (msg[OFFSET+SENSOR_REG_DATA_Z_LSB]);
				#undef OFFSET

				sens->overflow =
					(sens->data.x == SENSOR_DATA_OVERFLOW)
					|| (sens->data.y == SENSOR_DATA_OVERFLOW)
					|| (sens->data.z == SENSOR_DATA_OVERFLOW);

				sens->new_data_available = 1;

				if (sens->zero_compensation && !sens->overflow) {
					sens->data.x -= sens->zero.x;
					sens->data.y -= sens->zero.y;
					sens->data.z -= sens->zero.z;
				}

				sens->error_while_reading = 0;
				return SENSOR_FUNC_DONE;
			} else {
				sens->error_while_reading = 1;
				return SENSOR_FUNC_ERROR;
			}
		default:
			sens->error_while_reading = 1;
			return SENSOR_FUNC_ERROR;
	}
}  // }}}

void sensor_start_continuous_reading() {  // {{{
	SensorData *sens = &sensor;
	FIX_POINTER(sens);

	sens->func_step = 0;
	sens->new_data_available = 0;
	sens->error_while_reading = 0;
	sens->continuous_reading = 1;
}  // }}}

void sensor_stop_continuous_reading() {  // {{{
	SensorData *sens = &sensor;
	FIX_POINTER(sens);

	sens->func_step = 0;
	//sens->new_data_available = 0;
	//sens->error_while_reading = 0;
	sens->continuous_reading = 0;
}  // }}}


uchar sensor_read_identification_string(uchar *s) {  // {{{
	// Reads the 3 identification registers from the sensor.
	// They should read as ASCII "H43".
	//
	// Receives a pointer to a string with at least 4 chars of size.
	// After reading the registers, stores them at *s, followed by '\0'.
	// In case of a transmission error, the *s is not touched.
	//
	// This function is non-blocking.

	// 1 address byte + 3 chars
	uchar msg[4];

	uchar lastTransOK;

	switch(sensor.func_step) {
		case 0:  // Set address pointer
			if (TWI_Transceiver_Busy()) return SENSOR_FUNC_STILL_WORKING;

			sensor_set_address_pointer(SENSOR_REG_ID_A);
			sensor.func_step = 1;
		case 1:  // Start reading operation
			if (TWI_Transceiver_Busy()) return SENSOR_FUNC_STILL_WORKING;

			msg[0] = SENSOR_I2C_READ_ADDRESS;
			TWI_Start_Transceiver_With_Data(msg, 4);

			sensor.func_step = 2;
		case 2:  // Finished reading operation
			if (TWI_Transceiver_Busy()) return SENSOR_FUNC_STILL_WORKING;

			lastTransOK = TWI_Get_Data_From_Transceiver(msg, 4);
			sensor.func_step = 0;

			if (lastTransOK) {
				s[0] = msg[1];
				s[1] = msg[2];
				s[2] = msg[3];
				s[3] = '\0';
				sensor.error_while_reading = 0;
				return SENSOR_FUNC_DONE;
			} else {
				sensor.error_while_reading = 1;
				return SENSOR_FUNC_ERROR;
			}
		default:
			sensor.error_while_reading = 1;
			return SENSOR_FUNC_ERROR;
	}
}  // }}}


void sensor_init_configuration() {  // {{{
	// This must be called AFTER interrupts were enabled and AFTER
	// TWI_Master has been initialized.

	// According to avr-libc FAQ, the compiler automatically initializes all
	// variables with zero.
	//sensor.func_step = 0;
	//sensor.new_data_available = 0;
	//sensor.error_while_reading = 0;

	// Safer code for reading from the EEPROM:
	//eeprom_read_block(&sensor.zero, &eeprom_sensor_zero, sizeof(sensor.zero));
	//sensor.zero_compensation = eeprom_read_byte(&eeprom_sensor_zero_compensation);

	// Smaller code for reading from the EEPROM:
	// Assuming there is no padding in SensorData struct, we can do this:
	eeprom_read_block(SENSOR_STRUCT_EEPROM_START, &eeprom_sensor_zero_compensation, SENSOR_STRUCT_EEPROM_SIZE);


	sensor_set_register_value(
		SENSOR_REG_CONF_A,
		SENSOR_CONF_A_SAMPLES_8
		| SENSOR_CONF_A_RATE_75
		| SENSOR_CONF_A_BIAS_NORMAL
	);
	sensor_set_register_value(
		SENSOR_REG_CONF_B,
		SENSOR_CONF_B_GAIN_1_3
	);
	sensor_set_register_value(
		SENSOR_REG_MODE,
		SENSOR_MODE_CONTINUOUS
	);
}  // }}}


// vim:noexpandtab tabstop=4 shiftwidth=4 foldmethod=marker foldmarker={{{,}}}
