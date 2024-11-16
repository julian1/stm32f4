
#pragma once

// peripheral interface/abstraction
// not device/instance



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




/*
  these functions are typed on spi_t and not on spi_ice40_t.
  not clear if should be placed here in this file.
  even if they are always used and associated with ice40
*/
uint32_t spi_ice40_reg_write32( spi_t *, uint8_t reg, uint32_t val);
uint32_t spi_ice40_reg_read32( spi_t *, uint8_t reg);
uint32_t spi_ice40_reg_write_n( spi_t *, uint8_t reg, const void *s, size_t n );




/*
  it is easy to switch betweetn peripherals, when using the fpga to mux.
  eg. between 4094,mdac.

  we just take care to write the mux register, first on the ice40 using cs1.
  no need for ugly attempts to nest the functionality.

  spi.config_port()                // configure spi,
  spi.set_mux_reg( REG_MUX, _4094 ).            // using ice40 cs1.

  4094.config_port()
  4094  write.          //  using ice40 cs2.

  EXTR. and the caller should be responsible. for setting it up.
    not nested delegation.

  eg. remove these types of functions,
    void spi_mux_4094(uint32_t spi )
*/


