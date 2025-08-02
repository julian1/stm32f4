
#pragma once

// peripheral interface/abstraction
// not device/instance


#include <stdio.h>  // FILE

#include <peripheral/spi.h>


typedef struct spi_ice40_t  spi_ice40_t ;

struct spi_ice40_t
{
  spi_t  ;   // anonymous.  for composition.


  // derived functionality
  void (*rst)(spi_ice40_t *, uint8_t );
  bool (*cdone)(spi_ice40_t * );
};




static inline void spi_ice40_rst( spi_ice40_t *spi, uint8_t val)
{
  spi->rst( spi, val);
}



static inline bool spi_ice40_cdone( spi_ice40_t *spi)
{
  return spi->cdone( spi);
}



int spi_ice40_bitstream_send( spi_ice40_t *spi , FILE *f, size_t size , volatile uint32_t *system_millis);

