
#pragma once

#include <stdbool.h>
#include <stddef.h> // size_t, uint32_t

// void spi_mux_ice40(uint32_t spi);   // moved from mux.h

// void spi_mux_ice40_simple(uint32_t spi);




/*
  GOOD.
    EXTR. it ought to be quite easy to use the different peripherals,   4094,mdac.

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

*/


// these are not specific devices.  they are device abstractions.
// should be in a different file.


typedef struct spi_ice40_t  spi_ice40_t ;

struct spi_ice40_t
{
  /*
    - require spi for the busy_wait() fucntion needed for any cs().
        and we need to call setup(), and config() on spi device.
    - we dont need a port_config()  actually config for port can be done once.
  */

  // magic, type, size.
  uint32_t  spi;

  /*
  // problem is that the configure() is different for bitstream loading, versus use.
    doesn't matter. just handle it as is .   eg. out-of-band.they are like two different devices.
    */

  // all of this is device specific. so belongs here.
  void (*setup)(spi_ice40_t *);   // gpio
  void (*config)(spi_ice40_t *);  // clk,pol,phase
  void (*cs)(spi_ice40_t *, uint8_t );

  // EXTR. we don't pass/set  cs2. at all. instead the 4094/dac devices own this pin.

  // specific to ice40.  perhaps should be a different structure/ or base structure
  void (*rst)(spi_ice40_t *, uint8_t );
  bool (*cdone)(spi_ice40_t * );

  // we also have the interupt.
} ;


/*
uint32_t spi_ice40_reg_write32(uint32_t spi, uint8_t reg, uint32_t val);
uint32_t spi_ice40_reg_read32(uint32_t spi, uint8_t reg);

uint32_t spi_ice40_reg_write_n(uint32_t spi, uint8_t reg, const void *s, size_t n );
*/




uint32_t spi_ice40_reg_write32( spi_ice40_t *, uint8_t reg, uint32_t val);
uint32_t spi_ice40_reg_read32( spi_ice40_t *, uint8_t reg);


uint32_t spi_ice40_reg_write_n( spi_ice40_t *, uint8_t reg, const void *s, size_t n );









/*
  - so we will have two create for the u202. and spi1 ice40.
  fundamental problem is the size of the structure.
  - we would like the size to be opaque.
  - but that will require a malloc().
  - Ok

*/





