

#include <libopencm3/stm32/spi.h>

#include "voltage-source-2/spi.h"
#include "voltage-source-2/4094.h"

// #include "util.h"  // msleep()
#include "assert.h"
// #include "streams.h"    // usart1_printf



/*

  ------
  "The data in the shift register is transferred to the storage register when the STR input is HIGH"

  - so if strobe is high then storage register and output will be transparent. and the output will not change as atomic update.
      which is generally what we want.

  - so we only want to assert strobe high briefly - after we have clocked the correct data in.
  - the shift register always gets/holds the last mosi data. regardless of the cs/strobe
  ----------


  - there is no way to read the register contents, because the value being clocked out, is not the value that was latched in by the strobe.
    instead it is just whatever data was present from the last use of the spi lines - perhaps intended for another spi peripheral device.
  --------
 
  - this is in addition to the MISO/ output which will not go high-Z when used, meaning cannot share spi lines for other spi peripherals.
  - and we don't have the right timing characteristics with the strobe if wanted to use tri-state buffer
  - and we would need to write a dummy value, while reading.
  - to avoid would need two wire control for read/strobe. but won't work - again because the shift value last contents are not the latch contents.


*/



uint8_t spi_4094_reg_write(uint32_t spi, uint8_t v)
{
  // expect port is already configured with gpio for cs etc.

  // TODO maybe remove the enable.  not required by 4094. maybe required by stm32 spi hardware.
  spi_enable( spi );
  uint8_t val = spi_xfer(spi, v);
  spi_disable( spi );


  // briefly assert strobe/cs2
  spi_cs2_set(spi);
  for(uint32_t i = 0; i < 100; ++i)
     __asm__("nop");

  spi_cs2_clear(spi);

  return val;
}



void spi_4094_setup(uint32_t spi)
{
  // assert(spi == SPI1 || spi == SPI2);
  assert( spi == SPI2);

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
