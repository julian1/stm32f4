
#include <libopencm3/stm32/gpio.h>    // bsrr
#include <libopencm3/stm32/spi.h>

#include <support.h>


/*
  - almost all spi is active lo.
  - only fpga config . should change name to enable/disable

  - 4094 is inverted, by fpga or 74lvc1g.

  only expose enable/disable here.  whether active hi/lo is detail.

  ---
  whether active high/lo depends on the peripheral device. and fpga may invert
  so use  _enable(), disable() . instead of clear
  following normal CS convention. active low

*/



static inline void spi_wait_until_not_busy(uint32_t spi)
{
  /*
    see,
    http://libopencm3.org/docs/latest/stm32f4/html/spi__common__all_8c_source.html#l00194
  */
  /* Wait until not busy */
  while (SPI_SR(spi) & SPI_SR_BSY);
}

static inline void spi_wait_for_transfer_finish(uint32_t spi)
{
   /* Wait for transfer finished. */
   while (!(SPI_SR(spi) & SPI_SR_TXE));


}


void spi_wait_ready(uint32_t spi )
{
/*
  see example, that also uses a loop.
  https://github.com/libopencm3/libopencm3-examples/blob/master/examples/stm32/f4/stm32f429i-discovery/lcd-dma/lcd-spi.c
*/
  // so we actually need both of these,

  spi_wait_until_not_busy (spi);

  spi_wait_for_transfer_finish( spi);   // may not need. if check not_busy, on both enable and disable.

}


void gpio_write_val(uint32_t gpioport, uint16_t gpios, bool val)
{
  // better place for this?
  // inline in include/gpio perhaps.

  /*
  // set/clear gpios bits, according to bool val
  // use CM3 style args
  // eg. gpio_write_val( GPIOA, GPIO9, 1);
  // eg.

  assert( GPIO9 == 1<< 9 );
  assert( GPIOA ==  (PERIPH_BASE_AHB1 + 0x0000) );

  */


  // BSRR == bit set/reset register.

  GPIO_BSRR( gpioport) |= gpios  << (val ? 0: 16);
}


