

#include <libopencm3/stm32/spi.h>

#include "spi.h"
#include "4094.h"

// #include "util.h"  // msleep()
#include "assert.h"
// #include "streams.h"    // usart1_printf



/*
  "The data in the shift register is transferred to the storage register when the
    STR input is HIGH"

  - so if strobe is high then storage register and output will be transparent.
  - so we only want to assert strobe high briefly - after we have clocked the data in.
  - the shift register always gets the mosi data. regardless of the cs/strobe

*/



uint8_t spi_4094_reg_write(uint32_t spi, uint8_t v)
{
  // expect port is configured with gpio for cs etc.

  // TODO maybe remove the enable.
  spi_enable( spi );
  uint8_t val = spi_xfer(spi, v);
  spi_disable( spi );


  // briefly assert strobe/cs2
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
