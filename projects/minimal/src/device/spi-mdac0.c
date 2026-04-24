

#include <stdio.h>
#include <assert.h>



#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/spi.h>


#include <peripheral/spi.h>


#include <device/spi-mdac0.h>
#include <device/spi-fpga0-reg.h>   // cs vec
#include <device/support.h>      // spi_wait_read()




#define UNUSED(x) ((void)(x))




static void port_configure(spi_t *spi )
{
  UNUSED( spi);

  // port already setup by shared spi controller.
}



static void controller_configure( spi_t *spi_)
{
  assert( spi_ && spi_->spi );
  uint32_t spi = spi_->spi;

  spi_reset( spi );   // critical, avoid hang

  // dac8811  data is clked in on clk leading rising edge.
  // ad5446 on falling edge.
  spi_init_master(
    spi,
    SPI_CR1_BAUDRATE_FPCLK_DIV_4,       // actually works over 50cm. idc cable.
    // SPI_CR1_BAUDRATE_FPCLK_DIV_16,
    // SPI_CR1_BAUDRATE_FPCLK_DIV_32,
    SPI_CR1_CPOL_CLK_TO_1_WHEN_IDLE,      // ad5446 reads on neg edge. ONLY DIFFERENCE.   park to 0/lo == positive clok edge. park to 1 == negative clk edge.
    SPI_CR1_CPHA_CLK_TRANSITION_1,    // 1 == leading edge,  2 == falling edge
    SPI_CR1_DFF_8BIT,
    SPI_CR1_MSBFIRST
  );

  spi_enable( spi );
}



static void cs_assert(spi_t *spi)
{
  spi_wait_ready( spi->spi);

  assert(SPI_CS_MDAC0 == 3);
  gpio_write_with_mask( GPIOC, 7, 0b111, SPI_CS_MDAC0);

}

static void cs_deassert(spi_t *spi)
{
  spi_wait_ready( spi->spi);

  gpio_write_with_mask( GPIOC, 7, 0b111, SPI_CS_DEASSERT);
}


void spi_mdac0_init( spi_t *spi)
{
  assert(spi);

  *spi = (const spi_t) {

    .spi          = SPI1,     // consider pass underlying spi by contructor.
    .port_configure =  port_configure,
    .controller_configure = controller_configure,
    .cs_assert    = cs_assert,
    .cs_deassert  = cs_deassert,
  };

}

