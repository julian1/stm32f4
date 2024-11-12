

#include <peripheral/spi-port.h>



/* has to return a pointer, not take a pointer. to be opaque.  which means calling malloc() . 
    but only needs to be done once at startup.

*/


spi_ice40_t * spi2_u202_create( void);



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
  ----------

  a way to group - .
    spi port.  eg. spi-1,spi2
    the spi paraeters   phase,rising,falling
    the cs to use.
    maybe rst.
    other functions
*/



