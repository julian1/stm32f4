
#pragma once

// spi-basic
/*
  simple spi peripheral abstraction.
  can be used by different peripheral types (fpga,adc,dac)  and devices (instances, u202,u102 etc).

  the spi port_configure(). should be implemented per device.
  even if it is the same - for different *types*.

*/


typedef struct spi_t spi_t;



struct spi_t
{

  // magic, type, size.
  uint32_t  spi;    // controller. maybe shared.

  void (*setup)(spi_t *);
  void (*port_configure)(spi_t *);
  void (*cs)(spi_t *, uint8_t );
} ;


static inline void spi_setup( spi_t *spi)
{
  // assert(spi);
  spi->setup( spi);
}

static inline void spi_cs( spi_t *spi, uint8_t val)
{
  spi->cs( spi, val);
}


static inline void spi_port_configure( spi_t *spi)
{
  assert(spi);
  spi->port_configure( spi);
}


