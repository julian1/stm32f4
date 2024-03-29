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

/*
  - move back to actual spi port - see if can still bit bash.
  - see if peripheral spi works.
  - mon - to ADC - resistor divider? won't work - for negative signals. 
*/

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


#define DAC_SPI       SPI1

#define DAC_PORT_SPI   GPIOA
#define DAC_CS        GPIO4
// use spi1/ port A alternate function
#define DAC_CLK       GPIO5
// #define DAC_MOSI      GPIO6   // think BROKEN 
// #define DAC_MOSI      GPIO3   // also not strong... --- uggh...  maybe its the dac? or a short to the dac somehow?

//#define DAC_MISO      GPIO7
#define DAC_MISO      GPIO3 // not connected right now.
#define DAC_MOSI      GPIO7

// fucking hell 
// spi 1 AF5 
//  MOSI == PA7 == GPIO7    DAC SDI pin 4
//  MISO == PA6 == GPIO6    DAC SDO pin 5

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




static int last = 0;

static void led_blink_task2(void *args __attribute((unused))) {

	for (;;) {

		gpio_toggle(LED_PORT, LED_OUT);

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

// oko - there is is absolutely nothign on the scope...  when Vref=4V sometimes.
// like the register never got wrote. 
// and ain mon is not working
// what if we try to write multiple times...

static void dac_write_register__(uint32_t r)
{
/*
  // yes... amount is on the right
  spi_send( DAC_SPI, (r >> 16) & 0xff );
  spi_send( DAC_SPI, (r >> 8) & 0xff  );
  spi_send( DAC_SPI, r & 0xff  );  // depending on what we set this we get different values back in c.
*/

  for(int i = 23; i >= 0; --i) {

    gpio_set(DAC_PORT_SPI, DAC_CLK);  // clock high
    // msleep(1);      // OK... interesting it doesn't like this at all...
                        // but it should be fine

    // assert value
    if( r & (1 << i ))
      gpio_set(DAC_PORT_SPI, DAC_MOSI );
    else
      gpio_clear (DAC_PORT_SPI, DAC_MOSI );


    // gpio_set(DAC_PORT_SPI, DAC_CLK);  // clock high - works...

    msleep(1);
    gpio_clear(DAC_PORT_SPI, DAC_CLK);  // slave gets value on down transition
    msleep(1);

  }

  // maybe
}


static void dac_write_register1(uint32_t r)   // change name dac_write_register_cs
{
  // is there an interaction between clk and CS.
  gpio_set(DAC_PORT_SPI, DAC_CLK); // raise clock
  msleep(1);

  gpio_clear(DAC_PORT_SPI, DAC_CS);  // CS active low
  msleep(1);

  dac_write_register__( r );        // writes,
  msleep(1); // required
  gpio_set(DAC_PORT_SPI, DAC_CS);      // if ldac is low, then latch will latch on deselect cs.
  msleep(1);
}





static uint32_t dac_read(void)
{
  // return 123;
  // this will overwrite the register... because we cannot clock in a clear value...
  // this whole thing just hangs...

  msleep(1); // required
  gpio_clear(DAC_PORT_SPI, DAC_CS);  // CS active low

  // think problem is that it doesn't fiddle the clock.
  uint8_t a = spi_read(DAC_PORT);
  uint8_t b = spi_read(DAC_PORT);
  uint8_t c = spi_read(DAC_PORT);

  /*
  uint8_t a = spi_xfer(DAC_PORT, 0);
  uint8_t b = spi_xfer(DAC_PORT, 0 );
  uint8_t c = spi_xfer(DAC_PORT, 0);
  */
  msleep(1); // required
  gpio_set(DAC_PORT_SPI, DAC_CS);

  return (a << 16) | (b << 8) | c;
}



/*
  OK.  bitbashing sometimes works,

  - seems to really likes a scope probe on it, to burden pin with some capacitance?.
  - but could there be an issue - with interrupts - changing pins - or timing of clocking....
  - some stray mcu signal?

  - ok lowered the vref to 1V and outputting 0.733V - and it doesn't fail nearly as much
      could it be that the high voltage is doing stuff...
    - also on the positive side. not negative rail.
      - note also there's a spike to 4V when it turns on for about 10uS.

    - could remove scope probes and still working

  - we have the decoupling - on the power supply board.
  - which is pretty far away...  should add 10nF.

  - the timing of 100mS is bizarre.
  - need to check mcu isn't resetting somehow/ emit a count...

  - put scope on RST. make sure pin is clean.

  - its a very clean switch-off (sharp transient).... at a precise timing of 200mS ... how... coming from mcu.
  -- like RST was grounded .  or its on an AF
*/

static void dac_test(void *args __attribute((unused)))
{
  /*
  Reset input (active low). Logic low on this pin resets the input registers
  and DACs to the values RST67I defined by the UNI/BIP pins, and sets the Gain
  Register and Zero Register to default values.
  */

  /* Load DAC latch control input(activelow). When LDAC is low, the DAC latch
  is transparent and the LDAC 56I contents of the Input Data Register are
  transferred to it.The DAC output changes to the corresponding level
  simultaneously when the DAClat */

  // do latch first... not sure order makes a difference
  gpio_clear(DAC_PORT, DAC_LDAC);   // keep latch low, and unused, unless chaining

  gpio_clear(DAC_PORT_SPI, DAC_CLK); // raise clock


  /*
  Output mode selection of groupB (DAC-2 and DAC-3). When UNI/BIP-A is tied to
  IOVDD, group B is in unipolar output mode; when tied to DGND, group B is in
  bipolar output mode
  */
  gpio_clear(DAC_PORT, DAC_UNIBIPA);
  gpio_clear(DAC_PORT, DAC_UNIBIPB);  // bipolar, lower supply voltage range required.

  //msleep(1000);   // 500ms not long enough. on cold power-up.
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
  msleep(1000);
  gpio_set(DAC_PORT, DAC_RST);
  msleep(1000);
  uart_printf("reset done\n\r");

  // god damn it.

#if 0
  uart_printf("gpio read %d %d\n\r", gpio_get(DAC_PORT, DAC_GPIO0), gpio_get(DAC_PORT, DAC_GPIO1));
  // TODO - IMPORTANT - remove this.  just clear gpio pins separately if need to.
  uart_printf("dac clear\n\r");
  dac_write_register1( 0);
  uart_printf("gpio read %d %d\n\r", gpio_get(DAC_PORT, DAC_GPIO0), gpio_get(DAC_PORT, DAC_GPIO1));
  uart_printf("dac set\n\r");
  // OK. with bitbashing reg 8 and 9 look correct...
  dac_write_register1( 0 << 16 | 1 << 9 | 1 << 8); // ok so this really looks like it works ok...
  uart_printf("gpio read %d %d\n\r", gpio_get(DAC_PORT, DAC_GPIO0), gpio_get(DAC_PORT, DAC_GPIO1));


  // dac_write_register1( 0 << 16 | 1 << 14); // LD bit    // now sometimes comes up 0, 0.629, or 0.755 ...
                                                        // indicating that gain regsisters are etting garbage?
                                                        // LD default value is 0...
                                                        // freaking weird.
                          // 0.567, 0.629 or 0.755
                          //   0.253?

  // OKK we're getting all kinds of values now...
#endif


  // should check if gpio was successfully set... indicating dac is correctly initialized.

  /*
  34,the digital supplies (DVDD and IOVDD) and logic inputs (UNI/BIP-x) must be
  applied before AVSS and AVDD. Additionally, AVSS must be applied before AVDD
  unless both can ramp up at the same time. REF-x should be applied after AVDD
  comes up in order to make sure the ESD protection circuitry does not turn on.
  */

  uart_printf("turn rails on \n\r");
  gpio_clear(RAILS_PORT, RAILS_NEG);
  msleep(100);
  gpio_set  (RAILS_PORT, RAILS_POS);
  uart_printf("rails on \n\r");
  // turning the rails on - brings the monitor pin to 0
  msleep( 1000);

  /*
    I think we need to understand this better...
    ...
    p21.
    The DAC8734 updates the DAC latch only if it has been accessed since the last
    time the LDAC pin was brought low or the LD bit in the CommandRegister was set
    to'1', there by eliminating any unnecessary glitch.
  */

  // OK... check  we have not screwed up the LDAC output ...

  // WRITING THIS - does not affect mon value...
  uart_printf("writing dac register 1\n\r");
  // dac_write_register1( 0b00000100 << 16 | 0x7f7f ); // write dac 0
  //dac_write_register1( 0b00000101 << 16 | 0x3fff ); // write dac 1 1.5V out.
  // dac_write_register1( 0b00000101 << 16 | 0x2fff ); // write dac 1 1.129 out.
  // dac_write_register1( 0b00000101 << 16 | -10000 ); // didn't work
  // dac_write_register1( 0b00000101 << 16 | 10000 ); // works 0.919V
  // dac_write_register1( 0b00000101 << 16 | 0xffff - 10000 ); //  works. output -0.919V
  dac_write_register1( 0b00000101 << 16 | 0x5fff );


#if 0
  dac_write_register1( 0b00000110 << 16 | 0x7f7f ); // write dac 2
  dac_write_register1( 0b00000111 << 16 | 0x7f7f ); // write dac 3
  msleep(1);  // must wait for update - before we read
#endif


  /*
    OKK - power consumption - seems *exactly* right.  around 10mA.
    AIDD (normaloperation) ±10V output range, no loading current, VOUT=0V 2.7-3.4mA/Channel
    AAISS(normaloperation)±10V outputrange, no loadingcurrent, VOUT=0V 3.3-4.0mA/Channel
    Input current  1μA - this is for the digital section only.
    // guy says device is drawing 10mA.
    // https://e2e.ti.com/support/data-converters/f/73/t/648061?DAC8734-Is-my-dac-damaged-

  */

  // 11 is ain. 13 is dac1.

  uart_printf("write mon register for ain\n\r");
  // dac_write_register1( 0b00000001 << 16 | (1 << 11) ); // select AIN.
  dac_write_register1( 0b00000001 << 16 | (1 << 13) ); // select dac 1.
  msleep(1000);



  uart_printf("set mosi \n\r");
  gpio_set(DAC_PORT_SPI, DAC_MOSI );
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

#if 0
  // spi alternate function 5
  gpio_mode_setup(GPIOA, GPIO_MODE_AF, GPIO_PUPD_NONE,  DAC_CLK | DAC_MOSI | DAC_MISO );

  gpio_set_af(GPIOA, GPIO_AF5,  DAC_CLK | DAC_MOSI | DAC_MISO );

  // rcc_periph_clock_enable(RCC_SPI1);
  spi_init_master(DAC_SPI,
    SPI_CR1_BAUDRATE_FPCLK_DIV_4,     // when we change this - we get different values?
    // SPI_CR1_BAUDRATE_FPCLK_DIV_256,     // the monitor pin values change - but still nothing correct
    SPI_CR1_CPOL_CLK_TO_0_WHEN_IDLE,
    SPI_CR1_CPHA_CLK_TRANSITION_2,    // 1 == rising edge, 2 == falling edge.
    SPI_CR1_DFF_8BIT,
    SPI_CR1_MSBFIRST
    // SPI_CR1_LSBFIRST
  );
  spi_enable_ss_output(DAC_SPI);
  spi_enable(DAC_SPI);


#endif

  // is there something else on the damn ports????
  // if one of the gpio pins is broken - maybe others are as well - but not as obviously...
  // OK... we can set faster or slower speed... no problem.

  // OK. miso was incorrectly configured... it now does 4V ref seemingly ok. but not higher... but 

  // OK - setting to 100MHz clock and nothing works...
  gpio_mode_setup(DAC_PORT_SPI, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, DAC_CS  | DAC_CLK | DAC_MOSI);
  // gpio_set_output_options(DAC_PORT_SPI, GPIO_MODE_OUTPUT, GPIO_OSPEED_2MHZ, DAC_CS  | DAC_CLK | DAC_MOSI); // fails
  // gpio_set_output_options(DAC_PORT_SPI, GPIO_MODE_OUTPUT, GPIO_OSPEED_100MHZ, DAC_CS  | DAC_CLK | DAC_MOSI); // fails
  // gpio_set_output_options(DAC_PORT_SPI, GPIO_MODE_OUTPUT, GPIO_OSPEED_100MHZ, DAC_CS  ); // fails 
  // gpio_set_output_options(DAC_PORT_SPI, GPIO_MODE_OUTPUT, GPIO_OSPEED_100MHZ, DAC_CLK | DAC_MOSI );  // fails
  // gpio_set_output_options(DAC_PORT_SPI, GPIO_MODE_OUTPUT, GPIO_OSPEED_100MHZ, DAC_CLK ); // fails 
  // gpio_set_output_options(DAC_PORT_SPI, GPIO_MODE_OUTPUT, GPIO_OSPEED_100MHZ, DAC_MOSI ); // fails 

  // ok - setting any output option - and it fails...


  /////
  // internal pu, doesn't change anything - because its powered off, and starts up high-Z.
  gpio_mode_setup(DAC_PORT, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, DAC_RST | DAC_LDAC | DAC_UNIBIPA | DAC_UNIBIPB);
  // gpio_set_output_options(DAC_PORT, GPIO_MODE_OUTPUT, GPIO_OSPEED_2MHZ, DAC_RST | DAC_LDAC | DAC_UNIBIPA | DAC_UNIBIPB ); // OK...
  // gpio_set_output_options(DAC_PORT, GPIO_MODE_OUTPUT, GPIO_OSPEED_100MHZ, DAC_RST | DAC_LDAC | DAC_UNIBIPA | DAC_UNIBIPB ); // OK...


  /////////////////////
  gpio_mode_setup(DAC_PORT_SPI, GPIO_MODE_INPUT, GPIO_PUPD_NONE, DAC_MISO );

  // dac gpio inputs, pullups work
  gpio_mode_setup(DAC_PORT, GPIO_MODE_INPUT, GPIO_PUPD_PULLUP, DAC_GPIO0 | DAC_GPIO1 ); // these are open-drain as inputs
}


// set the gpio edges to be harder? eg. 20MHz.
// OK - using spi didn't work. but bitbashing does. so perhaps there's still issues.
// not power supplies... but writing.

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
  ------------
  sep 1.
  OK. started from cold ok. but now cannot get to start now.
    No seems to come on ok with 1V ref on. 0.730V dac1-out. 

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
  // rcc_periph_clock_enable(RCC_SPI1);



  ///////////////
  // setup
  led_setup();
  usart1_setup();


  uart_printf("------------------\n\r");

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

