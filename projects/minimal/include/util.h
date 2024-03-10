
#pragma once


#include <stdbool.h>
#include <stdint.h>




#define CLK_FREQ 20000000


uint32_t nplc_to_aper_n( double nplc, uint32_t lfreq );

double aper_n_to_nplc( uint32_t aper_n, uint32_t lfreq);

double aper_n_to_period( uint32_t aper_n);
uint32_t period_to_aper_n(  double period );

bool nplc_valid( double nplc );

void aper_cc_print( uint32_t aperture,  uint32_t lfreq);

//

unsigned str_decode_uint( const char *s, uint32_t *val);

unsigned str_decode_float( const char *s, double *val);
