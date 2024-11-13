
#pragma once

// spi-basic
/*
  simple spi peripheral abstraction.
  can be used by different peripheral types (adc,dac)  and devices (instances).

  the spi port config . should be implemented per device. and not on this structure.
*/


typedef struct spi_t spi_t;



struct spi_t
{

  // magic, type, size.
  uint32_t  spi;

  void (*setup)(spi_t *);
  void (*cs)(spi_t *, uint8_t );
} ;


