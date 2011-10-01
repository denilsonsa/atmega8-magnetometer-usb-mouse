/********************************************************************
 * Copyright (C) 2003 ATMEL Corporation
 *
 * FILE       :  AVR104.C
 * COMPILER   :  IAR EWAVR 2.28A
 * MODIFIED   :  FEBRUARY 28, 2003
 *
 * REVISION HISTORY
 *   1.2     28 FEB 2003     JLL
 *   0.91    20 FEB 2003     TC
 *   0.9     15 JAN 2003     TC
 *
 ********************************************************************/

#include "defines.h"
#include "eeprom.h"

void main (void)
{
  unsigned char index;
  
  __disable_interrupt();                //Disable Global Interrupts
  MCUCR |= (2 << SM0);                  //Initialize Power-Down Sleep Mode
  DDRD = 0xFF;                          //Configure PORTD as Output

  /*Stuff plenty data into the buffer. Write data than there is room for in the buffer */
  /* to illustrate that overstuffing is handled) */
  for (index = 0; index < BUFFER_SIZE + 4; index++)  //Loop (BUFFER_SIZE + 4) Times
    EEPROM_PutChar(index, 255 - index);          //Write to EEPROM Buffer
                                                 //Begin @ EE Address $0000

  /*Read from EEPROM. Since not all data are written yet, data not yet written to */
  /* the EEPROM is read from the buffer rather than waiting for them to be written */
  /* to EEPROM*/
  for (index = 0; index < BUFFER_SIZE; index++)  //Loop Buffer_SIZE Times
    PORTD = EEPROM_GetChar(BUFFER_SIZE - index); //Read from EEPROM Buffer
                                                 //Begin @ End of Buffer

  /*Writing to an EEPROM address which is already in buffer. Data in buffer*/
  /* is altered so that the new data are written instead of the original data*/
  EEPROM_PutChar(0x02, 0x00);    //Write $00 to EEPROM @ End of Buffer
                                        //Update Buffer, Location Should Exist

  /*Read, modify, write and read. Only the first read actually accesses */
  /* the EEPROM. The second reading is from the buffer*/
  PORTD = EEPROM_GetChar(0x01FF);       //Read EEPROM Contents @ $01FF
  EEPROM_PutChar(0x01FF, 0xAA);         //Write $AA to EEPROM @ $01FF
  PORTD = EEPROM_GetChar(0x01FF);       //Read EEPROM Contents @ $01FF

  __sleep();                            //Sleep!

  for(;;)                               //Since EEPROM Ready interrupt will wake up the device
    __no_operation();
}
