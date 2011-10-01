/********************************************************************
 * Copyright (C) 2003 ATMEL Corporation
 *
 * FILE       :  AVR104.C
 * COMPILER   :  IAR EWAVR 2.28A
 * MODIFIED   :  FEBRUARY 20, 2003
 *
 * REVISION HISTORY
 *   1.0     27 Feb 2003    JLL
 *   0.91    20 FEB 2003    TC
 *   0.9     15 JAN 2003    TC
 *
 ********************************************************************/

#ifndef __DEFINES_H
#define __DEFINES_H

#include <inavr.h>                      // Include Intrinsic Functions
#include <ioavr.h>                      // Include device definitions according
                                        //  to the device selected in project options

#ifdef __AT90Mega128__                  //Since some devices use the register name SPMCSR rather than SPMCR
 #define SPMCR SPMCSR
#endif

#ifdef __AT90Mega169__                  //Since some devices use the register name SPMCSR rather than SPMCR
 #define SPMCR SPMCSR
#endif

#endif /* __DEFINES_H */
