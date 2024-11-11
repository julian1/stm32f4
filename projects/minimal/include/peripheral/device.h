

#pragma once

/*
  light abstraction -
    over raw low level libopencm3 calls.
    and for devices. so functions can work on abstractions instead of specfic instances. eg. if several fpgas, or 4094 chains. for leds, etc.

*/

/* EXTR. the way to do this - is to call config.

    and just check and check a static boolean to see - if we have already initialized.
    then we can flatten the structure.

    and have spi,

    No. because we end up writing the xfer funcs multiple times.

*/

/*
  Should be device specific.
    eg. just add the functions we need
    cs,rst,cdone.   all vary (use different pins) so make them all explitic.


*/



typedef struct led_t  led_t ;

struct led_t
{
  void (*port_setup)(led_t *);
  void (*set)(led_t *, uint8_t );
};




typedef struct spi_port_t  spi_port_t ;

struct spi_port_t
{
  // magic, type, size.
  // uint32_t  spi;     // should not be exposed. be hidden

  void (*config)(spi_port_t *);      // differs per port.eg. different gpio pins.
  void (*spi_wait_ready) ( spi_port_t *);
  uint8_t (*xfer)( spi_port_t *, uint8_t );
};


// different insta



// dev_t or dev_spi_ice40_t.
typedef struct spi_ice40_t  spi_ice40_t ;

struct spi_ice40_t
{
  // magic, type, size.
  uint32_t  spi;

  void (*config)(spi_ice40_t *);      // call before use.

  //  pin assignment varies between instance. so give explitic functions
  void (*cs)(spi_ice40_t *, uint8_t );
  void (*rst)(spi_ice40_t *, uint8_t );
  bool (*cdone)(spi_ice40_t * );
} ;




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


