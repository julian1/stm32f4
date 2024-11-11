
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

/*
void spi_port_cs_u202(uint32_t spi, unsigned val);
void spi_port_creset_u202(uint32_t spi, unsigned val);
*/



/* thiis is an spi device.  not a spi port abstraction
  probably should name  dev_.  or spi_.

  rename dev_spi.
  -------
  we need to associate spi id, and cs.  and a spi parameter configure function.


  - required to abstract over device - when have several devices of the same type. - ice40, 4094 chaing, dac.
      to allow writing functions, that dont care which instance they are operating on.
      otherwise we could mostly get by - calling specific functions.  eg. spi_cs_u202();

      but we cannot pass this to a low function read()/write() funcs that need to assert/deassert cs.
      eg. standard writing of registers etc. for different ice40, dacs, 4094 systems.

    eg. ice40.

  1. array of funcs
  2. safe downcasting. of opaque structures.  safely. for extended func.
  3. using filre descriptor like switch statements
*/



// dev_t or dev_spi_t.
typedef struct spi_t  spi_t ;

struct spi_t
{
  // magic, type, size.
  uint32_t  spi;

  void (*config)(spi_t *);      // wont work if using a register muxing. although we can make it.
  void (*cs)(spi_t *, uint8_t );
  void (*rst)(spi_t *, uint8_t );

  ///////////
  // ice40 peripheral specific. use ioctl?
  bool (*cdone)(spi_t * );
  // void (*oe)(spi_t *, uint8_t );
} ;


/*
  optional extra functions.  not common enough to carry around on spi_t structure.
  oe.  cdone.  perhaps rst.
  ---------
  Simplest - is probably just a (safe) downcast the interface of the device.
*/
uint32_t ioctl_spi(spi_t *t, uint32_t request, uint32_t *val);


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







