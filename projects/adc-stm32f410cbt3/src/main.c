/*
  serial,
  rlwrap -a picocom -b 115200 /dev/ttyUSB0

  screen
  openocd -f openocd.cfg
  rlwrap nc localhost 4444  # in new window

   reset halt ; flash write_image erase unlock ../blinky-stm32f410cbt3/main.elf; sleep 1500; reset run

  *********
  with gnu sprintf, and floating point code, this still fits in 27k, tested by editing f410.ld.  good!!
  *********

*/


#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>

// #include <libopencm3/cm3/nvic.h>
// #include <libopencm3/cm3/systick.h>

#include <libopencm3/stm32/spi.h>   // SPI1


#include <stddef.h> // size_t
//#include <math.h> // nanf
//#include <stdio.h>
#include <string.h>   // memset


#include "cbuffer.h"
#include "usart2.h"
#include "util.h"
#include "assert.h"

#include "spi1.h"
#include "ice40.h"



typedef struct app_t
{
  CBuf console_in;
  CBuf console_out;


} app_t;


static void update_console_cmd(app_t *app)
{

  if( !cBufisEmpty(&app->console_in) && cBufPeekLast(&app->console_in) == '\r') {

    // usart_printf("got CR\n");

    // we got a carriage return
    static char tmp[1000];

    size_t nn = cBufCount(&app->console_in);
    size_t n = cBufCopyString(&app->console_in, tmp, ARRAY_SIZE(tmp));
    ASSERT(n <= sizeof(tmp));
    ASSERT(tmp[n - 1] == 0);
    ASSERT( nn == n - 1);

    // chop off the CR to make easier to print
    ASSERT(((int) n) - 2 >= 0);
    tmp[n - 2] = 0;

    // TODO first char 'g' gets omitted/chopped here, why? CR handling?
    usart_printf("got command '%s'\n", tmp);

    // process_cmd(app, tmp);
  }
}




static void loop(app_t *app)
{
  /*
    loop() subsumes update()
  */

  // TODO move to app_t structure?.
  static uint32_t soft_500ms = 0;

  while(true) {

    update_console_cmd(app);

    // usart_output_update(); // shouldn't be necessary


    // 500ms soft timer. should handle wrap around
    if( (system_millis - soft_500ms) > 500) {
      soft_500ms += 500;

      //
      led_toggle();

      static int count = 0;
      uint32_t ret ; 


      // ok. seems to work. 
      usart_printf("here\n");

      spi_reg_write_24(SPI1, 7, 0xffffff );   
      ret = spi_reg_write_24(SPI1, 7, count );   
      ASSERT(ret == 0xffffff);


      spi_reg_write_24(SPI1, 7, 0xff00ff );   
      ret = spi_reg_write_24(SPI1, 7, count );   
      ASSERT(ret == 0xff00ff);

      spi_reg_write_24(SPI1, 7, 126371 );   
      ret = spi_reg_write_24(SPI1, 7, count );   
      ASSERT(ret == 126371 );

      // ok. 

#if 1
      ret = spi_reg_write_24(SPI1, 7, count );   
      usart_printf("here %u  %u\n", count ,  ret);
      ++count;
#endif

    }

  }
}




static char buf_console_in[1000];
static char buf_console_out[1000];


static app_t app;


int main(void)
{
  // hsi setup high speed internal!!!
  // TODO. not using.


  // clocks
  rcc_periph_clock_enable(RCC_SYSCFG); // maybe required for external interupts?

  // LED
  rcc_periph_clock_enable(RCC_GPIOA); // f410 led.

  // USART
  // rcc_periph_clock_enable(RCC_GPIOA);     // f407
  rcc_periph_clock_enable(RCC_GPIOB); // F410
  rcc_periph_clock_enable(RCC_USART1);

  // spi / ice40
  rcc_periph_clock_enable(RCC_SPI1);



  //////////////////////
  // setup

  // 16MHz. from hsi datasheet.
  systick_setup(16000);

  // led
  led_setup();


  memset(&app, 0, sizeof(app_t));

  ///////
  // uart/console
  cBufInit(&app.console_in,  buf_console_in, sizeof(buf_console_in));
  cBufInit(&app.console_out, buf_console_out, sizeof(buf_console_out));

  //////////////
  // uart
  // usart_setup_gpio_portA();
  usart_setup_gpio_portB();

  usart_set_buffers(&app.console_in, &app.console_out);

  // setup print
  // usart_printf_set_buffer() 
  usart_printf_init(&app.console_out);


  ////////////////
  // spi1/ice40
  spi1_port_setup();
  spi1_special_gpio_setup();
  // adc interupt...
  // spi1_interupt_gpio_setup( (void (*) (void *))spi1_interupt, &app);

  
  spi_ice40_setup(SPI1);




  usart_printf("\n--------\n");
  usart_printf("starting loop\n");
  usart_printf("sizeof bool   %u\n", sizeof(bool));
  usart_printf("sizeof float  %u\n", sizeof(float));
  usart_printf("sizeof double %u\n", sizeof(double));

  ASSERT(1 == 2);

  usart_printf("a float formatted %g\n", 123.456f );

  // state_change(&app, STATE_FIRST );

  loop(&app);
}


