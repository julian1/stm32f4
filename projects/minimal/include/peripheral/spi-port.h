
#pragma once

#include <stdint.h>  // uint32_t


void spi1_port_setup(void);


///////

// abstracted over spi
// hardware ness with set_nss_high etc.
// doesn't work well with more than one cs.

/*
  we cannot abstract over these.
    cs1 will be common. but that functionality will be different
    ----------

  But consumers/users of this spi - should not have to know if its spi1 or spi2.
    eg. dac code, 4094 code etc.

  so something is not right.

*/

void spi_port_cs1_enable(uint32_t );    // active lo
void spi_port_cs1_disable(uint32_t);


void spi_port_cs2_enable(uint32_t) ;
void spi_port_cs2_disable(uint32_t );



void spi1_port_interupt_setup(void);
void spi1_port_interupt_handler_set( void (*pfunc_)(void *), void *ctx);

// actually this is probably wrongly named.
// should be gpio_cdone_get().
bool spi_port_cdone_get(void);


//////////////////////

void spi_wait_ready(uint32_t spi );

//////////////

void spi2_port_setup(void);








//////////////////////////////////////////////////////////////

// these are not specific devices.  they are device abstractions.
// should be in a different file.


typedef struct spi_ice40_t  spi_ice40_t ;

struct spi_ice40_t
{
  /*
    - access to spi is required.  for the busy_wait() fucntion needed for any cs().
        and we need to call reset() and config() on spi device.
    - we dont need a port_config()  actually config for port can be done once.
  */

  // magic, type, size.
  uint32_t  spi;

  /*
  // problem is that the configure() is different for bitstream loading, versus use.
    doesn't matter. just handle it as is .   eg. out-of-band.they are like two different devices.
    */

  // all of this is device specific. so belongs here.
  void (*setup)(spi_ice40_t *);
  void (*config)(spi_ice40_t *);
  void (*cs)(spi_ice40_t *, uint8_t );

  // specific to ice40.  perhaps have a different structure
  void (*rst)(spi_ice40_t *, uint8_t );
  bool (*cdone)(spi_ice40_t * );

  // we also have the interupt.
} ;


/*
  - so we will have two create for the u202. and spi1 ice40.
  fundamental problem is the size of the structure.
  - we would like the size to be opaque.
  - but that will require a malloc().
  - Ok

*/



typedef struct spi_4094_t  spi_4094_t ;
struct spi_4094_t
{
  // magic, type, size.

  void (*config)(spi_4094_t *);      // call before use.

  //  pin assignment varies between instance. so give explitic functions
  void (*strobe)(spi_4094_t *, uint8_t );
  void (*oe)(spi_4094_t *, uint8_t );
} ;






