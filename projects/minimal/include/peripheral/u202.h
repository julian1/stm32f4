
/* specific device/ instance.

*/
#include <peripheral/spi-port.h>



/*
  func has to return a pointer, not take a pointer.
  to be opaque.  which requires calling malloc() .
  but only needs to be done once at startup.
  -  otherwise would have to instantiate in main.c

*/


typedef struct spi_ice40_t spi_ice40_t;

spi_ice40_t * spi2_u202_create( void);



