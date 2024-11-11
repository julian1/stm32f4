
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


void spi2_port_setup(void);

void spi_port_cs_u202(uint32_t spi, unsigned val);
void spi_port_creset_u202(uint32_t spi, unsigned val);



/*
  could use an abstraction like the following.
  but most of the cs is device/peripheral specific.
  so except for stuff like fpga initialization
  there is not a lot of use.

  using functions - can make static and just bind the cs,rest,done opaquely.

  spi->cs(spi, 1 );     // de-assert
  EXTR.  don't have to worry about trying to use pin encoding.
  ------------------------------
*/


/*
  - abstraction for spi device. over the top of spi.
  - eg. can have multiple devices sitting on the same spi line.
  ---
  - it is nice - because can bind the cs without exposing port,pin,
  - and bind the mcu configuration. in same place.
  - cdone can be ignored.
  -----
  - stuff like cdone,oe,rst - could all be done out-of-band .   eg. like linux ioctl()

*/

typedef struct spi_t  spi_t ;

struct spi_t
{
  uint32_t  spi;

  void (*config_mcu)(spi_t *);      // wont work if using a register muxing. although we can make it.
  void (*cs)(spi_t *, uint8_t );
  void (*rst)(spi_t *, uint8_t );
  // void (*oe)(spi_t *, uint8_t );

  // ice40 peripheral specific. use ioctl?
  bool (*cdone)(spi_t * );
} ;

void spi2_u202_init( spi_t *);



/*
// or even just the following.

struct spi2_t
{
  uint32_t  spi;

  uint16_t  cs;   //pin.  using hal ?
  uint16_t  rst;   //pin.  using hal ?
  uint16_t  cdone;
} ;

*/
