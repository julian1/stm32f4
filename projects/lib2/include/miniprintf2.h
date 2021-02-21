/* Minimal printf() facility for MCUs
 * Warren W. Gay VE3WWG, Sun Feb 12 2017
 * 
 * This work is placed into the public domain. No warranty, or guarantee
 * is expressed or implied. When uou use this source code, you do so
 * with full responsibility.
 */
#ifndef MINIPRINTF_H
#define MINIPRINTF_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdarg.h>


int
mini_printf(
	void	(*putc)(void *, char),	
	void 	*argp,			          
  const char *format,
  ...
)
; 


#ifdef __cplusplus
}
#endif

#endif // MINIPRINTF_H

/* End miniprintf.h */
