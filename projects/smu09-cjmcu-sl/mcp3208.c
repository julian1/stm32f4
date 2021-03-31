
// cjmcu  stm32f407.
// issue. stlink doesn't appear to reset cleanly. needs sleep.


#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/spi.h>



#include "usart2.h"
#include "util.h"
// #include "flash.h"

#include "flash.h"    // msse_xfer_spi



//////////////////////////

#define SPI_ICE40       SPI1

#define SPI_ICE40_PORT  GPIOA
#define SPI_ICE40_CLK   GPIO5
#define SPI_ICE40_CS    GPIO4
#define SPI_ICE40_MOSI  GPIO7
#define SPI_ICE40_MISO  GPIO6

// output reg.
#define SPI_ICE40_SPECIAL GPIO3




static void spi1_flash_setup(void)
{
  // same...
  uint16_t out = SPI_ICE40_CLK | SPI_ICE40_CS | SPI_ICE40_MOSI ; // not MISO
  uint16_t all = out | SPI_ICE40_MISO;

  // rcc_periph_clock_enable(RCC_SPI1);

  gpio_mode_setup(SPI_ICE40_PORT, GPIO_MODE_AF, GPIO_PUPD_NONE, all);
  gpio_set_af(SPI_ICE40_PORT, GPIO_AF5, all); // af 5
  gpio_set_output_options(SPI_ICE40_PORT, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, out);


  spi_init_master(
    SPI_ICE40,
    // SPI_CR1_BAUDRATE_FPCLK_DIV_4,
    SPI_CR1_BAUDRATE_FPCLK_DIV_16,   // slow
    // SPI_CR1_BAUDRATE_FPCLK_DIV_256,   // slow
    SPI_CR1_CPOL_CLK_TO_0_WHEN_IDLE,
    SPI_CR1_CPHA_CLK_TRANSITION_1,    // 1 == rising edge. difference
    SPI_CR1_DFF_8BIT,
    SPI_CR1_MSBFIRST
  );

  spi_disable_software_slave_management( SPI_ICE40);
  spi_enable_ss_output(SPI_ICE40);
}

	
/*
  mcp3208 - dout - goes high-Z when not in use. see p20.
    https://ww1.microchip.com/downloads/en/DeviceDoc/21298e.pdf

  it's actually nice. it clocks data through the SAR and 
  to the output pin, so there is no delay. eg. spi is part of the pipeline.

  actually may have to send a dummy 0 first byte. then the 4 bit input. 
  in order to clock data out after the 4 bit value..
  ----
  or can continuously clock 2 bytes - being - aware of the 1 byte offset
  in return value.
just 

  50 ksps max. sampling rate at VDD = 2.7V

*/

//////////////////////////////////////////////


static void soft_500ms_update(void)
{
  // blink led
  led_toggle();

  ////////

  uint32_t spi = SPI_ICE40; 

  // first channel, single ended
  // uint8_t data[2] = { 0b1000 << 4 , 0x00 };
  // uint8_t data[3] = { 0b10000000 , 0x00, 0x00 };
  uint8_t data[3] = { 0b01000000 , 0x00, 0x00 };
  // uint8_t data[3] = {  0x00, 0b10000000 , 0x00 };

//      uint8_t addr = 0b01100000 | ((pin & 0b111) << 2);

  spi_enable(spi);
  mpsse_xfer_spi(spi, data, 3);
  spi_disable(spi);

  // important - maybe clocking to fast 50kps at 2.7V.   
  
  usart_printf("\n");
  usart_printf("data[0] %d\n", data[0]);
  usart_printf("data[1] %d\n", data[1]);
  usart_printf("data[2] %d\n", data[2]);
}


static void loop(void)
{

  static uint32_t soft_500ms = 0;

  while(true) {

    // EXTREME - can actually call update at any time, in a yield()...
    // so long as we wrap calls with a mechanism to avoid stack reentrancy
    // led_update(); in systick.


    // pump usart queues
    usart_input_update();
    usart_output_update();


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
  usart_setup_();

  //
  spi1_flash_setup();

  ////////////////////

  usart_printf("\n--------\n");
  usart_printf("starting\n");

  loop();

	for (;;);
	return 0;
}




