
// cjmcu  stm32f407.
// issue. is that board/stlink doesn't appear to reset cleanly. needs sleep.

// - can plug a thermocouple straight into the sense +p, +n, and
// then use the slow (digital) integrator/pid loop - as a temperature controler..

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

#include "core.h"   // some of the above files include core.h. need header guards.

#define UNUSED(x) (void)(x)

// here? or in mux.h.
#define SPI_ICE40       SPI1



static char buf1[1000];
static char buf2[1000];

static CBuf console_in;
static CBuf console_out;



static void update(void)
{

}

/*
  AHHH. is clever
    74hc125.
    if fpga gpio comes up high - OE - is disabled. so out is low.
    if fpga gpio comes up lo   - buffer is disabled so out low.

    there seems to be a 50-100uS spike though on creset.
    hopefully too short a time to turn on opto coupler.
*/


static float imultiplier = 0;
static float vmultiplier = 0;

static void current_range_set_1A(uint32_t spi)
{
  // 2V on 1A is 200mA

  // turn on current relay range X.
  io_set(spi, RELAY_COM_REGISTER, RELAY_COM_X);

  // turn on 1st b2b fets.
  io_set(spi, IRANGEX_SW_REGISTER, IRANGEX_SW1 | IRANGEX_SW2);

  // turn on current sense ina 1
  io_clear(spi, IRANGE_SENSE_REGISTER, IRANGE_SENSE1);

  io_clear(spi, GAIN_FB_REGISTER, GAIN_IFB_OP1 );

  // sense gain = 0.1x  ie. 0.1ohm sense resistor
  // ina gain x10.
  // extra amp gain = x10.

  imultiplier = 0.1f;
}

static void current_range_set_10A(uint32_t spi)
{
  // 10A is the same as 1A, except no 10x gain
  current_range_set_1A(spi);
  // TODO must use write()
  io_set(spi, GAIN_FB_REGISTER, GAIN_IFB_OP1 );
  imultiplier = 1.f;
}



static void current_range_set_100mA(uint32_t spi)
{
  // 2V on 100mA range should be 20mA.
  // 0.2V across 10ohm. g=10x, 0.2 / 10 = 0.02A = 20mA.
  // adc imultiplier should be 0.1.

  // TODO, these should all be write... not dependent on previous state/reset.
  // Careful. to do this, the separate gain for Vrange has to be split into another register, to avoid overwriting it.

  // turn on current relay range X.
  io_set(spi, RELAY_COM_REGISTER, RELAY_COM_X);

  // turn on 2nd b2b fets.
  io_set(spi, IRANGEX_SW_REGISTER, IRANGEX_SW3 | IRANGEX_SW4);

  // turn on current sense ina 2
  io_clear(spi, IRANGE_SENSE_REGISTER, IRANGE_SENSE2);

  imultiplier = 0.01f; // sense gain = x10 (10ohm) and x10 gain.

  // turn off gain fb.
  io_set(spi, GAIN_FB_REGISTER, GAIN_IFB_OP1 | GAIN_IFB_OP2);

  // turn on x10 gain, x100
  // io_clear(spi, GAIN_FB_REGISTER, GAIN_IFB_OP1 | GAIN_IFB_OP2);
}


static void voltage_range_set_10V(uint32_t spi)
{
  io_clear(spi, RELAY_REGISTER, RELAY_VRANGE ); // turn off vrange

  vmultiplier = 1.f;
  // also vgain..
}

static void voltage_range_set_100V(uint32_t spi)
{

  io_set(spi, RELAY_REGISTER, RELAY_VRANGE ); // turn on vrange

  // vimultiplier.

  vmultiplier = 10.f;
}



static void clamps_set_source_pve(uint32_t spi)
{
  // change name first_quadrant
  // sourcing, charging adc val 1.616501V
  // source +ve current/voltage.
  io_clear(spi, CLAMP1_REGISTER, CLAMP1_VSET_INV | CLAMP1_ISET_INV);
  io_clear(spi, CLAMP2_REGISTER, CLAMP2_MAX);
}



// ok. ads131. ought to be able to read value - without interrupt.
//

/*
  - current feedback appears working.
  - OK. we want ads to report current as well.
  - connect the output up to a led.
  - different valued sense resistor.
*/

/*
  there is no reason cannot have nested event driven fsm.  this is simple and works well.
  and there is no reason cannot have tasks in 500ms soft timer/ separate from main fsm state.
*/

/*
  we can test the mask write. just on the dg333. without analog power.
*/

static void soft_500ms_update(void)
{
  /*
    This is wrong. statemachine - should run in fast inner loop.
    but easier to test like this.
    and means don't oversample the ads. we haven't got interrupts yet.
  */

  // blink mcu led
  led_toggle();

  ////////
  // put this in spi1.h.  i think....
  uint32_t spi = SPI_ICE40;


  static uint32_t count = 0; // -1
  ++count;    // increment first. because state machine funcs return early.


  mux_io(spi);

  ////////////////////////////////
  // clear led1
  io_clear(spi, LED_REGISTER, LED1);



#if 0
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

  io_toggle(spi, LED_REGISTER, LED2);

  // tests
  // io_write(spi, CLAMP1_REGISTER, count);  // works
  // io_write(spi, CLAMP2_REGISTER, count);  // works
  // io_write(spi, RELAY_COM_REGISTER, count);
  // io_write(spi, IRANGEX_SW_REGISTER, count);
  // io_write(spi, IRANGE_SENSE_REGISTER, count);
  // io_write(spi, GAIN_FB_REGISTER, count);

  // io_toggle(spi, RELAY_COM_REGISTER, RELAY_COM_X);
  // io_toggle(spi, RELAY_REGISTER, RELAY_VRANGE);
  // io_toggle(spi, RELAY_REGISTER, RELAY_OUTCOM);
  // io_toggle(spi, RELAY_REGISTER, RELAY_SENSE);




  /*
    querying adc03 via spi, is slow (eg. we also clock spi slower to match read speed) .
    so it should only be done in soft timer eg. 10ms is probably enough.
    preferrably should offload to fpga with set voltages, -  and fpga can raise an interupt.
  */
  // get supply voltages,
  mux_adc03(spi);
  float lp15v = spi_mcp3208_get_data(spi, 0) * 0.92 * 10.;
  float ln15v = spi_mcp3208_get_data(spi, 1) * 0.81 * 10.;

  // usart_printf("lp15v %f    ln15v %f\n", lp15v, ln15v);

  int32_t ch; 
  while( (ch = cBufPop(&console_in)) >= 0) {

      usart_printf("got char %c \n", ch );
  }



  typedef enum state_t {
    FIRST,        // INITIAL
    DIGITAL_UP,   // DIGIAL_DIGITAL_UP
    ERROR,
    ANALOG_UP,
    DONE
  } state_t;

  // static
  // should probably be put in state record structure, rather than on the stack?
  static state_t state = FIRST;



  switch(state) {

    case FIRST:  {
      // if any of these fail, this should progress to error

      usart_printf("-----------\n");
      usart_printf("digital init start\n" );

      mux_io(spi);


      // io_clear(spi, CORE_SOFT_RST, 0);    // any value addressing this register.. to clear
      // no. needs dg444/mux stuff. pulled high. for off.

      // BUT I THINK we should probably hold RAILS_OE high / deasserted.

#if 1
      // REALLy need to rely on fpga reset, setting this stuff.

      // should we have wrapper functions here, can then put comments
      // make sure rails are off
      io_clear(spi, RAILS_REGISTER, RAILS_LP15V | RAILS_LP30V | RAILS_LP60V);

      // may as well keep rails OE deasserted, until really ready
      io_set(spi, RAILS_REGISTER, RAILS_OE);

      // turn off dac ref mux. pull-high, active lo.
      io_set( spi, DAC_REF_MUX_REGISTER, DAC_REF_MUX_A | DAC_REF_MUX_B);

      // turn off all clamp muxes, active lo.
      io_set(spi, CLAMP1_REGISTER, CLAMP1_VSET | CLAMP1_ISET | CLAMP1_ISET_INV | CLAMP1_VSET_INV);
      io_set(spi, CLAMP2_REGISTER, CLAMP2_MIN | CLAMP2_INJECT_ERR | CLAMP2_INJECT_VFB | CLAMP2_MAX);

      // active hi
      io_clear(spi, RELAY_COM_REGISTER, RELAY_COM_X | RELAY_COM_Y | RELAY_COM_Z);

      // adg1334, controlling b2b fets. wired to provide +-15V as needed.
      io_clear(spi, IRANGEX_SW_REGISTER, IRANGEX_SW1 | IRANGEX_SW2 | IRANGEX_SW3 | IRANGEX_SW4);

      // active hi
      io_clear(spi, RELAY_REGISTER, RELAY_VRANGE | RELAY_OUTCOM | RELAY_SENSE);

      // active lo
      io_set(spi, IRANGE_SENSE_REGISTER, IRANGE_SENSE1 | IRANGE_SENSE2 | IRANGE_SENSE3 | IRANGE_SENSE4);


      // gain fb. turn off ifb and vfb gain ,active hi
      io_set(spi, GAIN_FB_REGISTER, GAIN_VFB_OP1 | GAIN_VFB_OP2 | GAIN_IFB_OP1 | GAIN_IFB_OP2);
      // io_write_mask(spi, GAIN_FB_REGISTER, GAIN_IFB_OP1 | GAIN_IFB_OP2, GAIN_IFB_OP1 | GAIN_IFB_OP2);


#if 0
      // change name GAIN_IFB_OP1 ... GAIN_VFB_OP2   etcc
      // eg. clear ifb regs.
      io_write_mask(spi, GAIN_FB_REGISTER, GAIN_IFB_OP1 | GAIN_IFB_OP2, GAIN_IFB_OP1 | GAIN_IFB_OP2);
      io_write_mask(spi, GAIN_FB_REGISTER, GAIN_VFB_OP1 | GAIN_VFB_OP2,  GAIN_VFB_OP1 | GAIN_VFB_OP2);
      io_write_mask(spi, GAIN_FB_REGISTER, GAIN_IFB_OP1 | GAIN_IFB_OP2, 0 );
      state = DONE;
      return;
#endif



      // TODO soft reset would be much better here.
      //  make sure fpga can configure initial state.
      // we must turn everything off. or else issue a soft reset.
#endif
      // test the flash
      // TODO. check responses.
      mux_w25(spi);
      spi_w25_get_data(spi);

      // dac init
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


        usart_printf("turn on analog rails - lp15v\n" );
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

        // EXTREME. feedback is always negative. why we just plug vfb and ifb without inverses.
        // its easier to think of everything without polarity.   (the polarity just exists because we tap/ com at 0V).

        // turn on set voltages 2V and 4V outputs. works.
        spi_dac_write_register(spi, DAC_VSET_REGISTER, voltage_to_dac( 10 ) ); // // we cannot output 12V with 15V rails ...
        // spi_dac_write_register(spi, DAC_VSET_REGISTER, voltage_to_dac( 4 ) );
        // spi_dac_write_register(spi, DAC_ISET_REGISTER, voltage_to_dac( 3.0) ); // 30mA on 100mA..
        spi_dac_write_register(spi, DAC_ISET_REGISTER, voltage_to_dac( 10.0) ); // 0.5A on 1A range.. overheats bjt.
        // spi_dac_write_register(spi, DAC_ISET_REGISTER, voltage_to_dac( 1.0) ); // 100mA on 1A... hot. because drops 100mA over 15V=1.5W.

        // 100mA*15V=1.5W.   think mje15034g is rated at 2W without heatsink.

        // can probably increase current. 12V out. 15-12V=  3V* 2A=6W... no thats even more lot.
        // needs a heatsink.


        /*  none of this works.
            Because, V MON pin output impedance is too low. and needs a buffer. (approximately 2.2kΩ).
        */
        // spi_dac_write_register(spi, DAC_MON_REGISTER, 0 );
        // spi_dac_write_register(spi, DAC_MON_REGISTER, 0 DAC_MON_MDAC0  );
        // spi_dac_write_register(spi, DAC_MON_REGISTER, DAC_MON_AIN );
        // spi_dac_write_register(spi, DAC_MON_REGISTER, DAC_MON_MDAC0  );
        // spi_dac_write_register(spi, DAC_MON_REGISTER, DAC_MON_MDAC1  );


        //////////////////////////////////
        // set up clamps
        mux_io(spi);

    #if 0
        // source -ve current/voltage.
        // should be write(  ~CLAMP1_VSET | CLAMP1_ISET) etc.
        io_clear(spi, CLAMP1_REGISTER, CLAMP1_VSET | CLAMP1_ISET);    // inv for +ve voltage, non invert for negative
        io_clear(spi, CLAMP2_REGISTER, CLAMP2_MIN);             // max.   for source +ve voltage, min for source -ve voltage
    #endif

    #if 0
        // sourcing, charging adc val 1.616501V
        // source +ve current/voltage.
        io_clear(spi, CLAMP1_REGISTER, CLAMP1_VSET_INV | CLAMP1_ISET_INV);
        io_clear(spi, CLAMP2_REGISTER, CLAMP2_MAX);
    #endif


#if 0
        // OK. think its sinking current. 2V over 1k = i=v/r = 2mA. eg. battery discharging.  1.591694V
        // but I don't think V is set correctly
        // except I think V set would have to be negative as well to hold.
        // not sure. set it to 1V and it works. but it goes out of range?
        io_clear(spi, CLAMP1_REGISTER, CLAMP1_VSET_INV | CLAMP1_ISET);
        io_clear(spi, CLAMP2_REGISTER, CLAMP2_MAX);
#endif


        // io_set(spi, RELAY_REGISTER, RELAY_VRANGE ); // turn on vrange register

        // should ok for 12V. perhaps we're running into a limit of voltage drops on output

        // so. should try connecting a higher voltage output. or add 7815 regulation. and turn up bench?

        // 100V range (eg. using 221k resisters
        //  8V output.
        //                  unloaded  8.0861   disconnected.
        //                  loaded    8.0859     OK.
        //   11V output       11.092 loaded  or unloaded. good.
        // 10V
        // 8V range
        //                  unloaded    8.0226  disconnected.
        //                  loaded      8.0225V  great.

        // issue - set and measuring 11V.    but meter reads 10V? 




        clamps_set_source_pve(spi);

        // voltage_range_set_100V(spi);        // 1.2 / 10 * 100 = 12V output. good.
        voltage_range_set_10V(spi);

        // current_range_set_100mA(spi);
        current_range_set_1A(spi);

        // turn on output relay
        io_set(spi, RELAY_REGISTER, RELAY_OUTCOM);


        /////////////////
        // adc init has to be done after rails are up...
        // adc init
        int ret = adc_init(spi, ADC_REGISTER);
        if(ret != 0) {
          state = ERROR;
          return;
        }

        usart_printf("analog init ok\n" );
        // maybe change name RAILS_OK, RAILS_UP ANALOG_OK, ANALOG_UP

        // turn on power rails
        // effectively turn on output
#if 1
        ////////////////////
        // power rails
        usart_printf("turn on power rails - lp30v\n" );
        mux_io(spi);
        io_set(spi, RAILS_REGISTER, RAILS_LP30V );
        msleep(50);
#endif

        // analog and power... change name?
        state = ANALOG_UP;
      }
      break ;


    case ANALOG_UP:
      if((lp15v < 14.7 || ln15v < 14.7)  ) {

        mux_io(spi);
        usart_printf("supplies bad - turn off rails\n");
        usart_printf("lp15v %f    ln15v %f\n", lp15v, ln15v);

        // turn off power
        io_clear(spi, RAILS_REGISTER, RAILS_LP15V | RAILS_LP30V | RAILS_LP60V);

        // go to state error
        state = ERROR;
      }

      // ... ok.
      // how to return. pass by reference...
      float ar[4];
      spi_adc_do_read(spi, ar, 4);

      // current in amps.
      // til, we format it better
      // %f formatter, doesn't pad with zeros properly...
      // why is the voltage *10?
      // Force=Potential=3V, etc.
      usart_printf("adc %fV    %fA\n",
        ar[0] / 1.64640 * vmultiplier,
        ar[1] / 1.64640 * imultiplier
      );

      // we want to go to another state here... and bring up POWER_UP...

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


    case DONE: {
      // same as error, but different reporting
      static int first = 0;
      if(!first) {
        first = 1;
        usart_printf("entered done state\n" );
        io_clear(spi, RAILS_REGISTER, RAILS_LP15V);
      }
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

}




static void loop(void)
{

  static uint32_t soft_500ms = 0;

  while(true) {

    // EXTREME - could actually call update at any time, in a yield()...
    // so long as we wrap calls with a mechanism to avoid stack reentrancy
    // led_update(); in systick.
    // but better just to flush() cocnsole queues.conin/out


    // usart_input_update();
    usart_output_update();


    // 500ms soft timer
    if( system_millis > soft_500ms) {
      // soft_500ms = system_millis + 1000;
      soft_500ms = system_millis + 500;
      soft_500ms_update();
    }


    update();
  }
}



/*
typedef struct App
{
  // state tree.


} App;
*/



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

  // uart/console
  cBufInit(&console_in,  buf1, sizeof(buf1));
  cBufInit(&console_out, buf2, sizeof(buf2));

  usart_setup_gpio_portA();
  usart_setup(&console_in, &console_out);
  usart_printf_init(&console_out);


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


