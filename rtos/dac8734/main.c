/*
 *
  see, for
  ~/devel/stm32/FreeRTOSv10.3.1/FreeRTOS/Demo/CORTEX_M4F_STM32F407ZG-SK/FreeRTOSConfig.h

  doc,
  https://www.freertos.org/FreeRTOS-for-STM32F4xx-Cortex-M4F-IAR.html

  so, i think 'proper' usart will use dma.

 */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"




// #include <libopencm3/cm3/nvic.h>
#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>
// #include <libopencm3/stm32/timer.h>

#include <libopencm3/stm32/spi.h>
// #include <libopencm3/stm32/gpio.h>
// #include <libopencm3/stm32/usart.h>
// #include <libopencm3/cm3/nvic.h>


#include "usart.h"
#include "led.h"


#define DAC_PORT_CS   GPIOA
#define DAC_CS        GPIO4

//
#define DAC_SPI       SPI1

// use spi1/ port A alternate function
#define DAC_CLK       GPIO5
#define DAC_MOSI      GPIO6
#define DAC_MISO      GPIO7

//  GPIOE
#define DAC_PORT      GPIOE
// GPIO1 is led.
#define DAC_LDAC      GPIO2
#define DAC_RST       GPIO3
#define DAC_GPIO0     GPIO4  // gpio pe4   dac pin 8
#define DAC_GPIO1     GPIO5
#define DAC_UNIBIPA   GPIO6
#define DAC_UNIBIPB   GPIO7

// rails...  can we do it in order...
#define RAILS_PORT    GPIOE
// #define RAILS_POS     GPIO8   // pull high to turn on.  I think we fucked this port...

#define RAILS_NEG     GPIO9   // pull low to turn on

#define RAILS_POS     GPIO10



static void msleep(uint32_t x)
{
  // only works in a task thread... do not run in main initialization thread
  vTaskDelay(pdMS_TO_TICKS(  x  )); // 1Hz
}




// static int last = 0;

static void led_blink_task2(void *args __attribute((unused))) {

	for (;;) {

		gpio_toggle(LED_PORT, LED_OUT);

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



static void dac_write_register(uint32_t r)
{
  // yes... amount is on the right
  spi_send( DAC_SPI, (r >> 16) & 0xff );
  spi_send( DAC_SPI, (r >> 8) & 0xff  );
  spi_send( DAC_SPI, r & 0xff  );  // depending on what we set this we get different values back in c.
}


static void dac_write_register1(uint32_t r)   // change name dac_write_register_cs
{
  gpio_clear(DAC_PORT_CS, DAC_CS);  // CS active low
  // msleep(1);
  dac_write_register( r );        // writes,
  msleep(1); // required
  gpio_set(DAC_PORT_CS, DAC_CS);      // if ldac is low, then latch will latch on deselect cs.
}

/*
static void dac_write_register2(uint32_t a, uint32_t d)   // change name dac_write_register_cs
{
  // address/ data/
}
*/



static uint32_t dac_read(void)
{
  return 123;
  // this will overwrite the register... because we cannot clock in a clear value...
  // this whole thing just hangs...

  msleep(1); // required
  gpio_clear(DAC_PORT_CS, DAC_CS);  // CS active low
  msleep(1);
  /*
  uint8_t a = spi_read(DAC_PORT);
  uint8_t b = spi_read(DAC_PORT);
  uint8_t c = spi_read(DAC_PORT);
  */

  uint8_t a = spi_xfer(DAC_PORT, 0);
  uint8_t b = spi_xfer(DAC_PORT, 0 );
  uint8_t c = spi_xfer(DAC_PORT, 0);


  msleep(1); // required
  gpio_set(DAC_PORT_CS, DAC_CS);      // if ldac is low, then latch will latch on deselect cs.

  return (a << 16) | (b << 8) | c;
}


static void dac_test(void *args __attribute((unused)))
{
  /*
  Reset input (active low). Logic low on this pin resets the input registers
  and DACs to the values RST67I defined by the UNI/BIP pins, and sets the Gain
  Register and Zero Register to default values.
  */

  // do latch first... not sure if makes a difference
  gpio_clear(DAC_PORT, DAC_LDAC);   // keep latch low, and unused, unless chaining

  /*
  Output modeselection of groupB (DAC-2 and DAC-3). When UNI/BIP-A is tied to
  IOVDD, group B is in unipolar output mode; when tied to DGND, group B is in
  bipolar output mode
  */
  gpio_set(DAC_PORT, DAC_UNIBIPA);
  gpio_set(DAC_PORT, DAC_UNIBIPB);

  msleep(1000);   // 500ms not long enough. on cold power-up.
                  // 1s ok.
                  // actually sometimes 1s. fails.
                  // maybe issue with latch...
                  // Ok. 500ms was ok. when clear latch first.
                  // and 250ms was ok.
                  // actually nope. it must have been running from cap charge.  10secs usb unplug no good.
                  // ----------
                  // Maybe need to pull reset low - as first gpio configuration action, before configure spi etc.
                  // Also could be rails, need to be pulled to ground - first, and there is stray capacitance on them.
                  // Also could be,


  uart_printf("dac reset\n\r");

  gpio_clear(DAC_PORT, DAC_RST);
  msleep(100);
  gpio_set(DAC_PORT, DAC_RST);
  msleep(100);

  uart_printf("reset done\n\r");


/*
  uart_printf("dac gpio read %d %d\n\r", gpio_get(DAC_PORT, DAC_GPIO0), gpio_get(DAC_PORT, DAC_GPIO1));
  // TODO - IMPORTANT - remove this.  just clear gpio pins separately if need to.
  uart_printf("dac clear\n\r");
  dac_write_register1( 0);
  uart_printf("gpio read %d %d\n\r", gpio_get(DAC_PORT, DAC_GPIO0), gpio_get(DAC_PORT, DAC_GPIO1));
*/



  // check gpio was set... else exit.

  /*
  34,the digital supplies (DVDD and IOVDD) and logic inputs (UNI/BIP-x) must be
  applied before AVSS and AVDD. Additionally, AVSS must be applied before AVDD
  unless both can ramp up at the same time. REF-x should be applied after AVDD
  comes up in order to make sure the ESD protection circuitry does not turn on.
  */

  msleep(1000);
  uart_printf("turn rails on \n\r");
  gpio_clear(RAILS_PORT, RAILS_NEG);
  msleep(100);
  gpio_set  (RAILS_PORT, RAILS_POS);

  uart_printf("rails on \n\r");
  // turning the rails on - brings the monitor pin to 0
  msleep( 1000);


  // OK. seems to be an issue - when we turn the voltage ref on..
  // then it sucks 10mA on both rails.
  // doesn't seem right... but doesn't go current mode?
  // but its not CC. so maybe under 10mA.

  // don't need to have anything else connected...
  // are other pins - needing fb?

  /////
  // if cannot get working - then should test if can get AIN MON to work. eg. feed it a voltage.
  // and see if can get muxer to work - to select alternative input.
  // this would verify register writing.
  // and test.

  // OK. we should set the uni/pol . shouldn't matter. any value will do... it is output configured?

  // OKK. no. i think there is something weird... perhaps pinout...
  // should not draw current... and more current when ref is higher.

  // touching either of the refs with 3V.. fails... check for oscillation? no.
  // because siggen ground is not floating?

  ////////********
  // 0) test if AIN / MON work. - eg. can write register for selection properly. and fb is what we expect.
  // 1) perhaps there is a solder bridge... somewhere - around the refs.
  // 2) DO NOT  write the 0 register with 0 on init there is no need.
  // 3) other pins - need fb circuits setup -- perhaps oscillating?
  // 4) don't modify any register - but check if still have high current draw issue. and can also try to write dac register.
          // should try not to modify - the primary register at all - in case writing 0 in all bits is causing problems.
          // but can still write the monitor, or dac registers with values. ok.

  // 5) LDAC register behavior is possibly different for dac registers than general registers?
  // 6) is the REF supposed to be a shunt or something? or negative?
  // 7) we should be able to do a read of a register - even if overwriting it / using mcu spi ... good to test

  // problem we cannot write any register cleanly without affecting other bits - if we cannot read register first
  // to OR the values.
  // FUCK. even still it shouldn't matter - if we know the default values...



  //////////////


  // we cannot clear pins - without also clearing default values, because cannot OR...  without read value
  // dac_write_register1( 0b00000000 << 16 | 1 << 8 | 1 << 7  ); // write gpio pins
  // dac_write_register1( 0b00000000 << 16 | 0  );               // write gpio pins
  // msleep(1);
  // uart_printf("gpio now %d %d\n\r", gpio_get(DAC_PORT, DAC_GPIO0), gpio_get(DAC_PORT, DAC_GPIO1));

  /*
    ...
    p21.
    The DAC8734 updates the DAC latch only if it has been accessed since the last
    time the LDAC pin was brought low or the LD bit in the CommandRegister was set
    to'1', there by eliminating any unnecessary glitch.
  */



  // WRITING THIS - does not affect mon value...
  uart_printf("writing dac register 1\n\r");
  dac_write_register1( 0b00000101 << 16 | 0xffff ); // write dac 1.
  msleep(1);  // must wait for update - before we read
  /*  */


  // toggle latch in case it makes a difference
  uart_printf("toggle ldac\n\r");
  gpio_set(DAC_PORT, DAC_LDAC);
  msleep(1);
  gpio_clear(DAC_PORT, DAC_LDAC);
  msleep(1);


  // it is too strange - that the damn monitor muxer doesn't work for ain.
  // indicates the supply rails must be off - or chip is no good.

  // selecting dac  inputs gets voltages...
  // lets make sure the uni/bip  is not highZ

  // OK. with ref=3V, get mon of -0.7V when reading mon for  dac0 and dac1
  // with ref=1V      get mon of -0.5V when reading mon for dac0 and dac1
  // writing dac value does not seem to do anything.


  uart_printf("write mon register for ain\n\r");
  dac_write_register1( 0b00000001 << 16 | 0b00001000 << 10 ); // select AIN.
  msleep(2000);

  uart_printf("write mon register for dac1\n\r");
  dac_write_register1( 0b00000001 << 16 | 0b00001000 << 12   ); // select dac 1
  msleep(2000);

  // ok - we have a slightly different value... for dac0 or dac1, from cleared

  // dac0 also has slightly different value...
  uart_printf("write mon register for dac0\n\r");
  dac_write_register1( 0b00000001 << 16 | 0b00001000 << 11   ); // select dac 0
  msleep(2000);

  uart_printf("write mon register to clear\n\r");
  dac_write_register1( 0b00000001 << 16 | 0   );
  msleep(2000);

 //


  // So... try to provide a reference voltage maybe...


  // So everything is always 0. and when plug ref in, it starts to use lots of current.
  // try to wire it again?


  // but clearing it again - has no effect...
  // uart_printf("clearing the mon register\n\r");
  // dac_write_register1( 0b00000001 << 16 | 0   ); // clear
  // dac_write_register1( 0b00000001 << 16 | 1 << 11   ); // set dac 0
  // msleep(1000);


/*
  uart_printf("dac gpio read now %d %d\n\r", gpio_get(DAC_PORT, DAC_GPIO0), gpio_get(DAC_PORT, DAC_GPIO1));
  msleep(100);
*/

/*
  // doesn't seem to work at all...
  uart_printf("starting dac 1 read");
  // try to read...
  dac_write_register1( 0b10000101 << 16 );
  msleep(1);
  uint32_t  x = dac_read();
  uart_printf("dac 1 val %d\n\r", x);
*/

  uart_printf("finished\n\r");

  // sleep forever
  // exiting a task thread isn't very good...
  for(;;) {
    msleep(1000);
  }
}



static void dac_setup( void )
{

  uart_printf("dac gpio/af setup\n\r");

  // spi alternate function 5
  gpio_mode_setup(GPIOA, GPIO_MODE_AF, GPIO_PUPD_NONE,  DAC_CLK | DAC_MOSI | DAC_MISO );

  gpio_set_af(GPIOA, GPIO_AF5,  DAC_CLK | DAC_MOSI | DAC_MISO );

  // rcc_periph_clock_enable(RCC_SPI1);
  spi_init_master(DAC_SPI,
    SPI_CR1_BAUDRATE_FPCLK_DIV_4,
    SPI_CR1_CPOL_CLK_TO_0_WHEN_IDLE,
    SPI_CR1_CPHA_CLK_TRANSITION_2,    // 1 == rising edge, 2 == falling edge.
    SPI_CR1_DFF_8BIT,
    SPI_CR1_MSBFIRST);
  spi_enable_ss_output(DAC_SPI);
  spi_enable(DAC_SPI);


  // CS same pin as SPI alternate function, but we use it as gpio out.
  gpio_mode_setup(DAC_PORT_CS, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, DAC_CS);

  /////
  // other outputs
  /// gpio_mode_setup(DAC_PORT, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, DAC_LDAC  | DAC_UNIBIPA | DAC_UNIBIPB);
  // gpio_mode_setup(DAC_PORT, GPIO_MODE_OUTPUT, GPIO_PUPD_PULLUP, DAC_RST );

  // internal pu, doesn't change anything - because its powered off, and starts up high-Z.
  gpio_mode_setup(DAC_PORT, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, DAC_RST | DAC_LDAC | DAC_UNIBIPA | DAC_UNIBIPB);



  // dac gpio inputs, pullups work
  gpio_mode_setup(DAC_PORT, GPIO_MODE_INPUT, GPIO_PUPD_PULLUP, DAC_GPIO0 | DAC_GPIO1 );
}




static void rails_setup( void )
{

  uart_printf("rails setup\n\r");


  // ok. define before enabling...
  // if we do this after setup - then the neg rail, needs high, will glitch on reset.
  // turn off

  uart_printf("rails off \n\r");
  gpio_clear(RAILS_PORT, RAILS_POS);
  gpio_set  (RAILS_PORT, RAILS_NEG);


  gpio_mode_setup(RAILS_PORT, GPIO_MODE_OUTPUT,  GPIO_PUPD_NONE /*GPIO_PUPD_PULLDOWN */, RAILS_POS  );
  gpio_mode_setup(RAILS_PORT, GPIO_MODE_OUTPUT,  GPIO_PUPD_NONE /*GPIO_PUPD_PULLUP*/,   RAILS_NEG );
  gpio_mode_setup(RAILS_PORT, GPIO_MODE_OUTPUT,  GPIO_PUPD_NONE ,   GPIO8 ); // broken.. gpio.


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


}


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
*/

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


  uart_printf("--------\n\r");

  dac_setup();

  rails_setup();

  ///////////////
  // tasks
  // value is the stackdepth.
	xTaskCreate(led_blink_task2, "LED",100,NULL,configMAX_PRIORITIES-1,NULL);
  xTaskCreate(uart_task,      "UART",200,NULL,configMAX_PRIORITIES-1,NULL); /* Highest priority */

  // IMPORTANT setting from 100 to 200, stops deadlock
  xTaskCreate(prompt_task,    "PROMPT",200,NULL,configMAX_PRIORITIES-2,NULL); /* Lower priority */


  // ok....
  xTaskCreate(dac_test,    "DAC_TEST",200,NULL,configMAX_PRIORITIES-2,NULL); /* Lower priority */

	vTaskStartScheduler();

	for (;;);
	return 0;
}













  // msleep(1);
  // gpio_clear(DAC_PORT_CS, DAC_CS);  // CS active low
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
  // gpio_set(DAC_PORT_CS, DAC_CS);      // if ldac is low, then latch will latch on deselect cs.


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


