/*
  flashing takes ages - because it's writing the empty sectors.
  and that means it will overwrite configuration

  but we could have initial code, and then subsequent code.
  that are flashed separately.
  relink.

  Sector 0 0x0800 0000 - 0x0800 3FFF 16 Kbytes
  Sector 1 0x0800 4000 - 0x0800 7FFF 16 Kbytes
  Sector 2 0x0800 8000 - 0x0800 BFFF 16 Kbytes
  Sector 3 0x0800 C000 - 0x0800 FFFF 16 Kbytes

  Sector 4 0x0801 0000 - 0x0801 FFFF 64 Kbytes  <- where we align.
  Sector 5 0x0802 0000 - 0x0803 FFFF 128 Kbytes
  Sector 6 0x0804 0000 - 0x0805 FFFF 128 Kbytes
  Sector 7 0x0806 0000 - 0x0807 FFFF 128 Kbytes
  System memory 0x1FFF 0000 - 0x1FFF 77FF 30 Kbytes
  OTP area 0x1FFF 7800 - 0x1FFF 7A0F 528 bytes
  Option bytes 0x1FFF C000 - 0x1FFF C00F 16 byte

  file:///tmp/mozilla_me0/DM00119316-.pdf
  https://medium.com/theteammavericks/programming-flash-rom-in-stm32-f5b7d6dcba4f

  --------
  libopencm3 flash code for f4 here,

  Note there are multiple flash files, linked in the libopencm3 Makefile for f4

  OBJS    += flash.o flash_common_all.o flash_common_f.o flash_common_f24.o
    eg.

    flash_wait_for_last_operation() is defined here ../../libopencm3/lib/stm32/f4/flash.c
                                    but used here, ../../libopencm3/lib/stm32/common/flash_common_f24.c

    flash_unlock(void)  is defined in
                                      ../../libopencm3/lib/stm32/common/flash_common_f.c

  flash is read by simply reading the memory address.


  ---------------
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


#include <libopencm3/stm32/flash.h>

#include <stddef.h> // size_t
//#include <stdio.h>
#include <string.h>   // memset
#include <assert.h>   // directs to local assert.h



#include "cbuffer.h"
#include "usart2.h"
#include "streams.h"
#include "util.h"
// #include "assert.h"




#define CMD_BUF_SZ  100


typedef struct app_t
{
  CBuf console_in;
  CBuf console_out;

  bool data_ready ;


  // FBuf      measure_rundown;


  /*
    the command processor (called in deep instack) can configure this. ok.
    continuation function.
    so we can set this anywhere (eg. in command processor). and control will pass.
    and we can test this.
    - allows a stack to run to completion even if early termination -  to clean up resources.
  */
  void *continuation_ctx;
  void (*continuation_f)(void *);

  // need to initialize
  char  cmd_buf[CMD_BUF_SZ ];
  unsigned cmd_buf_i;


} app_t;








static void update_console_cmd(app_t *app)
{

  /* using peekLast() like this wont work
     since it could miss a character.
    we kind of need to transfer all chars to another buffer. and test for '\n'.
    -----
    No. the easiest way is to handle the interupt character. directly...
    actually no. better to handle in main loop..

  */


  while( ! cBufisEmpty(&app->console_in)) {

    // got a character
    int32_t ch = cBufPop(&app->console_in);
    assert(ch >= 0);

    if(ch != '\r' && app->cmd_buf_i < CMD_BUF_SZ - 1) {


      // push_char(app->cmd_buf, &app->cmd_buf_i, ch );

      // push onto a vector? or array?
      app->cmd_buf[ app->cmd_buf_i++ ] = ch;
      // app->cmd_buf[ app->cmd_buf_i ] = 0;

    }  else {
      // we got a command

      app->cmd_buf[ app->cmd_buf_i ]  = 0;

      usart_printf("got command '%s'\n", app->cmd_buf );
      usart_printf("> " );


      if(strcmp(app->cmd_buf , "write") == 0) {

        usart_printf("writing flash\n");
        flash_unlock();
        usart_printf("erasing sector \n");

        /*
          A sector must:w first be fully erased before attempting to program it.
          [in]  sector  (0 - 11 for some parts, 0-23 on others)
          program_size  0 (8-bit), 1 (16-bit), 2 (32-bit), 3 (64-bit)
        */

        flash_erase_sector(2 , 0 );
        unsigned char buf[] = "whoot";
        flash_program(0x08008000 , buf, sizeof(buf) );
        usart_printf("lock\n");
        flash_lock();

      }

      if(strcmp(app->cmd_buf , "read") == 0) {

        char *s = (char *) 0x08008000;

        printf( "flash char is '%c'\n", *s);

        // expect null terminator
        if(*s == 'w')
          printf( "string is '%s'\n", s );
      }


      if(strcmp(app->cmd_buf , "whoot") == 0) {
        // So.  how do we handle changing modes????

        // if we are in separate loops for calibration, permutation , etc.
        // how do we cancel, break out. and start another?
        // coroutines. not really an answer.

        // this function can be tested and be used to return early.
        // or set a flag. like cancel current command/action.

        // also - sometimes we want to change something - without setting the continuation.

        app->continuation_ctx = 0;
      }


      app->cmd_buf_i = 0;
    }


  }
}




static void loop(app_t *app)
{
  usart_printf("=========\n");
  usart_printf("main loop\n");
  usart_printf("> ");

 static uint32_t soft_500ms = 0;


  // NO. recompiling - overwrites the flash.

  while(true) {

    update_console_cmd(app);
    // usart_output_update(); // shouldn't be necessary, now pumped by interupts.

    // 500ms soft timer. should handle wrap around
    if( (system_millis - soft_500ms) > 500) {
      soft_500ms += 500;

      //
      led_toggle();
    }
  }
}




static char buf_console_in[1000];
static char buf_console_out[1000];

// static float buf_rundown[6];

static app_t app;


int main(void)
{
  // hsi setup high speed internal!!!
  // TODO. not using.

  // rcc_clock_setup_pll(&rcc_hse_8mhz_3v3[RCC_CLOCK_3V3_84MHZ] );  // stm32f411  upto 100MHz.
  rcc_clock_setup_pll(&rcc_hse_8mhz_3v3[RCC_CLOCK_3V3_168MHZ]);


  // clocks
  rcc_periph_clock_enable(RCC_SYSCFG); // maybe required for external interupts?

  // LED
  rcc_periph_clock_enable(RCC_GPIOA); // f410 led.

  // USART
  // rcc_periph_clock_enable(RCC_GPIOA);     // f407
  rcc_periph_clock_enable(RCC_GPIOE);     // f407
  rcc_periph_clock_enable(RCC_GPIOB); // F410
  rcc_periph_clock_enable(RCC_USART1);

  //////////////////////
  // setup

  // 16MHz. from hsi datasheet.
//  systick_setup(16000);
//  systick_setup(84000);
  systick_setup(168000);

  // led
  led_setup();


  memset(&app, 0, sizeof(app_t));

  ///////
  // uart/console
  cBufInit(&app.console_in,  buf_console_in, sizeof(buf_console_in));
  cBufInit(&app.console_out, buf_console_out, sizeof(buf_console_out));


  // buffer of measurements.
  // fBufInit(&app.measure_rundown, buf_rundown, ARRAY_SIZE(buf_rundown));


  //////////////
  // uart
  // usart_setup_gpio_portA();
  usart_setup_gpio_portB();

  usart_set_buffers(&app.console_in, &app.console_out);

  // standard streams for printf, fprintf, putc.
  init_std_streams(  &app.console_out );


  usart_printf("\n--------\n");
  usart_printf("addr main() %p\n", main );
  // assert(0);


  usart_flush();

  loop(&app);
}



