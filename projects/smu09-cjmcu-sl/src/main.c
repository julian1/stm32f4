
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


#include "spi1.h"
#include "mcp3208.h"
#include "w25.h"
#include "dac8734.h"
#include "ads131a04.h"

#include "mux.h"



// here? or in mux.h.
#define SPI_ICE40       SPI1



/*
  AHHH. is clever
    74hc125.
    if fpga gpio comes up high - OE - is disabled. so out is low.
    if fpga gpio comes up lo   - buffer is disabled so out low.

    there seems to be a 50-100uS spike though on creset.
    hopefully too short a time to turn on opto coupler.
*/


// ok. ads131. ought to be able to read value - without interrupt.
//


static void soft_500ms_update(void)
{
  // blink mcu led
  led_toggle();

  ////////
  // put this in spi1.h.  i think....
  uint32_t spi = SPI_ICE40;


  static int count = 0;


  mux_io(spi);

#if 1
  ////////////////////////////////
  // clear led1
  io_clear(spi, LED_REGISTER, LED1);

  // would be nice to have a toggle function.

  // toggle led2
  if(count % 2 == 0) {
    io_set(spi, LED_REGISTER, LED2);
    // io_set(spi, ADC_REGISTER, ADC_RST);
  }
  else {
    io_clear(spi, LED_REGISTER, LED2);
    // io_clear(spi, ADC_REGISTER, ADC_RST);
  }
#endif

  // DO we want to do this every loop?
  // get supply voltages,
  mux_adc03(spi);
  float lp15v = spi_mcp3208_get_data(spi, 0) * 0.92 * 10.;
  float ln15v = spi_mcp3208_get_data(spi, 1) * 0.81 * 10.;





  typedef enum state_t {
    FIRST,    // INITIAL
    DIGITAL_UP,  // DIGIAL_DIGITAL_UP
    ERROR,
    ANALOG_UP
  } state_t;

  // static
  static state_t state = FIRST;



  switch(state) {

    case FIRST:  {
      // if any of these fail, this should progress to error

      usart_printf("-----------\n");
      usart_printf("digital init start\n" );

      mux_io(spi);

      // should we have wrapper functions here, can then put comments
      // make sure rails are off
      io_clear(spi, RAILS_REGISTER, RAILS_LP15V | RAILS_LP30V | RAILS_LP60V);

      // may as well keep rails OE deasserted, until really ready
      io_set(spi, RAILS_REGISTER, RAILS_OE);

      // turn off dac ref mux. pull-high
      io_set( spi, DAC_REF_MUX_REGISTER, DAC_REF_MUX_A | DAC_REF_MUX_B); // active lo

      // test the flash
      mux_w25(spi);
      spi_w25_get_data(spi);

      // init dac.
      int ret = dac_init(spi, DAC_REGISTER); // bad name?
      if(ret != 0) {
        state = ERROR;
        return;
      }

      usart_printf("digital init ok\n" );
      state = DIGITAL_UP;
      break;
    }


    case DIGITAL_UP:
      if( lp15v > 15.0 && ln15v > 15.0 )
      {

        usart_printf("-----------\n");

        usart_printf("doing analog init -  supplies ok \n");
        usart_printf("lp15v %f    ln15v %f\n", lp15v, ln15v);

#if 1
        mux_io(spi);
        // assert rails oe
        io_clear(spi, RAILS_REGISTER, RAILS_OE);

        // turn on +-15V analog rails
        io_set(spi, RAILS_REGISTER, RAILS_LP15V );
        msleep(50);


        // turn on refs for dac
        mux_dac(spi);
        usart_printf("turn on ref a for dac\n" );
        mux_io(spi);
        io_clear(spi, DAC_REF_MUX_REGISTER, DAC_REF_MUX_A); // active lo


        // turn on set voltages 2V and 4V outputs. works.
        spi_dac_write_register(spi, DAC_VSET_REGISTER, voltage_to_dac( 4.0) );
        spi_dac_write_register(spi, DAC_ISET_REGISTER, voltage_to_dac( 2.0) );
#endif

        /*  Also, the V MON pin output impedance is approximately 2.2kΩ;
            therefore, V MON should be measured with a high-impedance input.
            ----- 
            think we need to unload circuit and test .... ci
        */

        // maybe it is multiplexing multiple values - analog values -  WE CLEAR ALL MON regs.
        // we
        spi_dac_write_register(spi, DAC_MON_REGISTER, DAC_MON_MDAC0  );      // 3.429V    should be vset...

        // spi_dac_write_register(spi, DAC_MON_REGISTER, DAC_MON_AIN );   
        spi_dac_write_register(spi, DAC_MON_REGISTER, DAC_MON_MDAC0  );      // 3.429V    should be vset...
        // spi_dac_write_register(spi, DAC_MON_REGISTER, DAC_MON_MDAC1  );   // 1.71V ? should be vset...
                                                                              // we are forgetting the internal fb.
                                                                              // i think.
        // but it's not outputting the full dac value. because uni/bipolar. or bit 


        /////////////////
        // adc init has to be done after rails are up...
        // init adc
        int ret = adc_init(spi, ADC_REGISTER);
        if(ret != 0) {
          state = ERROR;
          return;
        }

        usart_printf("analog init ok\n" );
        // maybe change name RAILS_OK, RAILS_UP ANALOG_OK, ANALOG_UP
        state = ANALOG_UP;
      }
      break ;


    case ANALOG_UP:
      if((lp15v < 14.7 || ln15v < 14.7)  ) {

        mux_io(spi);
        usart_printf("supplies bad - turn off rails\n");
        usart_printf("lp15v %f    ln15v %f\n", lp15v, ln15v);

        // turn off power
        io_clear(spi, RAILS_REGISTER, RAILS_LP15V );

        // go to state error
        state = ERROR;
      }

      // ... ok.
      float val = spi_adc_do_read(spi );
      usart_printf("adc val %f\n", val);

      break;
/*
      // timeout to turn off...
      if( system_millis > timeout_off_millis) {
        state = INIT;
        usart_printf("timeout - turning off \n");
      }
 */

    case ERROR: {

      // should power down rails if up. but want to avoid looping. so need to read
      // or use a state variable
      // but need to be able to read res
      // actually should perrhaps go to a power down state? after error



      // TODO improve.
      // turn off power
      static int first = 0;
      if(!first) {
        first = 1;

        usart_printf("entered error state\n" );
        io_clear(spi, RAILS_REGISTER, RAILS_LP15V );
        // do other rails also
        // turn off dac outputs. relays.
      }

      // stay in error.
    }

    break;


    default:
      ;
  };

  // OK. setting it in the fpga works???


/*
  ok. so get the ref mux in. so we can test outputting a voltage.
  don't need ref. use siggen for ref?
  then do ads131.

*/



  count++;
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
      // soft_500ms = system_millis + 1000;
      soft_500ms = system_millis + 500;
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
  usart_printf("starting loop\n");
  usart_flush(); 
  // usart_printf("size %d\n", sizeof(fbuf) / sizeof(float));


  loop();

	for (;;);
	return 0;
}


