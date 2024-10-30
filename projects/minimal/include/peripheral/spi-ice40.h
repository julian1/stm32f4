
#pragma once

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

  /* we have the interupt to deal with.
    since it is a specific gpio pin. it should be put here.
    and we should register the handler here.
  */
} ;



void spi_ice40_port_configure( spi_ice40_t *spi); // for normal spi operation.

// void spi_ice40_port_bitstream_configure( spi_ice40_t *spi);   doesn't need to be exposed.

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
    problem is that the configure() is different for bitstream loading, versus use.
    doesn't matter. just handle it, out-of-band. as if it was a different device.
    ----------

    configure should be a freestanding function.
  */

  // void (*config)(spi_ice40_t *);  // clk,pol,phase


  // EXTR. we don't pass/set  cs2. at all. instead the 4094/dac devices own this pin.

  // specific to ice40.  perhaps should be a different structure/ or base structure



/*
  GOOD.
    EXTR. it should be easy to use the different peripherals,   4094,mdac.

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


// not a specific device, it is a device abstraction.
// allows using same generic functions

/*

  inheritance. eg. nesting a spi base class.
  is not that useful. because the way register access is done.
  is device specific.


struct spi_basic_t
{

  // magic, type, size.
  uint32_t  spi;


  // all of this is device specific. so belongs here.
  void (*setup)(spi_ice40_t *);   // gpio
  void (*config)(spi_ice40_t *);  // clk,pol,phase
  void (*cs)(spi_ice40_t *, uint8_t );
};

*/



  // spi_basic_t spi;


  /*
    - spi handle is needed here for the busy_wait() fucntion needed for any cs().
        and we need to call setup(), and config() on spi device.
    - port setup can be done once.
  */



/*
uint32_t spi_ice40_reg_write32(uint32_t spi, uint8_t reg, uint32_t val);
uint32_t spi_ice40_reg_read32(uint32_t spi, uint8_t reg);
uint32_t spi_ice40_reg_write_n(uint32_t spi, uint8_t reg, const void *s, size_t n );
*/


/*
  EXTR.

// making the port_configure() external
// means peripheals like generic spi - become abstract and can be used for several adc/dac. etc.
// and they are not devices.
//
*/


/*
  - so we will have two create for the u202. and spi1 ice40.
  fundamental problem is the size of the structure.
  - we would like the size to be opaque.
  - but that will require a malloc().
  - Ok

*/





