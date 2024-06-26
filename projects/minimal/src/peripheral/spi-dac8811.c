
/*

*/
#include <assert.h>
#include <stdio.h>

#include <libopencm3/stm32/spi.h>

#include <ice40-reg.h>

#include <peripheral/spi-port.h>   // spi_port_cs2_enable()
#include <peripheral/spi-dac8811.h>
#include <peripheral/spi-ice40.h>



void spi_port_configure_dac8811(uint32_t spi)
{
  // EXTR. setup on the ice40 side.
  // printf("spi mux dac8811 \n");
  // ensure cs disabled
  spi_port_cs1_disable( spi );  // disable, acvei lo
  spi_port_cs2_disable( spi);

  // dac8811  data is clked in on clk leading rising edge.
  // ad5446 on falling edge.
  spi_init_master(
    spi,
    SPI_CR1_BAUDRATE_FPCLK_DIV_4,       // actually works over 50cm. idc cable.
    // SPI_CR1_BAUDRATE_FPCLK_DIV_16,
    // SPI_CR1_BAUDRATE_FPCLK_DIV_32,
    SPI_CR1_CPOL_CLK_TO_0_WHEN_IDLE,      // park to 0/lo == positive clok edge. park to 1 == negative clk edge.
    SPI_CR1_CPHA_CLK_TRANSITION_1,    // 1 == leading edge,  2 == falling edge
    SPI_CR1_DFF_8BIT,
    SPI_CR1_MSBFIRST
  );

  spi_enable( spi );
}



/* code is the same for dac8811 and ad5446
  but keep separate. enable bounds check for 14 bit etc.
  ad5446, requires top two bits kept low to maintain spi polarity.
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




#if 0
static void spi_write_mux_reg( uint32_t spi, uint32_t reg_spi_mux_val) 
{
  assert( reg_spi_mux_val == SPI_MUX_DAC8811 
      || reg_spi_mux_val == SPI_ISO_CS );   // (1<<1)

  spi_mux_ice40( spi);  // should not need.

  // default state - should always be to *not* to propagate spi on 4094 lines.  to avoid emi
  assert( spi_ice40_reg_read32(spi, REG_SPI_MUX ) == SPI_MUX_NONE);

  // set ice40 to mux spi to dac peripheral
  spi_ice40_reg_write32(spi, REG_SPI_MUX,  reg_spi_mux_val  /*SPI_MUX_DAC8811*/  );
}

#endif




// void spi_mux_dac(uint32_t spi );
// void spi_mux_iso_dac(uint32_t spi);



#if 0
void spi_mux_dac8811(uint32_t spi)
{
  spi_write_mux_reg( spi, SPI_MUX_DAC8811);
  spi_configure_port(spi);
}

void spi_mux_iso(uint32_t spi)
{
  spi_write_mux_reg( spi, SPI_ISO_CS);
  spi_configure_port(spi);
}
#endif


