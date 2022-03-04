/*

  spi master and slave on the same device.

  ------------
  nix-shell ~/devel/nixos-config/examples/arm.nix
  make

  serial,
  rlwrap -a picocom -b 115200 /dev/ttyUSB0

  usb
  screen /dev/ttyACM0 115200

  screen
  openocd -f openocd.cfg
  rlwrap nc localhost 4444  # in new window

  reset halt ; flash write_image erase unlock /home/me/devel/stm32/stm32f4/projects/control-panel-2/main.elf; sleep 1; reset run

  *********
  with gnu sprintf, and floating point code, this still fits in 27k, tested by editing f410.ld.  good!!
  *********
  -----------------------------
  one mcu or two.

  two two.
    control paenl just issue spi commands and return core structure.
    spi slave - just handle as interupt.
        eg. receive a byte, decide what to do. could be just using spi_xfer()

    - any function in core - can be exposed as spi command. if required.
    - just enable an interupt on CS. then in the isr - handle it.

    - nss pin is set as an interupt input.


*/


#include <libopencm3/stm32/rcc.h>
// #include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/spi.h>

// #include <libopencm3/cm3/nvic.h>
// #include <libopencm3/cm3/systick.h>



#include <libopencm3/usb/usbd.h>


// #include <setjmp.h>
#include <stddef.h> // size_t
//#include <math.h> // nanf
//#include <stdio.h>
#include <string.h>   // memset
#include <stdio.h>   // putChar


#include "cbuffer.h"
#include "cstring.h"
#include "usart.h"
#include "util.h"
#include "assert.h"
#include "cdcacm.h"
#include "streams.h"


// #include "str.h"  //format_bits
#include "format.h"  //format_bits


#include "spi.h"
#include "dac8734.h"
#include "4094.h"


typedef struct app_t
{
  CBuf console_in;
  CBuf console_out;

  CString     command;

  usbd_device *usbd_dev ;

} app_t;




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

      printf("cmd is '%s'\n", cmd);


      // reset buffer
      cStringClear( &app->command);

      // issue new command prompt
      usart_printf("> ");
    }
  }
}









static void loop(app_t *app)
{
  /*
    loop() subsumes update()
  */

  // TODO move to app_t structure?.
  static uint32_t soft_500ms = 0;
  static uint32_t soft_1000ms = 0;

  while(true) {

		usbd_poll(app->usbd_dev);

    update_console_cmd(app);

    // usart_output_update(); // shouldn't be necessary
    if( (system_millis - soft_500ms) > 500) {
      soft_500ms += 500;
      led_toggle();
    }


    // 500ms soft timer. should handle wrap around
    if( (system_millis - soft_1000ms) > 1000) {

      soft_1000ms += 1000;

#if 0
      static unsigned count = 0;

      usart_printf("writing spi1 using cs2 \n" );
      spi_enable( SPI1 );
      // uint8_t val =  spi_xfer(SPI1, count % 2 == 0 ? 0xff : 0x00  ); // commmand.

      /*
      // this works. nice.
      // OK. reading back the last written value. works. but is QS1. high-z.  when strobe not set.
      uint8_t val =  spi_xfer(SPI1,  count );
      printf("count %d read value %d\n", count, val );
      */

      UNUSED(val);
      spi_disable( SPI1 );

      ++count;
#endif

    }

  }
}




static char buf_console_in[1000];
static char buf_console_out[1000];


static char buf_command[100];

static app_t app;






////////////////////
// I don't see that we can transfer control.
// eg. the write will pause until it has sent.
// and only then transfer to the read.

////////////////////



/*
  Ok, vertical is ok.   but we want to flip the horizontal origin.
  to draw from top left. that shoudl be good for fillRect, and for agg letter.
*/

int main(void)
{

  // required for usb
	// rcc_clock_setup_pll(&rcc_hse_8mhz_3v3[RCC_CLOCK_3V3_84MHZ] );  // stm32f411  upto 100MHz. works stm32f407 too.
  rcc_clock_setup_pll(&rcc_hse_8mhz_3v3[RCC_CLOCK_3V3_168MHZ] );  // stm32f407

  /*
  // http://libopencm3.org/docs/latest/stm32f4/html/f4_2rcc_8h.html

    see here for rcc.c example, defining 100MHz, using 25MHz xtal.
      https://github.com/insane-adding-machines/unicore-mx/blob/master/lib/stm32/f4/rcc.c
  */
  // 16MHz. for hsi from datasheet.

  // systick_setup(16000);
  // systick_setup(16000);
  // systick_setup(84000);  // 84MHz.
  systick_setup(168000);



  // clocks
  rcc_periph_clock_enable(RCC_SYSCFG); // maybe required for external interupts?

  // LED
  rcc_periph_clock_enable(RCC_GPIOA);
  rcc_periph_clock_enable(RCC_GPIOB);
  rcc_periph_clock_enable(RCC_GPIOE);

  // USART
  rcc_periph_clock_enable(RCC_GPIOB); // F410/f411
  rcc_periph_clock_enable(RCC_USART1);


  // USB
	rcc_periph_clock_enable(RCC_OTGFS);



  // spi1
  rcc_periph_clock_enable(RCC_SPI1);

  // spi2
  // rcc_periph_clock_enable(RCC_SPI2);


  //////////////////////
  // setup



  // led
  led_setup();


  memset(&app, 0, sizeof(app_t));

  // uart/console
  cBufInit(&app.console_in,  buf_console_in, sizeof(buf_console_in));
  cBufInit(&app.console_out, buf_console_out, sizeof(buf_console_out));


  cStringInit(&app.command, buf_command, buf_command + sizeof( buf_command));


  // standard streams for printf, fprintf, putc.
  init_std_streams( &app.console_out );



  //////////////
  // initialize usart before start all the app constructors, so that can print.
  // uart
  // usart_setup_gpio_portA();
  usart_setup_gpio_portB();

  usart_set_buffers(&app.console_in, &app.console_out);



  usart_printf("\n--------\n");
  usart_printf("addr main() %p\n", main );



#if 1
  ////////////////////////////
  // usb
  // might be better to pass as handler?
	app.usbd_dev = usb_setup();
  assert(app.usbd_dev);
#endif


  usart_printf("\n--------");
  usart_printf("\nstarting\n");

  usart_printf("\n--------\n");
  usart_printf("starting loop\n");
  usart_printf("sizeof bool   %u\n", sizeof(bool));
  usart_printf("sizeof float  %u\n", sizeof(float));
  usart_printf("sizeof double %u\n", sizeof(double));

  usart_printf("\n--------\n");

  /*
    OK. issues getting this work.
    turning on hse.  smu does not have hse turned on.
    and then having usb hang/ when not hse.
    ----
    when using hse. can see on scope. the CS never goes high, between the 24bit spi xfers.
    ----------
    OK. even when set the spi clk divider to 128. it still runs everything together...

  */

  // make sure starts lo
  spi1_cs2_clear();


  uint32_t spi = SPI1;

  uint8_t reg4064_value; 

  int ret = dac_init(spi, & reg4064_value); // bad name?
  if(ret != 0) {
    assert(0);
  }


  /// turn on rails.
  usart_printf("\n--------\n");
  usart_printf("turn on rails\n");
  spi1_port_cs2_setup();
  spi_4094_setup(spi);
  reg4064_value |= REG_RAILS_ON;
  assert(reg4064_value == (REG_RAILS_ON | REG_DAC_RST | REG_DAC_UNI_BIP_A));
  spi_4094_reg_write(spi, reg4064_value);


  // write an output
  usart_printf("\n--------\n");
  usart_printf("writing register for dac0 \n");
  spi1_port_cs1_setup(); // with CS.
  spi_dac_setup( spi);

  /* ahhh. remember for smu. we use unipolar outputs...  from memory.
  // perhaps leave as is. 
  */

  spi_dac_write_register( spi, DAC_DAC0_REGISTER, voltage_to_dac( 1.0 ));    // -2 not working??? emits positive.
  // spi_dac_write_register( spi, DAC_DAC0_REGISTER, 0xffff );   // 0xffff negative? 

 


  loop(&app);
}

// put the code after the function. to prevent inlining.



