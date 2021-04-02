
// cjmcu  stm32f407.
// issue. is that board/stlink doesn't appear to reset cleanly. needs sleep.


#include <libopencm3/stm32/rcc.h>

//#include <libopencm3/stm32/gpio.h>
//#include <libopencm3/cm3/nvic.h>
//#include <libopencm3/cm3/systick.h>
//#include <libopencm3/stm32/exti.h>
//#include <libopencm3/stm32/timer.h>
//#include <libopencm3/cm3/scb.h>
#include <libopencm3/stm32/spi.h>


#include <stddef.h> // size_t
//#include <math.h> // nanf
//#include <stdio.h>


#include "buffer.h"
#include "miniprintf2.h"
#include "usart2.h"
#include "util.h"

#include "common.h"







static void spi_fpga_setup(uint32_t spi)
{
  // the fpga as a spi slave.

  spi_init_master(
    spi,
    SPI_CR1_BAUDRATE_FPCLK_DIV_4,
    SPI_CR1_CPOL_CLK_TO_0_WHEN_IDLE,
    SPI_CR1_CPHA_CLK_TRANSITION_2,    // 2 == falling edge
    SPI_CR1_DFF_8BIT,
    SPI_CR1_MSBFIRST
  );

  spi_disable_software_slave_management( spi);
  spi_enable_ss_output(spi);
}


static uint32_t spi_write_register_16_here(uint32_t spi, uint32_t r)
{
  uint8_t a = spi_xfer( spi, (r >> 8) & 0xff  );
  uint8_t b = spi_xfer( spi, r & 0xff  );

  return (a << 8) + b; // msb first
}





static uint16_t spi_fpga_write1(uint32_t spi, uint32_t r)
{
  spi_special_flag_clear(spi);
  spi_enable(spi);
  uint16_t ret = spi_write_register_16_here(spi, r );
  spi_disable(spi);
  spi_special_flag_set(spi);
  return ret;
}


static uint16_t spi_fpga_write( uint32_t spi, uint8_t r, uint8_t v)
{
  uint16_t ret = spi_fpga_write1(spi, r << 8 | v );
  return ret;
}




//////////////////////////////////////////////

// REGISTER_DAC?

#define LED_REGISTER  0x07
#define LED1 (1<<0)    // D38
#define LED2 (1<<1)    // D37

#define DAC_REGISTER  0x09
#define DAC_LDAC      (1<<0)
#define DAC_RST       (1<<1)
#define DAC_UNI_BIP_A (1<<2)
#define DAC_UNI_BIP_B (1<<3)


#define SPI_MUX_REGISTER  0x08
#define SPI_MUX_ADC03     (1<<0)
#define SPI_MUX_DAC       (1<<1)
#define SPI_MUX_FLASH     (1<<2)



static void mux_fpga(uint32_t spi)
{
  spi_fpga_setup(spi);
}


static void mux_adc03(uint32_t spi)
{
  spi_fpga_setup(spi);
  uint32_t ret = spi_fpga_write(spi, SPI_MUX_REGISTER, SPI_MUX_ADC03 );


  usart_printf("-----------%d \n", ret);
  usart_printf("-----------%16b \n", ret);


  spi_mcp3208_setup(spi);
}

static void mux_flash(uint32_t spi)
{
  spi_fpga_setup(spi);
  spi_fpga_write(spi, SPI_MUX_REGISTER, SPI_MUX_FLASH );
  spi_flash_setup(spi);
}



#define SPI_ICE40       SPI1


static void soft_500ms_update(void)
{
  // blink mcu led
  led_toggle();

  ////////
  uint32_t spi = SPI_ICE40;

  // blink led
  static int count = 0;

  // setup spi1 for register control.
  usart_printf("-----------\n");

  mux_fpga(spi);
  spi_fpga_write(spi, LED_REGISTER, count++);

  mux_adc03(spi);
  float val = spi_mcp3208_get_data(spi);
  usart_printf("val %f\n", val);


  mux_flash(spi);
  spi_flash_get_data(spi);

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
      soft_500ms = system_millis + 1000;
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

  ////////////////
  spi1_port_setup();
  spi1_special_gpio_setup();


  ////////////////////


  usart_printf("\n--------\n");
  usart_printf("starting\n");
  // usart_printf("size %d\n", sizeof(fbuf) / sizeof(float));


  loop();

	for (;;);
	return 0;
}


