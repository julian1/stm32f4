

#pragma once

/*
  very light abstraction -
    so functions can work on abstractions instead of specfic instances. eg. if several fpgas, or 4094 chains. for leds, etc.
  ------------

  only enough to work with different devices - not caring which instance.
  not over libopencm3 calls.
  -----
  just an adapater.


*/



typedef struct led_t  led_t ;

struct led_t
{
  void (*port_setup)(led_t *);
  void (*set)(led_t *, uint8_t );
};


#if 0

// problem with this abstraction - is that specific spi device, needs access to underlying spi device. eg. SPI1. etc.
  to reconfigure phase/pol.

typedef struct spi_port_t  spi_port_t ;

struct spi_port_t
{
  // magic, type, size.
  uint32_t  spi;                      // should not be exposed. be hidden
                                      // config() of devices - needs the actual spi number.

  void (*config)(spi_port_t *);      // differs per port.eg. different gpio pins.

  // these can actually be
  ///void (*spi_wait_ready) ( spi_port_t *);
  // uint8_t (*xfer)( spi_port_t *, uint8_t );
};


#endif

// different insta

/*
  // double issue. is that the bitstream loading. maybe different port configuration from use.
  // or just



*/

typedef struct spi_ice40_t  spi_ice40_t ;

struct spi_ice40_t
{
  /*
    - access to spi is required.  for the busy_wait() fucntion needed for any cs().
        and we will call reset() and configure() on spi device.
    - we dont need a port_config()  actually config for port can be done once.
  */

  // magic, type, size.
  uint32_t  spi;

  /*
  // void (*port_config)(spi_ice40_t *);      // issue is that port is shared.
  // void (*config)(spi_ice40_t *);      // spi phase,edge.  needs access to underlying spi.
  // problem is that the configure() is different for bitstream loading, versus use. they are like two different devices.
    */

  // gpio pin assignment varies between instance. so need explitic functions
  void (*cs)(spi_ice40_t *, uint8_t );
  void (*rst)(spi_ice40_t *, uint8_t );
  bool (*cdone)(spi_ice40_t * );
} ;


/*
  - so we will have two create for the u202. and spi1 ice40.
  fundamental problem is the size of the structure.
  - we would like the size to be opaque.
  - but that will require a malloc().

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





/*
  optional extra functions.  not common enough to carry around on spi_t structure.
  oe.  cdone.  perhaps rst.
  ---------
  Simplest - is probably just a (safe) downcast the interface of the device.
*/
// uint32_t ioctl_spi(spi_t *t, uint32_t request, uint32_t *val);

/*
static inline void rst( spi_t *s, uint8_t val)
{
  // convenience.
  s->rst( s, val);
};

static inline void cs( spi_t *s, uint8_t val)
{
  // convenience.
  s->cs( s, val);
};

*/


