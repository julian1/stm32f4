
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



/* thiis is an spi device.  not port abstraction
  probably a better place to put it.
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



