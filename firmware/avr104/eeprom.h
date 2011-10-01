/********************************************************************
 * Copyright (C) 2003 ATMEL Corporation
 *
 * FILE       :  EEPROM.H
 * COMPILER   :  IAR EWAVR 2.28A
 * MODIFIED   :  FEBRUARY 28, 2003
 *
 * REVISION HISTORY
 *   1.2     28 Feb 2003     JLL
 *   0.91    20 FEB 2003     TC
 *   0.9     15 JAN 2003     TC
 *
 ********************************************************************/

#ifndef __EEPROM_H
#define __EEPROM_H

#define BUFFER_SIZE   16                // EEPROM aBuffer & dBuffer Size

unsigned char EEPROM_GetChar (unsigned int address);
void EEPROM_PutChar (unsigned int address, unsigned char data);

#endif /* __EEPROM_H */
