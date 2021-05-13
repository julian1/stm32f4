
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
#include <string.h>   // strcmp


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
  // 2V on 1A is 200mA, 5V is 0.5A
  // sense gain = 0.1x  ie. 0.1ohm sense resistor
  // ina gain x10.
  // extra amp gain = x10.

  // write() writes all the bits.

  // turn on current relay range X.
  io_write(spi, RELAY_COM_REGISTER, RELAY_COM_X);

  // turn off other fets
  io_write(spi, IRANGEX_SW58_REGISTER, 0);

  // turn on 1st b2b fets.
  io_write(spi, IRANGEX_SW_REGISTER, IRANGEX_SW1 | IRANGEX_SW2);

  // turn on current sense ina 1
  io_write(spi, IRANGE_SENSE_REGISTER, ~IRANGE_SENSE1);

  // active lo. turn on ifb gain op1, x10
  io_write(spi, GAIN_IFB_REGISTER, ~GAIN_IFB_OP1);


  imultiplier = 0.1f;
}

static void current_range_set_10A(uint32_t spi)
{
  // 0.1ohm
  // 300mV=3A across 0.1R sense.   could use 3.33 (10/3.3) gain after ina to get to 0-10V..
  // 1 / ( 1 +  2 )  = 0.3333333333333333
  // = divider with r1=1 and r2=2. eg. a 2 to 1.
  // eg. make op2 be

  // 10A is the same as 1A, except no 10x gain
  current_range_set_1A(spi);

  // active lo. turn off both ifb gain stages...
  // using 10x gain from ina, on 0.1R only.
  io_write(spi, GAIN_IFB_REGISTER, GAIN_IFB_OP1 | GAIN_IFB_OP2 );

  imultiplier = 1.0f;
}



static void current_range_set_100mA(uint32_t spi)
{
  // 10ohm.
  // 2V on 100mA range should be 20mA.
  // 0.2V across 10ohm. g=10x, 0.2 / 10 = 0.02A = 20mA.
  // adc imultiplier should be 0.1.

  // turn on current relay range X.
  io_write(spi, RELAY_COM_REGISTER, RELAY_COM_X);

  // turn off other fets
  io_write(spi, IRANGEX_SW58_REGISTER, 0);

  // turn on 2nd b2b fets.
  io_write(spi, IRANGEX_SW_REGISTER, IRANGEX_SW3 | IRANGEX_SW4);


  // active lo, current sense 2
  io_write(spi, IRANGE_SENSE_REGISTER, ~IRANGE_SENSE2);

  // active lo. turn off both current gain ops
  io_write(spi, GAIN_IFB_REGISTER, GAIN_IFB_OP1 | GAIN_IFB_OP2 );

  imultiplier = 0.01f; // sense gain = x10 (10ohm) and x10 gain.
}

#if 0
static void current_range_set_none(uint32_t spi)
{
  // this won't work. without a current sense resistor, its an open circuit.

  // clear all current relays
  io_write(spi, RELAY_COM_REGISTER, 0 );

  // turn off other fets, switches
  io_write(spi, IRANGEX_SW58_REGISTER, 0);
  io_write(spi, IRANGEX_SW_REGISTER, 0);
}
#endif





static void current_range_set_10mA(uint32_t spi)
{
  // UNUSED(spi);
  /*
      10V = 0.01A * 1k.
      = 100mW.      maybe ok.  with high-watt 1
     1x gain.
  */


  // turn on current relay range X.
  io_write(spi, RELAY_COM_REGISTER, RELAY_COM_X);

  // turn off other fets
  io_write(spi, IRANGEX_SW_REGISTER, 0);

  // turn on 3rd b2b fets.
  io_write(spi, IRANGEX_SW58_REGISTER, IRANGEX_SW5 | IRANGEX_SW6);

  // active lo, current sense 3
  io_write(spi, IRANGE_SENSE_REGISTER, ~IRANGE_SENSE3);

  // active lo, turn off both current gain ops
  io_write(spi, GAIN_IFB_REGISTER, GAIN_IFB_OP1 | GAIN_IFB_OP2 );

  imultiplier = 0.001f;
}




////////////////////////////



static void voltage_range_set_100V(uint32_t spi)
{
  // now using ina 143. with 1:10 divide by default

  io_clear(spi, RELAY_REGISTER, RELAY_VRANGE ); // no longer used. must be off.


  // active lo, turn both vfb gain stages off
  io_write(spi, GAIN_VFB_REGISTER, GAIN_VFB_OP1 | GAIN_VFB_OP2 );

  vmultiplier = 10.f;
}


static void voltage_range_set_10V(uint32_t spi)
{
  // now using ina 143. with 1:10 divide by default

  io_clear(spi, RELAY_REGISTER, RELAY_VRANGE ); // no longer used. must be off.

  // active lo. turn on OP1
  io_write(spi, GAIN_VFB_REGISTER, ~GAIN_VFB_OP1 );

  vmultiplier = 1.f;
}


static void voltage_range_set_1V(uint32_t spi)
{
  // now using ina 143. with 1:10 divide by default

  io_clear(spi, RELAY_REGISTER, RELAY_VRANGE ); // no longer used. must be off.

  // active lo.  turn on both OP1 and OP2
  io_write(spi, GAIN_VFB_REGISTER, ~(GAIN_VFB_OP1 | GAIN_VFB_OP2) );

  vmultiplier = 0.1f;
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



// TODO prefix these... ST_
// also want a DONE state.

typedef enum state_t {
  FIRST,        // INITIAL
  DIGITAL_UP,   // DIGIAL_DIGITAL_UP
  ERROR,
  ANALOG_UP,
  HALT
} state_t;

// static
// should probably be put in state record structure, rather than on the stack?
// except would need to pass by reference.
static state_t state = FIRST;





static void update_soft_500ms(uint32_t spi  /*, state */)
{


  static uint32_t count = 0; // -1
  ++count;    // increment first. because state machine funcs return early.


  // blink mcu led
  led_toggle();

  mux_io(spi);

  ////////////////////////////////
  // clear led1
  io_clear(spi, LED_REGISTER, LED1);


  io_toggle(spi, LED_REGISTER, LED2);

#if 0
  mux_adc03(spi);
  float lp15v = spi_mcp3208_get_data(spi, 0) * 0.92 * 10.;
  float ln15v = spi_mcp3208_get_data(spi, 1) * 0.81 * 10.;
  usart_printf("lp15v %f    ln15v %f\n", lp15v, ln15v);
#endif


 // tests
  // io_write(spi, CLAMP1_REGISTER, count);  // works
  // io_write(spi, CLAMP2_REGISTER, count);  // works
  // io_write(spi, RELAY_COM_REGISTER, count);
  // io_write(spi, IRANGEX_SW_REGISTER, count);
  // io_write(spi, IRANGE_SENSE_REGISTER, count);


  // io_write(spi, GAIN_IFB_REGISTER, count);
  // io_write(spi, GAIN_VFB_REGISTER, count);

  // test

  // usart_printf("count %d\n", count);
  // io_write(spi, IRANGEX_SW58_REGISTER, count);

  // io_toggle(spi, RELAY_COM_REGISTER, RELAY_COM_X);
  // io_toggle(spi, RELAY_REGISTER, RELAY_VRANGE);
  // io_toggle(spi, RELAY_REGISTER, RELAY_OUTCOM);
  // io_toggle(spi, RELAY_REGISTER, RELAY_SENSE);


  switch(state) {


    case ANALOG_UP: {

#if 0
      // ... ok.
      // how to return. pass by reference...
      float ar[4];
      spi_adc_do_read(spi, ar, 4);

      // current in amps.
      // til, we format it better
      // %f formatter, doesn't pad with zeros properly...
      // why is the voltage *10?
      // Force=Potential=3V, etc.
      usart_printf("adc %f V    %fA\n",
        ar[0] / 1.64640 * vmultiplier,
        ar[1] / 1.64640 * imultiplier
      );
#endif

#if 0
      usart_printf("adc %dV    %dA\n",
        ar[0] ,
        ar[1]
      );
#endif

      break;
    }

    default: ;
  };
}


/*

*/

static void update_console_cmd(uint32_t spi, CBuf *console_in, CBuf* console_out, CBuf *cmd_in )
{
  // needs to switch state.
  // and needs a buffer for local commands...

  UNUSED(spi);

  // OK. by not processing chars as we receive, we have lost character echo...

  int32_t ch;

  while( (ch = cBufPop(console_in)) >= 0) {
    // got a character

    // copy to command buffer
    cBufPut(cmd_in, ch);

    // echo to output, handling newlines...
    if(ch == '\r') {
      cBufPut(console_out, '\n');
    }

    cBufPut(console_out, ch);
  }


  if(cBufPeekLast(cmd_in) == '\r') {

    // we got a carriage return
    static char tmp[1000];
    size_t n = cBufCopy(cmd_in, tmp, sizeof(tmp));
    tmp[n - 1] = 0;   // drop tailing line feed

    usart_printf("got command '%s'   %d\n", tmp, n);


    if(strcmp(tmp, "halt") == 0) {
      // go to halt state
      usart_printf("switch off\n");
      state = HALT;
      return;
    }


    if(strcmp(tmp, "off") == 0) {
      usart_printf("switch off\n");

      // turn off relayc
      mux_io(spi);
      io_clear(spi, RELAY_REGISTER, RELAY_OUTCOM);
      return;
    }

    if(strcmp(tmp, "on") == 0) {
      usart_printf("switch on\n");
      mux_io(spi);
      io_set(spi, RELAY_REGISTER, RELAY_OUTCOM);
      return;
    }

    // need to be able to turn on/off adc reporting. and perhaps speed.
    // actually an entire state tree...

  }



}


static void update(uint32_t spi)
{
  // called as often as possible


  // mux_io(spi);
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
      io_clear(spi, RAILS_REGISTER, RAILS_LP5V | RAILS_LP15V | RAILS_LP30V | RAILS_LP60V);

      // may as well keep rails OE deasserted, until really ready
      io_set(spi, RAILS_OE_REGISTER, RAILS_OE); // actie lo



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
      io_set(spi, GAIN_IFB_REGISTER, GAIN_IFB_OP1 | GAIN_IFB_OP2);
      // io_write_mask(spi, GAIN_FB_REGISTER, GAIN_IFB_OP1 | GAIN_IFB_OP2, GAIN_IFB_OP1 | GAIN_IFB_OP2);


      // adg1334. x range, b2b fets, lo is off.
      io_clear(spi, IRANGEX_SW58_REGISTER, IRANGEX_SW5 | IRANGEX_SW6 | IRANGEX_SW7 | IRANGEX_SW8);


      io_set(spi, GAIN_VFB_REGISTER, GAIN_VFB_OP1 | GAIN_VFB_OP2 );


#if 0
      // change name GAIN_IFB_OP1 ... GAIN_VFB_OP2   etcc
      // eg. clear ifb regs.
      io_write_mask(spi, GAIN_FB_REGISTER, GAIN_IFB_OP1 | GAIN_IFB_OP2, GAIN_IFB_OP1 | GAIN_IFB_OP2);
      io_write_mask(spi, GAIN_FB_REGISTER, GAIN_VFB_OP1 | GAIN_VFB_OP2,  GAIN_VFB_OP1 | GAIN_VFB_OP2);
      io_write_mask(spi, GAIN_FB_REGISTER, GAIN_IFB_OP1 | GAIN_IFB_OP2, 0 );
      state = HALT;
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

#if 0
      // dac init
      int ret = dac_init(spi, DAC_REGISTER); // bad name?
      if(ret != 0) {
        state = ERROR;
        return;
      }
#endif

      // progress to digital up?
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


        usart_printf("whoot2 turn on analog rails - lp15v\n" );
        mux_io(spi);
        // assert rails oe
        io_clear(spi, RAILS_OE_REGISTER, RAILS_OE);

        // turn on +-15V analog rails
        // io_SET should be flipping single bit. 
   /* 
        io_set(spi, RAILS_REGISTER, RAILS_LP15V );
        io_set(spi, RAILS_REGISTER, RAILS_LP5V );
        io_set(spi, RAILS_REGISTER, RAILS_LP30V );
        io_set(spi, RAILS_REGISTER, RAILS_LP60V );
   */ 

        io_set(spi, RAILS_REGISTER, RAILS_LP5V | RAILS_LP15V | RAILS_LP30V | RAILS_LP60V );
        msleep(50);

        // signal integrity?

#if 0

        // turn on refs for dac
        //mux_dac(spi);
        usart_printf("turn on ref a for dac\n" );
        mux_io(spi);
        io_clear(spi, DAC_REF_MUX_REGISTER, DAC_REF_MUX_A); // active lo

        // EXTREME. feedback is always negative. why we just plug vfb and ifb without inverses.
        // its easier to think of everything without polarity.   (the polarity just exists because we tap/ com at 0V).

        // turn on set voltages 2V and 4V outputs. works.

        /*
          OK. can talk to fpga for io, or peripheral, without having to intersperse calls to mux_io() and mux_dac()
            special is asserted for io.
            ---
            but issue is the spi parameters might change for ice40 versus peripheral.
            use a second channel. and it would work.
        */
        //////////////////////////////////
        // set up clamps

        mux_io(spi);
        clamps_set_source_pve(spi);

        // WE DO need the mux() calls. to setup the spi parameters which may differ.
        // sometimes it looks like we don't because they use the *same* clock polarity.

        // voltage
        mux_dac(spi);
        spi_dac_write_register(spi, DAC_VSET_REGISTER, voltage_to_dac( 1.f ) ); // 10V

        mux_io(spi);
        // voltage_range_set_100V(spi);       // ie. 1.2  = 12V, 1.5=15V etc
        voltage_range_set_10V(spi);           // ie 1.2 = 1.2V
        // voltage_range_set_1V(spi);         // ie 1.2 = 0.12V

        // current
        mux_dac(spi);
        spi_dac_write_register(spi, DAC_ISET_REGISTER, voltage_to_dac( 1.f ) );  // 5.f

        mux_io(spi);
        // current_range_set_10A(spi);           // ie 1=1A, 0.5=0.5A, 0.1=0.1V
        // current_range_set_1A(spi);         // ie. 1=0.1A,10=1A
        // current_range_set_100mA(spi);      // 1=10mA, 10=100mA.
        current_range_set_10mA(spi);          // 1=1mA, 10=100mA.
        // current_range_set_none(spi);       // won't work. there's no circuit.

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
        // io_set(spi, RAILS_REGISTER, RAILS_LP30V );
        io_set(spi, RAILS_REGISTER, RAILS_LP60V );  // actually 15V
        msleep(50);
#endif

        // analog and power... change name?

#endif
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

        // turn off output relay
        io_clear(spi, RELAY_REGISTER, RELAY_OUTCOM);

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

        // turn off output relay
        io_clear(spi, RELAY_REGISTER, RELAY_OUTCOM);
        // turn off all power
        io_clear(spi, RAILS_REGISTER, RAILS_LP15V | RAILS_LP30V | RAILS_LP60V);
      }
      // stay in error.
    }
    break;


    case HALT: {

      static int first = 0;
      if(!first) {
        first = 1;
        usart_printf("entered halt state\n" );

        mux_io(spi);
        // turn off output relay
        io_clear(spi, RELAY_REGISTER, RELAY_OUTCOM);
        // turn off all power
        io_clear(spi, RAILS_REGISTER, RAILS_LP15V | RAILS_LP30V | RAILS_LP60V);
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



// should pass the console to routines that need it...
static char buf1[1000];
static char buf2[1000];

static CBuf console_in;
static CBuf console_out;



static char buf3[1000];
static CBuf cmd_in;


static void loop(void)
{

  static uint32_t soft_500ms = 0;

  ////////
  // put this in spi1.h.  i think....
  uint32_t spi = SPI_ICE40;



  while(true) {

    // EXTREME - could actually call update at any time, in a yield()...
    // so long as we wrap calls with a mechanism to avoid stack reentrancy
    // led_update(); in systick.
    // but better just to flush() cocnsole queues.conin/out


    // usart_input_update();
    usart_output_update();

    // update_console_cmd(spi, &console_in);
    update_console_cmd(spi, &console_in, &console_out, &cmd_in);

    update(spi);

    // 500ms soft timer
    if( system_millis > soft_500ms) {
      soft_500ms = system_millis + 500;
      update_soft_500ms( spi );
    }


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


  cBufInit(&cmd_in, buf3, sizeof(buf3));


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


