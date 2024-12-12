
#pragma once


#include <stdbool.h>
#include <stdint.h>



// move to ice40_reg.h ?
#define CLK_FREQ 20000000

// rename aper_n  to just aperture which is always expressed in clock count

uint32_t nplc_to_aperture( double nplc, uint32_t lfreq );

double aper_n_to_nplc( uint32_t aper_n, uint32_t lfreq);

double aper_n_to_period( uint32_t aper_n);
uint32_t period_to_aper_n(  double period );

bool nplc_valid( double nplc );

void aper_cc_print( uint32_t aperture,  uint32_t lfreq);

//

unsigned str_decode_uint( const char *s, uint32_t *val);

unsigned str_decode_int( const char *s, int32_t *val);

unsigned str_decode_float( const char *s, double *val);

// better name mux_to_string?
char * mux_to_string( unsigned val,  char *buf, unsigned n  );


