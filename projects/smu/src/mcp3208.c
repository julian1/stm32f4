
// cjmcu  stm32f407.
// issue. stlink doesn't appear to reset cleanly. needs sleep.


#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/spi.h>


#include <stdio.h>   // printf

#include "usart.h"
#include "util.h"



#include "flash.h"    // msse_xfer_spi TODO FIX ME.
#include "mcp3208.h"




void spi_mcp3208_setup(uint32_t spi)
{

  spi_init_master(
    spi,
    SPI_CR1_BAUDRATE_FPCLK_DIV_16,   // slow... 16Mhz / 16 = 1Mhz. for 50ksps
    SPI_CR1_CPOL_CLK_TO_0_WHEN_IDLE,
    SPI_CR1_CPHA_CLK_TRANSITION_1,    // 1 == rising edge
    SPI_CR1_DFF_8BIT,
    SPI_CR1_MSBFIRST
  );

  spi_disable_software_slave_management( spi );
  spi_enable_ss_output( spi);
}


/*
  mcp3208 - dout - goes high-Z when not in use. see p20.
    https://ww1.microchip.com/downloads/en/DeviceDoc/21298e.pdf

  it's actually nice. it clocks data through the SAR and
  to the output pin, so there is no delay. eg. spi is part of the pipeline.
*/
/*

  Throughput Rate
    fSAMPLE————
    100 ksps  VDD = VREF = 5V
    50ksps   VDD = VREF = 2.7V

  clock Frequency
    fCLK————
    2.0 MHz   VDD = 5V (Note 3)
    1.0MHz   VDD = 2.7V (Note 3

    eg. at 2.7V, 1MHz / (3bytesx8bits=24clock) ~= 50ksps
*/


float spi_mcp3208_get_data(uint32_t spi, uint8_t channel)
{
  // UNUSED(channel);

  // first channel, single ended
   uint8_t data[3] = { 0b01100000 | (channel << 2 ), 0x00, 0x00 };   // eg. delay by one bit so that data aligns
                                                    // on last two bits
  spi_enable(spi);
  mpsse_xfer_spi(spi, data, 3);
  spi_disable(spi);


  // turns it into 16bit value. so lower bytes are unused.
  // but should probably be 12bit.
  // eg.  return (b1 << 4) | (b2 >> 4);
  uint16_t x = (data[1] << 8) | (data[2] ); // TODO doesn't look right 10bits not 12 bits?

  float x2 = x / 65535.0 * 3.3;             // TODO doesn't look right, should be 2^12=4096

  return x2;
}


////////////////////////


#define SPI_ICE40       SPI1

static void soft_500ms_update(void)
{
  // blink led
  led_toggle();

  ////////

  // how is this known???
  uint32_t spi = SPI_ICE40;

  // first channel, single ended
   uint8_t data[3] = { 0b01100000 , 0x00, 0x00 };   // eg. delay by one bit so that data aligns
                                                    // on last two bits
  spi_enable(spi);
  mpsse_xfer_spi(spi, data, 3);
  spi_disable(spi);

  // important - maybe clocking to fast 50kps at 2.7V.
  printf("data[0] %d\n", data[0]);      // sometimes data[0] returns a bit that should be ignored.
  printf("data[1] %d\n", data[1]);
  printf("data[2] %d\n", data[2]);

  // turns it into 16bit value. so lower bytes are unused.
  // but should probably be 12bit.
  // eg.  return (b1 << 4) | (b2 >> 4);
  uint16_t x = (data[1] << 8) | (data[2] );

  printf("x %d\n", x );

  float x2 = x / 65535.0 * 3.3;
  printf("volts %f\n", x2 );


  printf("\n");
}





/*
  OK. so we can share cs,clk,mosi.  for more than one peripheral

  but fpga doesn't want to share miso.  because more than one driver.
  so think we have to mux it.
*/

#if 0





static void loop(void)
{

  static uint32_t soft_500ms = 0;

  while(true) {

    // EXTREME - can actually call update at any time, in a yield()...
    // so long as we wrap calls with a mechanism to avoid stack reentrancy
    // led_update(); in systick.


    // pump usart queues
    usart_input_update();
    usart1_enable_output_interupt();

    // 500ms soft timer
    if( system_millis > soft_500ms) {
      soft_500ms = system_millis + 500;
      soft_500ms_update();
    }
  }
}





int main(void)
{
  // high speed internal!!!

  systick_setup(16000);


  // clocks
  rcc_periph_clock_enable(RCC_SYSCFG); // maybe required for external interupts?

  // LED
  rcc_periph_clock_enable(RCC_GPIOE);

  // USART
  rcc_periph_clock_enable(RCC_GPIOA);     // f407
  // rcc_periph_clock_enable(RCC_GPIOB); // F410
  rcc_periph_clock_enable(RCC_USART1);


  // spi / ice40
  rcc_periph_clock_enable(RCC_SPI1);

  //////////////////////
  // setup

  // led
  led_setup();

  // usart
  usart1_setup_();

  //
  // spi_mcp3208_setup();

  spi_port_setup();
  spi_mcp3208_setup();



  ////////////////////

  printf("\n--------\n");
  printf("starting\n");

  loop();

	for (;;);
	return 0;
}

#endif


