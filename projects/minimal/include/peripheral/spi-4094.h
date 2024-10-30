

#pragma once

#include <stddef.h> // size_t



typedef struct spi_4094_t  spi_4094_t ;
struct spi_4094_t
{
  // magic, type, size.
  uint32_t  spi;


  void (*config)(spi_4094_t *);      // call before use.

  //  pin assignment varies between instance. so give explitic functions
  void (*strobe)(spi_4094_t *, uint8_t );
  // void (*oe)(spi_4094_t *, uint8_t );  controlled by fpag. register.
} ;





// void spi_mux_4094(uint32_t spi );

uint8_t spi_4094_reg_write( spi_4094_t *spi, uint8_t v);


uint32_t spi_4094_reg_write_n( spi_4094_t *spi, const unsigned char *s, size_t n);

