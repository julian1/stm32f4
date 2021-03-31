
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

// #include "winbond.h"
#include "flash.h"




#define LED_PORT      GPIOE
#define LED_OUT       GPIO0



static void led_setup(void)
{
  gpio_mode_setup(LED_PORT, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, LED_OUT);
}



////////////////////////////////////////////////////////

/* Common function descriptions */
// #include "clock.h"

/* milliseconds since boot */
static volatile uint32_t system_millis;



// static void systick_setup(void)
static void systick_setup(uint32_t tick_divider)
{
  // TODO change name systick_setup().
  // TODO pass clock reload divider as argument, to localize.

  /* clock rate / 168000 to get 1mS interrupt rate */
  // systick_set_reload(168000);
  systick_set_reload(tick_divider);
  systick_set_clocksource(STK_CSR_CLKSOURCE_AHB);
  systick_counter_enable();

  /* this done last */
  systick_interrupt_enable();
}



void sys_tick_handler(void)
{
  /*
    this is an interupt context. shouldn't do any work in here
  */

  system_millis++;
}



////////////////////////////////////////////////////////

// implement critical_error_blink() msleep() and usart_printf()
// eg. rather than ioc/ dependency injection, just implement

void critical_error_blink(void)
{
  // avoid passing arguments, consuming stack.
	for (;;) {
		gpio_toggle(LED_PORT,LED_OUT);
		for(uint32_t i = 0; i < 500000; ++i)
       __asm__("nop");
	}
}

// defined in util.h
void msleep(uint32_t delay)
{
  // TODO. review. don't think this works on wrap around
  uint32_t wake = system_millis + delay;
  if(wake < delay) {

    // wait for overflow
    while (system_millis > (UINT32_MAX / 2));
  }
  while (system_millis < wake);
}




static char buf1[1000];
static char buf2[1000];

static CBuf console_in;
static CBuf console_out;




void usart_printf( const char *format, ... )
{
  // TODO rename to just printf... it's not the responsibiilty of user to know context
  // can override the write, to do flush etc.
	va_list args;

	va_start(args,format);
	internal_vprintf((void *)cBufPut, &console_out, format,args);
	va_end(args);
}

void flush( void)
{
  usart_sync_flush();
}



//////////////////////////

#define SPI_ICE40       SPI1

#define SPI_ICE40_PORT  GPIOA
#define SPI_ICE40_CLK   GPIO5
#define SPI_ICE40_CS    GPIO4
#define SPI_ICE40_MOSI  GPIO7
#define SPI_ICE40_MISO  GPIO6

// output reg.
#define SPI_ICE40_SPECIAL GPIO3




static void spi1_special_setup(void)
{
  // special
  gpio_mode_setup(SPI_ICE40_PORT, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, SPI_ICE40_SPECIAL);
  gpio_set_output_options(SPI_ICE40_PORT, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, SPI_ICE40_SPECIAL);

  gpio_set(SPI_ICE40_PORT, SPI_ICE40_SPECIAL ); // hi == off, active low...
}


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
    SPI_CR1_BAUDRATE_FPCLK_DIV_4,
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


// same flash part, but different chips,
// iceprog  0x20 0xBA 0x16 0x10 0x00 0x00 0x23 0x81 0x03 0x68 0x23 0x00 0x23 0x00 0x41 0x09 0x05 0x18 0x33 0x0F
// our code 0x20 0xBA 0x16 0x10 0x00 0x00 0x23 0x81 0x03 0x68 0x23 0x00 0x18 0x00 0x26 0x09 0x05 0x18 0x32 0x7A

static void soft_500ms_update(void)
{
  // rename update() or timer()
  // think update for consistency.

  // blink led
  gpio_toggle(LED_PORT, LED_OUT);

  ////////

  flash_reset( SPI_ICE40);
  flash_power_up(SPI_ICE40);

  flash_print_status(SPI_ICE40);
  flash_read_id( SPI_ICE40);
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


// hmmm. problem.
// the register writing using one type of clock dir and flash communication a different type of clock.
// actually it's kind of ok. we will set everything up first.
// need to unlock?



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
  cBufInit(&console_in, buf1, sizeof(buf1));
  cBufInit(&console_out, buf2, sizeof(buf2));
  usart_setup_gpio_portA();
  usart_setup(&console_in, &console_out);     // gahhh... we have to change this each time...


  //
  // spi1_ice40_setup();
  spi1_special_setup();
  spi1_flash_setup();




//  ice40_write_register1(0xff );
  // ice40_write_register1(0 );
//  ice40_write_register1( 2 );

  ////////////////////


  usart_printf("\n--------\n");
  usart_printf("starting\n");
  // usart_printf("size %d\n", sizeof(fbuf) / sizeof(float));



  loop();

	for (;;);
	return 0;
}




