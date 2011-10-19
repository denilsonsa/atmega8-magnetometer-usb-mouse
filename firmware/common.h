/* Name: common.h
 *
 * Random useful things that I don't want to repeat everywhere.
 */

#ifndef __common_h_included__
#define __common_h_included__


#ifndef uchar
#define uchar  unsigned char
#endif


// http://www.tty1.net/blog/2008-04-29-avr-gcc-optimisations_en.html
#define FIX_POINTER(_ptr) __asm__ __volatile__("" : "=b" (_ptr) : "0" (_ptr))


#endif  // __common_h_included__

