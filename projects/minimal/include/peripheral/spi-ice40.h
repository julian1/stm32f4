
#pragma once

// peripheral interface/abstraction
// not device/instance


#include <stdbool.h>
#include <stddef.h> // size_t, uint32_t

#include <peripheral/spi.h>


typedef struct spi_ice40_t  spi_ice40_t ;

struct spi_ice40_t
{
  spi_t  ;   // anonymous.


  // derived functionality
  void (*rst)(spi_ice40_t *, uint8_t );
  bool (*cdone)(spi_ice40_t * );
};




static inline bool spi_ice40_cdone( spi_ice40_t *spi)
{
  return spi->cdone( spi);
}




/*
  these functions are not typed on spi_ice40_t.   not clear if should go here.
  even if usage will only be associated with ice40
*/
//
uint32_t spi_ice40_reg_write32( spi_t *, uint8_t reg, uint32_t val);
uint32_t spi_ice40_reg_read32( spi_t *, uint8_t reg);
uint32_t spi_ice40_reg_write_n( spi_t *, uint8_t reg, const void *s, size_t n );






/*
    easy to switch betweetn peripherals, using fpga to mux,   4094,mdac.

    they are just a different ice40 device.  where cs is mapped to cs2.

    we just take care to write the mux register, first on the ice40 cs1 device.
    no need for ugly nesting of spi structures or anything.

    spi.config_port()                // configure spi,
    spi.set_mux_reg( REG_MUX, _4094 ).            // using ice40 cs1.

    4094.config_port()
    4094  write.          //  using ice40 cs2.

    EXTR. and the caller should be responsible. for setting it up.
      not nested delegation.

    eg. remove these types of functions,
      void spi_mux_4094(uint32_t spi )
  ----------------

*/


