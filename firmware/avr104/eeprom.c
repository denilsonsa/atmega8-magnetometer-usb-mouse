/********************************************************************
 * Copyright (C) 2003 ATMEL Corporation
 *
 * FILE       :  EEPROM.C
 * COMPILER   :  IAR EWAVR 2.28A
 * MODIFIED   :  FEBRUARY 28, 2003
 *
 * REVISION HISTORY
 *   1.2     28 Feb 2003     JLL
 *   0.91    20 FEB 2003     TC
 *   0.9     15 JAN 2003     TC
 *
 ********************************************************************/

#include "defines.h"
#include "eeprom.h"

static unsigned int  aBuffer [BUFFER_SIZE] =  //EEPROM Address Buffer
  {0xFFFF,0xFFFF,0xFFFF,0xFFFF,0xFFFF,0xFFFF,0xFFFF,0xFFFF,
  0xFFFF,0xFFFF,0xFFFF,0xFFFF,0xFFFF,0xFFFF,0xFFFF,0xFFFF};   
static unsigned char dBuffer [BUFFER_SIZE];   //EEPROM Data Buffer
static unsigned char bCount = 0;              //EEPROM Buffer Locations in Use
static unsigned char rSleep = 0;              //Sleep Register Backup

unsigned char EEPROM_GetChar (unsigned int address)
{
  unsigned char bIndex = 0;             //Initialize Buffer Index Counter
  unsigned char temp,temp2;             //For the return value etc
  do
  {
    __disable_interrupt();              //Interrupt disabled to ensure that buffer 
                                        // is not accessed prior to returning the buffer 
                                        // contents in case of a hit. 
    if (aBuffer[bIndex] == address)     //Is Address Currently in Buffer?
    { 
      temp = dBuffer[bIndex];
      __enable_interrupt();
      return (temp);                    //Return Data Buffer Contents to main()
    }
    else bIndex++;                      //Increment Buffer Index Counter
    __enable_interrupt();
  } while (bIndex < bCount);            //Loop Until Entire Buffer Contents Parsed

  temp2 = EECR & (1<<EERIE);            //Back-up EERIE bit
  EECR &= ~(1<<EERIE);                  //Disable EEPROM interrupt to let the EEPROM read in
  
  while ( EECR & (1 << EEWE));          //Is EEPROM Currently Being Accessed?  Yes, Loop
  EEAR = address;                       //Place Address in EEPROM Address Register
  EECR |= (1 << EERE);                  //Assert EEPROM Read Enable
  EEAR |= 0x0000;                       //Clear EEPROM Address Register
  temp = EEDR;                          //Snatch data from EEPROM data register
  
  EECR |= temp2;                        //Restore EERIE bit
  
  return (temp);                        //Return EEPROM Data Contents to main()
}


void EEPROM_PutChar (unsigned int address, unsigned char data)
{
  unsigned char bIndex = 0;             //Initialize Buffer Index Counter

  __disable_interrupt();                //Disable Global Interrupts

  do
  {
    if (aBuffer[bIndex] == address)     //Is Address Currently in Buffer?
    {
      dBuffer[bIndex] = data;           //Update Data in EEPROM Data Buffer
      __enable_interrupt();             //Enable Global Interrupts
      return;                           //Return to main()
    }
    else bIndex++;                      //Increment Buffer Index Counter
  } while (bIndex < bCount);            //Loop Until Entire Buffer Parsed

  while ( !(bCount < BUFFER_SIZE) )     //Loop Until Buffer Space Available
    __enable_interrupt();               //Enable Global Interrupts

  __disable_interrupt();                //Disable Global Interrupts
  aBuffer[bCount] = address;            //Place Address in EEPROM Address Buffer
  dBuffer[bCount] = data;               //Place Data in EEPROM Data Buffer
  bCount++;                             //Increment Buffer Counter

  if ( MCUCR & (1 << SM1) )             //If Power-Down, Power-Save, Standby or
  {                                     //Extended Standby Sleep Modes Selected
    rSleep = ( MCUCR & ( (1<<SM2)|(1<<SM1)|(1<<SM0) ) );   // Backup Current Sleep Mode Selection
    MCUCR &= ~( (1<<SM2)|(1<<SM1)|(1 << SM0) );    //Configure Sleep Mode to Operate in Idle mode.
                                                   // Alternatively ADC Noise Reduction Mode can be used
                                                   // however this requires that the modules disabled in
                                                   // ADC NR mode are not used.
  }

  EECR |= (1 << EERIE);                 // Enable EE_RDY Interrupt
  __enable_interrupt();                 // Enable Global Interrupts
  return;                               // Return to main()
}


#pragma vector = EE_RDY_vect
__interrupt void EE_RDY_ISR (void)
{
  if ( SPMCR & (1 << SPMEN) )           // Is Self-Programming Currently Active?
    return;                             // Yes, Return to main()

  EEAR = aBuffer[bCount - 1];           // Place EEPROM Address in EEAR
  EEDR = dBuffer[bCount - 1];           // Place EEPROM Data in EEDR
  EECR |= (1 << EEMWE);                  // Assert EEPROM Master Write Enable
  EECR |= (1 << EEWE);                   // Assert EEPROM Write Enable

  aBuffer[bCount - 1] = 0xFFFF;         // Clear EEPROM Address Buffer Location
  dBuffer[bCount - 1] = 0xFF;           // Clear EEPROM Data Buffer Location
  bCount--;                             // Decrement EEPROM Buffer Count

  if (bCount == 0)                      // Is EEPROM Buffer Empty?
  {
    EECR  &= ~(1 << EERIE);             // Disable EE_RDY Interrupt
    MCUCR &= ~( (1<<SM2)|(1<<SM1)|(1<<SM0) );  // Clear SMx Bits in MCUCR
    MCUCR |= rSleep;                    // Restore Previous Sleep Mode
  }
}
