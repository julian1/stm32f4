
#pragma once

/*
  simple spi peripheral abstraction.
  can be used by different peripheral types (fpga,adc,dac)  and device/instances ( u202,u102, dac2,mcu1 etc).
  abstracts gpio setup, and spi pol/phase configuration.
  could also abstract the spi by typing spi_xfer() on spi_t. but we dont bother
  eg. uint8_t spi_xfer(spi_t *, uint8_t ); but we don't bother

  the spi port_configure(). should be implemented per device.
  even if the function is repeated/the same for devices of the same *type* .

  ---
  could put interrupt callback on this structure also.  but not clear how useful.

*/

#include <stdbool.h>
#include <stdint.h> // uint32_t
#include <stddef.h> // size_t


typedef struct spi_t spi_t;



struct spi_t
{

  // magic, type, size.
  uint32_t  spi;    // controller. maybe shared.  should hide/make opaque?

  void (*setup)(spi_t *);                     // cs interupt gpio
  void (*port_configure)(spi_t *);
  void (*cs)(spi_t *, uint8_t );

  // void (*cs_assert)(spi_t *, uint8_t );  // better  if using the mcu line encoding/
  // void (*cs_deassert)(spi_t *, uint8_t );
};


static inline void spi_setup( spi_t *spi)
{
  // assert(spi);
  spi->setup( spi);
}

static inline void spi_port_configure( spi_t *spi)
{
  // assert(spi);
  spi->port_configure( spi);
}

static inline void spi_cs( spi_t *spi, uint8_t val)
{
  // should not really
  spi->cs( spi, val);
}

static inline void spi_cs_assert( spi_t *spi)
{
  // placeholder, until refactor
  spi->cs( spi, 0);
}

static inline void spi_cs_deassert( spi_t *spi)
{
  spi->cs( spi, 1);
}


