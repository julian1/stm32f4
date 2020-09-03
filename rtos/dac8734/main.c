/*
 *
  see, for
  ~/devel/stm32/FreeRTOSv10.3.1/FreeRTOS/Demo/CORTEX_M4F_STM32F407ZG-SK/FreeRTOSConfig.h

  doc,
  https://www.freertos.org/FreeRTOS-for-STM32F4xx-Cortex-M4F-IAR.html

  so, i think 'proper' usart will use dma.

  // OK. so we want to move the dac code.

 */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

/*
  - ok. move back to actual spi port - see if can still bit bash.
  - see if peripheral spi works.
  - mon - to ADC - resistor divider? won't work - for negative signals.

  - we want
*/

// #include <libopencm3/cm3/nvic.h>
#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>
// #include <libopencm3/stm32/timer.h>
// #include <libopencm3/stm32/spi.h>
// #include <libopencm3/stm32/gpio.h>
// #include <libopencm3/stm32/usart.h>
// #include <libopencm3/cm3/nvic.h>


#include "usart.h"
#include "led.h"
#include "rails.h"
#include "dac8734.h"
#include "utility.h" // msleep







static int last = 0;

static void led_blink_task2(void *args __attribute((unused))) {

	for (;;) {

    // led_toggle
		led_toggle();

    // ping
    uart_printf("hi %d\n\r", last++);
/*
    uart_printf("hi %d %d %d\n\r",
      last++,
      gpio_get(DAC_PORT, DAC_GPIO0),
      gpio_get(DAC_PORT, DAC_GPIO1 )

    );
*/
    msleep(500);
  }
}


// set the gpio edges to be harder? eg. 20MHz.
// OK - using spi didn't work. but bitbashing does. so perhaps there's still issues.
// not power supplies... but writing.




// OK. do we have a sleep function for bit bashing...?

int main(void) {

  ////
  // clocks
  rcc_clock_setup_pll(&rcc_hse_8mhz_3v3[RCC_CLOCK_3V3_168MHZ]);

  // led
  rcc_periph_clock_enable(RCC_GPIOE); // JA

  // usart
  rcc_periph_clock_enable(RCC_GPIOA);
  rcc_periph_clock_enable(RCC_USART1);

  // spi1
  rcc_periph_clock_enable(RCC_SPI1);



  ///////////////
  // setup
  led_setup();
  usart_setup();


  uart_printf("------------------\n\r");

  rails_setup();


  // dac_setup_bitbash();
  dac_setup_spi();


  ///////////////
  // tasks
  // value is the stackdepth.
	xTaskCreate(led_blink_task2, "LED",100,NULL,configMAX_PRIORITIES-1,NULL);
  xTaskCreate(uart_task,      "UART",200,NULL,configMAX_PRIORITIES-1,NULL); /* Highest priority */

  // IMPORTANT setting from 100 to 200, stops deadlock
  xTaskCreate(usart_prompt_task,    "PROMPT",200,NULL,configMAX_PRIORITIES-2,NULL); /* Lower priority */


  // ok....
  xTaskCreate(dac_test,    "DAC_TEST",200,NULL,configMAX_PRIORITIES-2,NULL); /* Lower priority */

	vTaskStartScheduler();

	for (;;);
	return 0;
}





// OK. on reset there is no glitch. for neg rail. or pos rail. but there is when 3.3V power first applied . 250nS.



// gpio_clear (RAILS_PORT, RAILS_NEG);    // turn on... eg. pull p-chan gate down from 3.3V to 0.

// OK - problem - our p-chan fet for neg rail is barely turning on.
// to pull up the n-chan gate.


// OK - a 1k on the fet gate. and defining the port before init... makes the top rail not glitch.

// bottom rail  - our p-chan fet is too weak.
// if use 1k / 10k. it doesn't turn on at all.
// if use without input resistor - it barely turns on.
///// /HMMMMM

// no it does glitch.
// but why? because of weako

// ok. and 1k seems to often work for neg rail.
// issue - but secondary issue is that the tx VGS is too bad for us to use it to turn the damn rail on.
// God damn it...
// sometimes it spikes and sometimes it doesn't - with or without resistor.
// when it first turns on - the gate is negative.
// freaking...
// ---------
// try to use dg444?
// and now we cannot trigger it...

// ok added a 22uF cap as well...   glitching is only occasional and slight.c
// powering on the other stmicro - 3.3V supply helps a bit also.





/*
  OK. its complicated.
    the fet comes on as 3V is applied...
    BUT what happens if
    if have rail voltage and no 3V. should be ok. because of biasing.

  When we first plug in- we get a high side pulse.
    about 250nS.

    Looks like the gate gets some spikes.
    OK. but what about a small cap...
  ----
  ok 1nF helps
  but there's still a 20nS pulse... that turns on the gateo
  --
  OK. both rails glitch at power-on.
  ...
  fuck.

  try a cap across the gate...
  ok 1nF seems to be ok.

  ok. 0.1uF seems to have fixed.
  NOPE.

  OK. lets try a 10k on the gate... to try and stop the ringing.
  mosfet gatep has cap anyway.

  OK. disconnecting the signal - and we don't trigger. very good.
  ------------
  sep 1.
  OK. started from cold ok. but now cannot get to start now.
    No seems to come on ok with 1V ref on. 0.730V dac1-out.

*/








  // msleep(1);
  // gpio_clear(DAC_PORT_SPI, DAC_CS);  // CS active low
  // msleep(1);

  /*
    we need to control the general 3.3V power rail, as well without unplugging the usb all the time
      and having to reset openocd.
    -------
    actually perhaps we need to control the 3.3V power for the dac.
    not. sure
    turn on only after have everything set up. no. better to give it power first?
    i think.
    --------
    would it make it easier to test things - if could power everything separately.
  */

  /*
  Writing a '1' to the GPIO-0 bit puts the GPIO-1 pin into a Hi-Z state(default).
  DB8 GPIO-01 Writing a '0' to the GPIO-0 bit forces the GPIO-1 pin low

  p22 After a power-on reset or any forced hardware or software reset, all GPIO-n
  bits are set to '1', and the GPIO-n pin goes to a high-impedancestate.
  */

  // spi_xfer is 'data write and read'.
  // think we should be using spi_send which does not return anything

  // dac_write_register(  1 << 8 | 1 << 7 );

  // dac_write_register( 1 << 22 | 1 << 8 | 1 << 6 ); // read and nop
  // dac_write_register( 1 << 8  | 1 << 6 );      // nop, does nothing
  // dac_write_register( 1 << 22 | 1 << 8 );      // writes, it shouldn't though...
  // dac_write_register( 0 );        // turns off ,
  // dac_write_register( 1 << 7  );

  // very strange - code does not initialize properly... when plugged...

  /*********
  // reset gives gpio values = 1, which is high-z, therefore pulled hi.
  // setting to 0 will clear.
  **********/

  // msleep(1); // required
  // gpio_set(DAC_PORT_SPI, DAC_CS);      // if ldac is low, then latch will latch on deselect cs.


  // gpio_clear(DAC_PORT, DAC_LDAC);



/*
  // first byte,
  spi_send(DAC_SPI, 0);
  //spi_send(DAC_SPI, 0);
  spi_send(DAC_SPI, 0 | 1 );           // dac gpio1 on

  // spi_send(DAC_SPI, 0);
  spi_send(DAC_SPI, 0 | 1 << 7 );  // turn on gpio0
*/

#if 0
static uint8_t dac_read(void)
{


  /*
    OK.
      some timing diagrams are weird. BUT

      case 5. p11.  for standalone mode. read timing is fine. see p11.
  */

/*
    OK. issue is that spi_xfer attempts to read after sending a full byte
    while we want simultaneous. read/write.
    Not sure if supported or can do it without bit-bashing supported.

    spi_xfer,
      "Data is written to the SPI interface, then a read is done after the incoming transfer has finished."

    issue is that we cannot just clock it out.
    instead we have to send a no-op, while clocing it out.

    BUT. if we used a separate spi channel for input.
    Then we could do the write.
    while simultaneously doing a blocking read in another thread.
    pretty damn ugly.
    better choice would be to bit-bash.
*/

  // dac_write_register1( 0 );



                                                // very strange
  return 123;   // c value is 128... eg. 10000000 this was kind of correct for the gpio1 in last byte. ....
              // so appear to be getting something out....
              // but really need to look at it on a scope

              // b is now returning 1....
              // c is returning 2.
}
#endif

// use spi_read


/*
  strange issue - when first plug into usb power - its not initialized properly...
  but reset run is ok.
  could be decoupling
  or because mcu starts with GPIO undefined?.. but when do 'reset run' the gpio is still defined because its
  a soft reset?
  - Or check the reset pin is genuinely working? or some other sync issue?
*/

/*
  OK. that is really very very good.
    we want to add the other uni/bip .

  and we want to try and do spi read.
  bit hard to know how it works - if get back 24 bytes.
    except we control the clock... well spi does.

  having gpio output is actually kind of useful - for different functions . albeit we would just use mcu.
  likewise the mixer.
  ---

  ok added external 10k pu. does not help. bloody weird.

*/
#if 0
  spi_send(DAC_SPI, 0);
  //spi_send(DAC_SPI, 0);
  spi_send(DAC_SPI, 0 | 1 );           // dac gpio1 on

  // spi_send(DAC_SPI, 0);
  spi_send(DAC_SPI, 0 | 1 << 7 );  // turn on gpio0
#endif


#if 0
  // pull latch up to write
  uart_printf("toggle ldac\n\r");
  gpio_set(DAC_PORT, DAC_LDAC);
  msleep(1);

  // gpio_clear(DAC_PORT, DAC_LDAC);
  // msleep(1);
#endif

