

#include <libopencm3/stm32/spi.h>

#include "spi.h"
#include "4094.h"

#include "util.h"  // msleep()
#include "assert.h"
#include "streams.h"    // usart_printf



  /*
    "The data in the shift register is transferred to the storage register when the
      STR input is HIGH"

    - so if strobe is high then storage register and output will be transparent.
    - so we only want to assert strobe high briefly - after we have clocked the data in.
    - the shift register always gets the data. regardless
    - it should only be blipped. from lo to hi back to lo against. when we want to write.
    -----------------

    It should perhaps remain configured as gpio.

    ALSO
    The '4094 only latches the data internally from its shift register into its
    output register on the falling edge of a clock pulse, when the strobe pin is
    high. Just making the strobe pin high by itself does nothing, I think.
      think this is wrong. but check.
    --------------
    
    OK. and if it is only done - onece - then then we *could* probably use CS active hi.
      
  */



uint8_t spi_4094_reg_write(uint32_t spi, uint8_t v)
{
  // change name spi_cs2_write_reg() 
  // Note. must have spi port configured to use cs2

  // usart_printf("writing spi1 using cs2 \n" );

  // TODO remove the enable.

  spi_enable( spi );
  uint8_t val = spi_xfer(spi, v);
  spi_disable( spi );


  // strobe the value
  spi1_cs2_set();
  for(uint32_t i = 0; i < 100; ++i)
     __asm__("nop");

  spi1_cs2_clear();

  return val;
}



void spi_4094_setup(uint32_t spi)
{
  assert(spi == SPI1);

  // rcc_periph_clock_enable(RCC_SPI2);
  spi_init_master(
    spi,
    SPI_CR1_BAUDRATE_FPCLK_DIV_4,
    SPI_CR1_CPOL_CLK_TO_0_WHEN_IDLE,
    // SPI_CR1_CPHA_CLK_TRANSITION_2,    // 2 == falling edge
    SPI_CR1_CPHA_CLK_TRANSITION_1,    // 1 == rising edge
    SPI_CR1_DFF_8BIT,
    SPI_CR1_MSBFIRST
  );


}
