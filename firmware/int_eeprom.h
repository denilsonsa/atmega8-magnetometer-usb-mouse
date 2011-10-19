/* Name: int_eeprom.h
 *
 * See the .c file for more information
 */

#ifndef __int_eeprom_h_included__
#define __int_eeprom_h_included__


// Maximum block that can be written (at once) to the EEPROM
// (measured in bytes)
// 1 boolean zero_compensation (1 bytes)
// 1 XYZVector for the zero value (6 bytes)
// 4 XYZVectors for the calibration corners (6 bytes each)
// Total of 31
#define INT_EEPROM_BUFFER_SIZE   32


// Init does nothing
#define init_int_eeprom() do{ }while(0)

void int_eeprom_write_block(
		const void * src,
		void* address,
		unsigned char size);


#endif  // __int_eeprom_h_included____

// vim:noexpandtab tabstop=4 shiftwidth=4 foldmethod=marker foldmarker={{{,}}}
