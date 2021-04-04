
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



#define UNUSED(x) (void)(x)



//////////////////////////////////////////////

// REGISTER_DAC?

// change name REG_DAC not DAC_REGISTER etc

// what the hell is happening...

// RENAME OR = output register and SRR set reset register

// OK. Now is there a way to do both...

#define LED_REGISTER  7
#define LED1 (1<<0)    // D38
#define LED2 (1<<1)    // D37
#define LED3 (1<<2)    // D37
#define LED4 (1<<3)    // D37


#define SPI_MUX_REGISTER  8
#define SPI_MUX_ADC03     (1<<0)
#define SPI_MUX_DAC       (1<<1)
#define SPI_MUX_FLASH     (1<<2)

/////

#define DAC_REGISTER  9
#define DAC_LDAC      (1<<0)
#define DAC_UNI_BIP_A (1<<1)
#define DAC_UNI_BIP_B (1<<2)
#define DAC_RST       (1<<3)

// rename RAILS_REG or REG_RAILS consistent with verilog?
#define RAILS_REGISTER  10
#define RAILS_LP15V   (1<<0)
#define RAILS_LP30V   (1<<1)
#define RAILS_LP60V   (1<<2)
#define RAILS_OE      (1<<3)

// 12 is soft reset

#define DAC_REF_MUX_REGISTER  12
#define DAC_REF_MUX_A     (1<<0)
#define DAC_REF_MUX_B     (1<<1)




// one bit


static void mux_fpga(uint32_t spi)
{
  spi_ice40_reg_setup(spi);
  // spi uses the special flag to communicate
}


static void mux_adc03(uint32_t spi)
{
  spi_ice40_reg_setup(spi);
  spi_ice40_reg_write(spi, SPI_MUX_REGISTER, SPI_MUX_ADC03);
  spi_mcp3208_setup(spi);
}

static void mux_w25(uint32_t spi)
{
  spi_ice40_reg_setup(spi);
  spi_ice40_reg_write(spi, SPI_MUX_REGISTER, SPI_MUX_FLASH);
  spi_w25_setup(spi);
}


static void mux_dac(uint32_t spi)
{
  spi_ice40_reg_setup(spi);
  spi_ice40_reg_write(spi, SPI_MUX_REGISTER, SPI_MUX_DAC);
  spi_dac_setup(spi);
}


/*
// Must check again. that
  ok. dac does not need analog voltage. to initialize.
  so can bring up first.bring up analog

  we need 2.5V ref.   for ads131. so need to do that.
*/


static uint32_t dac_init(uint32_t spi)  // bad name?
{
  // if something goes wrong we should not proceed.

  mux_fpga(spi);
  // dac_setup( spi );
  // keep latch low, and unused, unless chaining
  // need to set unipolar/bipolar
  spi_ice40_reg_clear(spi, DAC_REGISTER, DAC_LDAC);

  // unipolar output on a
  spi_ice40_reg_set(spi, DAC_REGISTER, DAC_UNI_BIP_A /*| DAC_UNIBIPB */);


  // dac reset
  usart_printf("doing dac reset\n");
  spi_ice40_reg_clear(spi, DAC_REGISTER, DAC_RST);
  msleep(20);
  spi_ice40_reg_set( spi, DAC_REGISTER, DAC_RST);
  msleep(20);


  // see if we can toggle the dac gpio0 output
  mux_dac(spi);
  uint32_t u1 = spi_dac_read_register(spi, 0);
  // usart_printf("read %d \n", u1 );
  usart_printf("gpio0 set %d \n", (u1 & DAC_GPIO0) != 0 ); // TODO use macro for GPIO0 and GPIO1 // don't need == here
  usart_printf("gpio1 set %d \n", (u1 & DAC_GPIO1) != 0);



  // startup has the gpio bits set.
  // spi_dac_write_register(spi, 0, DAC_GPIO0 | DAC_GPIO1); // measure 0.1V. eg. high-Z without pu.
  spi_dac_write_register(spi, 0, 0 );                 // measure 0V

  uint32_t u2 = spi_dac_read_register(spi, 0);
  // usart_printf("read %d \n", u2 );
  usart_printf("gpio0 set %d \n", (u2 & DAC_GPIO0) != 0);
  usart_printf("gpio1 set %d \n", (u2 & DAC_GPIO1) != 0);

  /* OK. to read gpio0 and gpio1 hi vals. we must have pullups.
     note. also means they can effectively be used bi-directionally.
  */

  if(u1 == u2) {
    // toggle not ok,
    usart_printf("dac toggle gpio not ok\n" );
    return -1;
  }

  // toggle ok,
  usart_printf("turning on ref a\n" );
  mux_fpga(spi);
  spi_ice40_reg_clear( spi, DAC_REF_MUX_REGISTER, DAC_REF_MUX_A);


  mux_dac(spi);

  spi_dac_write_register(spi, DAC_VSET_REGISTER, 12345);
  msleep( 1);
  uint32_t u = spi_dac_read_register(spi, DAC_VSET_REGISTER) ;

  // usart_printf("u is %d\n", u );
  usart_printf("v set register val %d\n", u & 0xffff );
  usart_printf("v set register is %d\n", (u >> 16) & 0x7f);

  if( (u & 0xffff) != 12345) {
    usart_printf("dac setting reg not ok\n" );
    return -1;
  }

  // should go to failure... and return exit...
  usart_printf("write vset ok\n");

    // clear it.
  spi_dac_write_register(spi, DAC_VSET_REGISTER, 0);

  return 0;
}





static void soft_500ms_update(void)
{
  // blink mcu led
  led_toggle();

  ////////
  uint32_t spi = SPI_ICE40;


  static int count = 0;


  mux_fpga(spi);

#if 1
  ////////////////////////////////
  // clear led1
  spi_ice40_reg_clear(spi, LED_REGISTER, LED1);

  // toggle led2
  if(count % 2 == 0  )
    spi_ice40_reg_set(spi, LED_REGISTER, LED2);
  else
    spi_ice40_reg_clear(spi, LED_REGISTER, LED2);
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

      uint32_t ret = dac_init(spi) ; // bad name?
      // progress to error if failed.
      if(ret != 0) {
        state = ERROR;
        return;
      }

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

        // set 2V and 4V outputs. works.
        // spi_dac_write_register(spi, DAC_VSET_REGISTER, voltage_to_dac( 2.0) );
        // spi_dac_write_register(spi, DAC_ISET_REGISTER, voltage_to_dac( 4.0) );
#endif
        // power up sequence
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


