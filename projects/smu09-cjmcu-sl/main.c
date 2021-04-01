
// cjmcu  stm32f407.
// issue. is that board/stlink doesn't appear to reset cleanly. needs sleep.


#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>

#include <libopencm3/cm3/nvic.h>
#include <libopencm3/cm3/systick.h>

#include <libopencm3/stm32/exti.h>

#include <libopencm3/stm32/timer.h>

#include <libopencm3/stm32/spi.h>

// #include <libopencm3/cm3/scb.h>

#include <stddef.h> // size_t
//#include <math.h> // nanf
//#include <stdio.h>


#include "buffer.h"
#include "miniprintf2.h"
#include "usart2.h"
#include "util.h"

#include "common.h"







//////////////////////////

#define SPI_ICE40       SPI1

#define SPI_ICE40_PORT  GPIOA
#define SPI_ICE40_CLK   GPIO5
#define SPI_ICE40_CS    GPIO4
#define SPI_ICE40_MOSI  GPIO7
#define SPI_ICE40_MISO  GPIO6

// output reg.
#define SPI_ICE40_SPECIAL GPIO3


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




static void spi1_special_setup(void)
{
/*
  uint16_t out = SPI_ICE40_CLK | SPI_ICE40_CS | SPI_ICE40_MOSI ; // not MISO 
  uint16_t all = out |  SPI_ICE40_MISO;


  // rcc_periph_clock_enable(RCC_SPI1);

  gpio_mode_setup(SPI_ICE40_PORT, GPIO_MODE_AF, GPIO_PUPD_NONE, all);
  gpio_set_af(SPI_ICE40_PORT, GPIO_AF5, all); // af 5
  gpio_set_output_options(SPI_ICE40_PORT, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, out);
*/

  spi_init_master(
    SPI_ICE40,
    SPI_CR1_BAUDRATE_FPCLK_DIV_4,     // SPI_CR1_BAUDRATE_FPCLK_DIV_256,
    SPI_CR1_CPOL_CLK_TO_0_WHEN_IDLE,  // SPI_CR1_CPOL_CLK_TO_1_WHEN_IDLE ,
    SPI_CR1_CPHA_CLK_TRANSITION_2,    // 2 == falling edge (from dac8734 doc.
    SPI_CR1_DFF_8BIT,
    SPI_CR1_MSBFIRST                  // SPI_CR1_LSBFIRST
  );

  spi_disable_software_slave_management( SPI_ICE40);
  spi_enable_ss_output(SPI_ICE40);

  ////////////


  // special 
  gpio_mode_setup(SPI_ICE40_PORT, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, SPI_ICE40_SPECIAL);
  gpio_set_output_options(SPI_ICE40_PORT, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, SPI_ICE40_SPECIAL);
  
  gpio_set(SPI_ICE40_PORT, SPI_ICE40_SPECIAL ); // hi == off, active low...
}


#if 0
// special register...

static uint32_t spi1_write_register_24(uint32_t spi, uint32_t r)
{
  uint8_t a = spi_xfer( spi, (r >> 16) & 0xff );
  uint8_t b = spi_xfer( spi, (r >> 8) & 0xff  );
  uint8_t c = spi_xfer( spi, r & 0xff  );

  // REVIEW
  // return (a << 16) + (b << 8) + c;
  return (c << 16) + (b << 8) + a;      // msb last... seems weird.
}
#endif

static uint32_t spi1_write_register_16(uint32_t spi, uint32_t r)
{
  uint8_t a = spi_xfer( spi, (r >> 8) & 0xff  );
  uint8_t b = spi_xfer( spi, r & 0xff  );

  // REVIEW
  // return (a << 16) + (b << 8) + c;
  return (b << 8) + a;      // msb last... seems weird.
}




static uint32_t spi1_write_special1(uint32_t r)
{

  gpio_clear(SPI_ICE40_PORT, SPI_ICE40_SPECIAL ); // assert special, active low...
  spi_enable( SPI_ICE40 );
  uint8_t ret = spi1_write_register_16(SPI_ICE40, r );
  spi_disable( SPI_ICE40 );
  gpio_set(SPI_ICE40_PORT, SPI_ICE40_SPECIAL ); // deassert special, active low...
  return ret;
}


static uint32_t spi1_write_special( uint8_t r, uint8_t v) 
{
  uint8_t ret = spi1_write_special1(r << 8 | v );
  return ret;
}


/*
// 
static uint32_t ice40_write_peripheral(uint32_t r)
{
  // gpio_set(SPI_ICE40_PORT, SPI_ICE40_SPECIAL ); // deassert special...
  spi_enable( SPI_ICE40 );
  uint8_t ret = spi1_write_register_16(SPI_ICE40, r );
  spi_disable( SPI_ICE40 );
  return ret;
}
*/



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


#if 0

    // tests
    // gpio_toggle(SPI_ICE40_PORT, SPI_ICE40_CLK);
    // gpio_toggle(SPI_ICE40_PORT, SPI_ICE40_CS);
    // gpio_toggle(SPI_ICE40_PORT, SPI_ICE40_MOSI );
    // gpio_toggle(SPI_ICE40_PORT, SPI_ICE40_SPECIAL );

  
    if(count % 2 == 0) 
      spi1_write_special( DAC_REGISTER,  DAC_UNI_BIP_A);
    else
      spi1_write_special( DAC_REGISTER,  ~DAC_UNI_BIP_A );


#endif





static void soft_500ms_update(void)
{
  // blink mcu led
  led_toggle();

  ////////
  // uint32_t spi = SPI_ICE40;

  // blink led
  static int count = 0;

  // setup spi1 for register control.

  usart_printf("-----------\n");

  /////////////////////////
  spi1_special_setup();
  // do register
  spi1_write_special( LED_REGISTER, count++ );
  // fpga mux adc03.
  spi1_write_special( SPI_MUX_REGISTER, SPI_MUX_ADC03 );   


  // setup spi1 for mcp3208
  spi1_mcp3208_setup();
  float val = spi1_mcp3208_get_data();
  usart_printf("val %f\n", val);

  /////////////////////////
  // setup spi1 for register control.
  spi1_special_setup();
  // do register
  spi1_write_special( SPI_MUX_REGISTER, SPI_MUX_FLASH );   


  // setup spi to read flash
  spi1_flash_setup();
  spi1_flash_get_data(); 



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


// hmmm. problem.
// the register writing using one type of clock dir and flash communication a different type of clock.
// actually it's kind of ok. we will set everything up first.


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


  spi1_port_setup();
  spi1_special_setup();


  ////////////////////


  usart_printf("\n--------\n");
  usart_printf("starting\n");
  // usart_printf("size %d\n", sizeof(fbuf) / sizeof(float));


  loop();

	for (;;);
	return 0;
}




