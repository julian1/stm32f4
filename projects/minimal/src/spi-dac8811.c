
#include <assert.h>

#include <libopencm3/stm32/spi.h>

#include <peripheral/spi-port.h>   // spi_port_cs2_enable()


#include <spi-dac8811.h>
#include <spi-ice40.h>
#include <reg.h>






// TODO change name spi_mux_dac8811()

void mux_spi_dac8811(uint32_t spi )
{
  // EXTR. setup on the ice40 side.
  // printf("mux 4094\n");

  assert( SPI_MUX_DAC8811 == 2);   // (1<<1)

  mux_spi_ice40( spi);

  // default state - should always be to *not* to propagate spi on 4094 lines.  to avoid emi
  assert( spi_ice40_reg_read32(spi, REG_SPI_MUX ) == SPI_MUX_NONE);

  // set ice40 to mux spi to dac peripheral
  spi_ice40_reg_write32(spi, REG_SPI_MUX,  SPI_MUX_DAC8811 );

  // ensure cs disabled
  spi_port_cs1_disable( spi );  // disable, acvei lo
  spi_port_cs2_disable( spi);


  // dac8811  data is clked in on clk leading rising edge.

   spi_init_master(
    spi,
    // SPI_CR1_BAUDRATE_FPCLK_DIV_4,
    // SPI_CR1_BAUDRATE_FPCLK_DIV_16,
    SPI_CR1_BAUDRATE_FPCLK_DIV_32,
    SPI_CR1_CPOL_CLK_TO_0_WHEN_IDLE,      // park to 0/lo == positive clok edge. park to 1 == negative clk edge.
    SPI_CR1_CPHA_CLK_TRANSITION_1,    // 1 == leading edge,  2 == falling edge
    SPI_CR1_DFF_8BIT,
    SPI_CR1_MSBFIRST
  );

  spi_enable( spi );
}


// catch errors for funcs no longer used
#define spi_enable(x) WHOOT(x)
#define spi_disable(x) WHOOT(x)


/*
static uint32_t spi_xfer_register_16(uint32_t spi, uint32_t r)
{
  uint8_t a = spi_xfer( spi, (r >> 8) & 0xff  );
  uint8_t b = spi_xfer( spi, r & 0xff  );

  return (a << 8) + b; // msb first, same as dac
}
*/



static uint16_t spi_xfer_16(uint32_t spi, uint16_t val)
{
  uint8_t a = spi_xfer( spi, (val >> 8) & 0xff );  // correct reg should be the first bit that is sent.
  uint8_t b = spi_xfer( spi, val & 0xff );

  return (a << 8) + b;
}


void spi_dac8811_write16(uint32_t spi, uint16_t val)
{
  spi_port_cs2_enable(spi);

  spi_xfer_16(spi, val );

  spi_port_cs2_disable(spi);

}




