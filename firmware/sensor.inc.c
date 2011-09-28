/* Name: sensor.inc.c
 * Project: atmega8-magnetometer-usb-mouse
 * Author: Denilson Figueiredo de Sa
 * Creation Date: 2011-09-28
 * Tabsize: 4
 * License: GNU GPL v2 or GNU GPL v3
 */


// This sensor has "L883 2105" written on the chip.
// It is known as HMC5883L or HMC5883.

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

// Return codes for the non-blocking functions  {{{

#define SENSOR_FUNC_STILL_WORKING 0
#define SENSOR_FUNC_DONE          1
#define SENSOR_FUNC_ERROR         2

// }}}

// }}}


// The X,Y,Z data from the sensor
int sensor_X;
int sensor_Y;
int sensor_Z;

// Boolean that detects if the sensor have reported an overflow
uchar sensor_overflow;
#define SENSOR_DATA_OVERFLOW -4096

// Set to 1 whenever new sensor data has been read.
uchar sensor_new_data_available;


// Used to determine the next step in non-blocking functions.
// Must be set to zero to ensure each function starts from the beginning.
uchar sensor_func_step;



static void sensor_set_address_pointer(uchar reg) {  // {{{
	// Sets the sensor internal register pointer.
	// This is required before reading registers.
	//
	// This function is non-blocking (except if TWI is already busy).

	uchar msg[2];
	msg[0] = SENSOR_I2C_WRITE_ADDRESS;
	msg[1] = reg;
	TWI_Start_Transceiver_With_Data(msg, 2);
}  // }}}

static void sensor_set_register_value(uchar reg, uchar value) {  // {{{
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


static uchar sensor_read_data_registers() {  // {{{
	// Reads the X,Y,Z data registers and store them at global vars.
	// In case of a transmission error, the previous values are not changed.
	//
	// This function is non-blocking.

	// 1 address byte + 6 data bytes = 7 bytes
	uchar msg[7];

	uchar lastTransOK;

	switch(sensor_func_step) {
		case 0:  // Set address pointer
			if (TWI_Transceiver_Busy()) return SENSOR_FUNC_STILL_WORKING;

			sensor_set_address_pointer(SENSOR_REG_DATA_START);
			sensor_func_step = 1;
		case 1:  // Start reading operation
			if (TWI_Transceiver_Busy()) return SENSOR_FUNC_STILL_WORKING;

			msg[0] = SENSOR_I2C_READ_ADDRESS;
			TWI_Start_Transceiver_With_Data(msg, 7);

			sensor_func_step = 2;
		case 2:  // Finished reading operation
			if (TWI_Transceiver_Busy()) return SENSOR_FUNC_STILL_WORKING;

			lastTransOK = TWI_Get_Data_From_Transceiver(msg, 7);
			sensor_func_step = 0;

			if (lastTransOK) {
				#define OFFSET (1 - SENSOR_REG_DATA_START)
				sensor_X = (msg[OFFSET+SENSOR_REG_DATA_X_MSB] << 8) | (msg[OFFSET+SENSOR_REG_DATA_X_LSB]);
				sensor_Y = (msg[OFFSET+SENSOR_REG_DATA_Y_MSB] << 8) | (msg[OFFSET+SENSOR_REG_DATA_Y_LSB]);
				sensor_Z = (msg[OFFSET+SENSOR_REG_DATA_Z_MSB] << 8) | (msg[OFFSET+SENSOR_REG_DATA_Z_LSB]);
				#undef OFFSET

				sensor_overflow =
					(sensor_X == SENSOR_DATA_OVERFLOW)
					|| (sensor_Y == SENSOR_DATA_OVERFLOW)
					|| (sensor_Z == SENSOR_DATA_OVERFLOW);

				sensor_new_data_available = 1;

				// TODO: zero compensation here.

				return SENSOR_FUNC_DONE;
			} else {
				return SENSOR_FUNC_ERROR;
			}
		default:
			return SENSOR_FUNC_ERROR;
	}
}  // }}}

static uchar sensor_read_identification_string(uchar *s) {  // {{{
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

	switch(sensor_func_step) {
		case 0:  // Set address pointer
			if (TWI_Transceiver_Busy()) return SENSOR_FUNC_STILL_WORKING;

			sensor_set_address_pointer(SENSOR_REG_ID_A);
			sensor_func_step = 1;
		case 1:  // Start reading operation
			if (TWI_Transceiver_Busy()) return SENSOR_FUNC_STILL_WORKING;

			msg[0] = SENSOR_I2C_READ_ADDRESS;
			TWI_Start_Transceiver_With_Data(msg, 4);

			sensor_func_step = 2;
		case 2:  // Finished reading operation
			if (TWI_Transceiver_Busy()) return SENSOR_FUNC_STILL_WORKING;

			lastTransOK = TWI_Get_Data_From_Transceiver(msg, 4);
			sensor_func_step = 0;

			if (lastTransOK) {
				s[0] = msg[1];
				s[1] = msg[2];
				s[2] = msg[3];
				s[3] = '\0';
				return SENSOR_FUNC_DONE;
			} else {
				return SENSOR_FUNC_ERROR;
			}
		default:
			return SENSOR_FUNC_ERROR;
	}
}  // }}}


static void init_sensor_configuration() {  // {{{
	sensor_func_step = 0;
	sensor_new_data_available = 0;

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
