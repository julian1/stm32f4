
#pragma once

// peripheral interface/abstraction
// not device/instance


#include <stdbool.h>
#include <stddef.h> // size_t, uint32_t



typedef struct spi_ice40_t  spi_ice40_t ;

struct spi_ice40_t
{
  // magic, type, size.
  uint32_t  spi;

  // all of this is device specific. so belongs here.
  void (*setup)(spi_ice40_t *);   // gpio
  void (*cs)(spi_ice40_t *, uint8_t );
  void (*rst)(spi_ice40_t *, uint8_t );
  bool (*cdone)(spi_ice40_t * );
};



void spi_ice40_port_configure( spi_ice40_t *spi); // for normal spi operation.

uint32_t spi_ice40_reg_write32( spi_ice40_t *, uint8_t reg, uint32_t val);
uint32_t spi_ice40_reg_read32( spi_ice40_t *, uint8_t reg);


uint32_t spi_ice40_reg_write_n( spi_ice40_t *, uint8_t reg, const void *s, size_t n );


static inline void spi_ice40_setup( spi_ice40_t *spi)
{
  spi->setup( spi);
}

static inline bool spi_ice40_cdone( spi_ice40_t *spi)
{
  return spi->cdone( spi);
}





/*
  the configure() is different for bitstream loading, versus use.
  doesn't matter for a peripheral. configure is device specific.

*/


/*
    easy to switch betweetn peripherals, using fpga to mux,   4094,mdac.

    they are just a different ice40 device.  where cs is mapped to cs2.

    we just take care to write the mux register, first on the ice40 cs1 device.
    no need for ugly nesting of spi structures or anything.

    spi.config()                // configure spi,
    spi.set_mux_reg().    // using ice40 cs1.
    4094.config()
    4094  write.          //  using ice40 cs2.

    EXTR. and the caller should be responsible. for setting it up.
      not nested delegation.

    eg. remove these types of functions,
      void spi_mux_4094(uint32_t spi )
  ----------------

*/


