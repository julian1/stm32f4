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
#include <stdio.h>
#include <string.h>   // memset
#include <assert.h>   // directs to local assert.h



#include "util.h"
#include "cbuffer.h"
#include "usart2.h"
#include "streams.h"
#include "cstring.h"
// #include "assert.h"






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


  CString    command;


} app_t;




static void flash_write(void)
{
  // put in a command
  usart_printf("writing flash\n");
  flash_unlock();
  usart_printf("erasing sector \n");

  /*
    A sector must:w first be fully erased before attempting to program it.
    [in]  sector  (0 - 11 for some parts, 0-23 on others)
    program_size  0 (8-bit), 1 (16-bit), 2 (32-bit), 3 (64-bit)
  */

  /*
    it is unhnecessary to always erase. instead erase once, then use COW strategy, 
    and seek forward until hit 0xff bytes, where can write updated data.
  */
  flash_erase_sector(2 , 0 );
  unsigned char buf[] = "whoot";

  usart_printf("writing\n");
  flash_program(0x08008000 , buf, sizeof(buf) );
  flash_lock();
  usart_printf("done\n");

}


static void flash_read(void)
{
  char *s = (char *) 0x08008000;
  printf( "flash char is '%c' %u\n", *s, *s);
  // expect null terminator
  if(*s == 'w')
    printf( "string is '%s'\n", s );

}


/*
  an advantage of a continuation loop.
  is that we can restrict the menu processing options.

  ----------
  - what about setting the exit continuation loop as well?
  - eg. the cancel continuation - to return to the same point.
  -----

  alterantively we have one loop - and instead pass measurement obs abstract function on every read.
  and that can change.
  or reset itself.

  on_data( app, localctx. counts )

  so the main command process - would set this, and could cancel it etc.
  -----
  kep issue is resource cleanup - eg. predictable cleanup - is easy with a linear function.
  linear function can control all phases of what it wants to do.

*/

static void loop2(app_t *app);
static void loop1(app_t *app);


static void update_console_cmd(app_t *app)
{


  while( ! cBufisEmpty(&app->console_in)) {

    // got a character
    int32_t ch = cBufPop(&app->console_in);
    assert(ch >= 0);


    if(ch != '\r' && cStringCount(&app->command) < cStringReserve(&app->command) ) {
      // normal character      
      cStringPush(&app->command, ch);
      // echo to output. required for minicom.
      putchar( ch);

    }  else {


      // newline or overflow
      putchar('\n');

      char *cmd = cStringPtr(&app->command);


      // flash write
      if(strcmp(cmd , "write") == 0) {
        flash_write();
      }
      // flash read
      else if(strcmp(cmd , "read") == 0) {
        flash_read();
      }

      else if(strcmp(cmd , "loop1") == 0) {
        app->continuation_ctx = app;
        app->continuation_f = (void (*)(void *)) loop1;
      }

      else if(strcmp(cmd , "loop2") == 0) {
        app->continuation_ctx = app;
        app->continuation_f = (void (*)(void *)) loop2;
      }

      // unknown command
      else {

        printf( "unknown command '%s'\n", cmd );
      }

      // reset buffer
      cStringClear( &app->command);

      // issue new command prompt
      usart_printf("> ");

    }
  }
}




#if 0
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
#endif


static void loop2(app_t *app)
{
  usart_printf("=========\n");
  usart_printf("loop 2\n");
  usart_printf("> ");

 static uint32_t soft_500ms = 0;

  while(true) {

    update_console_cmd(app);

    // 500ms soft timer. should handle wrap around
    if( (system_millis - soft_500ms) > 500) {
      soft_500ms += 500;

      //
      led_toggle();
    }

    if(app->continuation_f)
      return;


  }
}


/*
  ok. there's something wrong with this. the stack isn't retreating.
  because its ot a proper continuation.
*/


static void loop1(app_t *app)
{
  /* 
    OK. this worked, to reduce the stack by 100k.
    stack grows down. bottom toward 0x20000000
  */
  volatile char ch[100000 ] ;
  volatile char *x = &ch[100000 - 1 ]; 
  *x = 123;

  usart_printf("=========\n");
  usart_printf("loop1\n");

  print_stack_pointer();
  usart_printf("> ");

 static uint32_t soft_500ms = 0;

  while(true) {

    update_console_cmd(app);

    // 500ms soft timer. should handle wrap around
    if( (system_millis - soft_500ms) > 500) {
      soft_500ms += 500;
      //
      led_toggle();
    }


    if(app->continuation_f)
      return;
  }
}


static void loop_dispatcher(app_t *app)
{
  usart_printf("=========\n");
  usart_printf("continuation dispatcher\n");
  print_stack_pointer();
  usart_printf("> ");

 static uint32_t soft_500ms = 0;

  while(true) {

    update_console_cmd(app);

    // 500ms soft timer. should handle wrap around
    if( (system_millis - soft_500ms) > 500) {
      soft_500ms += 500;

      //
      led_toggle();
    }

    if(app->continuation_f) {
      printf("jump to continuation\n");
      void (*tmppf)(void *) = app->continuation_f;
      app->continuation_f = NULL;
      tmppf( app->continuation_ctx );

      printf("continuation done\n");
      usart_printf("> ");
    }

  }
}










static char buf_console_in[1000];
static char buf_console_out[1000];



static char buf_command[100];

static app_t app;


int main( int arg0 )
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


  // This is all bad. we should be declaraing and initializing on the stack. and then passing into the app_t structure by reference. 
  // eg. pointer.
  // even in c.


  // DONT do this in c++
  memset(&app, 0, sizeof(app_t));

  app.continuation_ctx = NULL;
  app.continuation_f = NULL;




  ///////
  // uart/console
  cBufInit(&app.console_in,  buf_console_in, sizeof(buf_console_in));
  cBufInit(&app.console_out, buf_console_out, sizeof(buf_console_out));


  // command buffer
  cStringInit(&app.command, buf_command, buf_command + sizeof( buf_command));
  assert(cStringReserve(&app.command) == sizeof( buf_command));
  assert(cStringCount(&app.command) == 1); // null terminator


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

  // ram growing up.
  printf("arg0 %u \n", ((unsigned )(void *) &arg0 )  );
  printf("arg0 diff %uk\n", (((unsigned )(void *) &arg0 )  - 0x20000000 ) / 1024 );

  print_stack_pointer();



  usart_flush();

  loop_dispatcher(&app);
  // loop1(&app);
}


