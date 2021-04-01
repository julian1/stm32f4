
// cjmcu  stm32f407.
// issue. stlink doesn't appear to reset cleanly. needs sleep.



#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/spi.h>


#include "usart2.h"
#include "util.h"
#include "flash.h"
#include "common.h"




//////////////////////////

#define SPI_ICE40       SPI1

#define SPI_ICE40_PORT  GPIOA
#define SPI_ICE40_CLK   GPIO5
#define SPI_ICE40_CS    GPIO4
#define SPI_ICE40_MOSI  GPIO7
#define SPI_ICE40_MISO  GPIO6

// output reg.
// #define SPI_ICE40_SPECIAL GPIO3


static void spi1_port_setup(void)
{
  // same...
  uint16_t out = SPI_ICE40_CLK | SPI_ICE40_CS | SPI_ICE40_MOSI ; // not MISO
  uint16_t all = out | SPI_ICE40_MISO;

  // rcc_periph_clock_enable(RCC_SPI1);

  gpio_mode_setup(SPI_ICE40_PORT, GPIO_MODE_AF, GPIO_PUPD_NONE, all);
  gpio_set_af(SPI_ICE40_PORT, GPIO_AF5, all); // af 5
  gpio_set_output_options(SPI_ICE40_PORT, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, out);

}




void spi1_flash_setup(uint32_t spi)
{


  spi_init_master(
    spi,
    SPI_CR1_BAUDRATE_FPCLK_DIV_4,
    SPI_CR1_CPOL_CLK_TO_0_WHEN_IDLE,
    SPI_CR1_CPHA_CLK_TRANSITION_1,    // 1 == rising edge. difference
    SPI_CR1_DFF_8BIT,
    SPI_CR1_MSBFIRST
  );

  spi_disable_software_slave_management(spi);
  spi_enable_ss_output(spi);
}




// same flash part, but different chips,
// iceprog  0x20 0xBA 0x16 0x10 0x00 0x00 0x23 0x81 0x03 0x68 0x23 0x00 0x23 0x00 0x41 0x09 0x05 0x18 0x33 0x0F
// our code 0x20 0xBA 0x16 0x10 0x00 0x00 0x23 0x81 0x03 0x68 0x23 0x00 0x18 0x00 0x26 0x09 0x05 0x18 0x32 0x7A



void spi1_flash_get_data(uint32_t spi)
{

  flash_reset( spi);
  flash_power_up(spi);
  flash_write_enable(spi );   // needed to change SR1 from 0x00 to 0x02, after repower.
  // flash_print_status(spi);
  flash_read_id( spi);
}




static void soft_500ms_update(void)
{
  // blink led
  led_toggle();

  ////////
  uint32_t spi = SPI_ICE40;

  flash_reset( spi);
  flash_power_up(spi);
  flash_write_enable(spi );   // needed to change SR1 from 0x00 to 0x02, after repower.
  flash_print_status(spi);
  flash_read_id( spi);
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


#if 0
int main(void)
{
  // need to test high speed internal!!!

  systick_setup(16000);


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
  spi1_port_setup();
  spi1_flash_setup();


  usart_printf("\n--------\n");
  usart_printf("starting\n");


  loop();

	for (;;);
	return 0;
}

#endif
