
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
#include "ice40.h"
#include "mcp3208.h"
#include "w25.h"
#include "dac8734.h"
#include "ads131a04.h"

#include "mux.h"







static void soft_500ms_update(void)
{
  // blink mcu led
  led_toggle();

  ////////
  // put this in spi1.h.  i think....
  uint32_t spi = SPI_ICE40;


  static int count = 0;


  mux_fpga(spi);

#if 1
  ////////////////////////////////
  // clear led1
  spi_ice40_reg_clear(spi, LED_REGISTER, LED1);

  // would be nice to have a toggle function.

  // toggle led2
  if(count % 2 == 0) {
    spi_ice40_reg_set(spi, LED_REGISTER, LED2);
    // spi_ice40_reg_set(spi, ADC_REGISTER, ADC_RST);
  }
  else {
    spi_ice40_reg_clear(spi, LED_REGISTER, LED2);
    // spi_ice40_reg_clear(spi, ADC_REGISTER, ADC_RST);
  }
#endif

  // DO we want to do this every loop?
  // get supply voltages,
  mux_adc03(spi);
  float lp15v = spi_mcp3208_get_data(spi, 0) * 0.92 * 10.;
  float ln15v = spi_mcp3208_get_data(spi, 1) * 0.81 * 10.;





  typedef enum state_t {
    FIRST,    // INITIAL
    INITIALIZED,  // DIGIAL_INITIALIZED
    ERROR,
    RAILSUP
  } state_t;

  // static
  static state_t state = FIRST;



  switch(state) {

    case FIRST:  {
      // if any of these fail, this should progress to error

      mux_fpga(spi);

      // make sure rails are off
      spi_ice40_reg_clear(spi, RAILS_REGISTER, RAILS_LP15V | RAILS_LP30V | RAILS_LP60V);

      // may as well keep rails OE deasserted, until really ready
      spi_ice40_reg_set(spi, RAILS_REGISTER, RAILS_OE);

      // turn off dac ref mux. pull-high
      spi_ice40_reg_set( spi, DAC_REF_MUX_REGISTER, DAC_REF_MUX_A | DAC_REF_MUX_B);

      // test the flash
      mux_w25(spi);
      spi_w25_get_data(spi);

      // init dac.
      uint32_t ret = dac_init(spi); // bad name?
      if(ret != 0) {
        state = ERROR;
        return;
      }

      // need to turn on rails before can init adc

      // done initialization
      state = INITIALIZED;
      break;
    }


    case INITIALIZED:
      if( lp15v > 15.0 && ln15v > 15.0 )
      {

        usart_printf("-----------\n");

        usart_printf("lp15v %f    ln15v %f\n", lp15v, ln15v);
        usart_printf("supplies ok - turning on rails\n");

#if 1
        mux_fpga(spi);
        // assert rails oe
        spi_ice40_reg_clear(spi, RAILS_REGISTER, RAILS_OE);

        // turn on +-15V analog rails
        spi_ice40_reg_set(spi, RAILS_REGISTER, RAILS_LP15V );
        msleep(50);


        // turn on refs for dac
        mux_dac(spi);
        usart_printf("turning on ref a\n" );
        mux_fpga(spi);
        spi_ice40_reg_clear( spi, DAC_REF_MUX_REGISTER, DAC_REF_MUX_A);


        // turn on set voltages 2V and 4V outputs. works.
        spi_dac_write_register(spi, DAC_VSET_REGISTER, voltage_to_dac( 2.0) );
        spi_dac_write_register(spi, DAC_ISET_REGISTER, voltage_to_dac( 4.0) );
#endif

        /////////////////
        // adc init has to be done after rails are up...
        // init adc
        adc_init(spi);



        // power up sequence complete
        state = RAILSUP;



      }
      break ;

    case RAILSUP:
      if((lp15v < 14.7 || ln15v < 14.7)  ) {

        mux_fpga(spi);
        usart_printf("supplies bad - turn off rails\n");
        usart_printf("lp15v %f    ln15v %f\n", lp15v, ln15v);

        // turn off power
        spi_ice40_reg_clear(spi, RAILS_REGISTER, RAILS_LP15V );

        // go to state error
        state = ERROR;
      }
      break;
/*
      // timeout to turn off...
      if( system_millis > timeout_off_millis) {
        state = INIT;
        usart_printf("timeout - turning off \n");
      }
 */

    default:
      ;
  };

  // OK. setting it in the fpga works???


/*
  ok. so get the ref mux in. so we can test outputting a voltage.
  don't need ref. use siggen for ref?
  then do ads131.

*/


  /*
    AHHH. is clever
      74hc125.
      if fpga gpio comes up high - OE - is disabled. so out is low.
      if fpga gpio comes up lo   - buffer is disabled so out low.

      there seems to be a 50-100uS spike though on creset.
      hopefully too short a time to turn on opto coupler.
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
  usart_printf("starting\n");
  // usart_printf("size %d\n", sizeof(fbuf) / sizeof(float));


  loop();

	for (;;);
	return 0;
}


