
#pragma once

#include <stddef.h> // size_t


#include <peripheral/spi.h> 



typedef struct spi_t spi_t;

void spi_ad5446_write16( spi_t *spi, uint16_t val);

void spi_ad5446_port_configure( uint32_t spi);


#if 0

typedef struct spi_ad5446_t spi_ad5446_t;

struct spi_ad5446_t
{
  /*
    - require spi for the busy_wait() fucntion needed for any cs().
        and we need to call setup(), and config() on spi device.
    - we dont need a port_config()  actually config for port can be done once.
  */

  // magic, type, size.
  uint32_t  spi;


  // all of this is device specific. so belongs here.
  void (*setup)(spi_ad5446_t *);
  // void (*config)(spi_ad5446_t *);
  void (*cs)(spi_ad5446_t *, uint8_t );

} ;


/*

  Think this is wrong. 
  ok.  the configure function is a bit wrong.

  This can take a generic spi structure. on initialization.  the

  the spi - is the device.

*/



// void spi_port_configure_ad5446( spi_ad5446_t *);

void spi_ad5446_write16( spi_ad5446_t *spi, uint16_t val);

#endif
