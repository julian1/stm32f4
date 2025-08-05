
#pragma once

/*
  simple spi peripheral abstraction.
  can be used by different peripheral types (fpga,adc,dac)  and device/instances ( u202,u102, dac2,mcu1, fpga0 etc).
  abstracts gpio setup, and spi pol/phase configuration.


  the spi port_configure(). should be implemented per device.
  even if the function is repeated/the same for devices of the same *type* .

  ---
  could put interrupt callback on this structure also.  but not clear how useful.

*/

#include <stdbool.h>
#include <stdint.h> // uint32_t


// should be implemented per device/ or per peripheral?
// #define SPI_MAGIC   789


typedef struct spi_t spi_t;



struct spi_t
{
  // magic, type, size.

  uint32_t  magic;
  uint32_t  spi;    // controller maybe shared.  should hide/make opaque?

  void (*setup)(spi_t *);
  void (*port_configure)(spi_t *);
  void (*cs_assert)(spi_t *);
  void (*cs_deassert)(spi_t *);
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



static inline void spi_cs_assert( spi_t *spi)
{
  spi->cs_assert( spi);
}

static inline void spi_cs_deassert( spi_t *spi)
{
  spi->cs_deassert( spi);
}


