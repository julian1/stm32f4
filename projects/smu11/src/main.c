
// vim :colorscheme default. loooks good.

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
#include <math.h> // nanf   fabs
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





// TODO prefix these... ST_FIRST... etc.
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
// static state_t state = FIRST;




/*
  VERY IMPORTANT.
  we need to iterate all the ranges. even if we don't use them. so that can test them.
  A switch statement. is almost certainly going to be easier separate functions.
*/


typedef enum vrange_t
{
  vrange_none,
  vrange_10V,
  vrange_1V,
  vrange_100mV,
  vrange_100V // ,

  // vrange_10V_2,

} vrange_t;



typedef enum irange_t
{
  // TODO rename range_current_none, range_current_1x etc.
  irange_none,
/*
  irange_1x,
  irange_10x,
  irange_100x,
*/

  irange_100uA,
  irange_1mA,
  irange_10mA,
  irange_100mA,
  irange_1A

} irange_t;



typedef enum format_t
{
  format_mV,
  format_V,

  format_uA,
  format_mA,
  format_A,

} format_t;






typedef struct app_t
{
  uint32_t spi;

  state_t   state;

  vrange_t  vrange;
  irange_t  irange;

  bool      print_adc_values;


  // bool      last_char_newline; // last console char
  // we could eliminate this. if we were to read the relay register...
  bool      output;   // whether output on/off

} app_t;




static void output_set(app_t *app, irange_t irange, uint8_t val);




static void range_voltage_set(app_t *app, vrange_t vrange)
{

  app->vrange = vrange;

  switch(app->vrange)
  {

    case vrange_10V:
      usart_printf("10V range \n");
      // flutters at 5 digit. nice.
      // 6th digit. with 9V and 0V.
      io_write(app->spi, REG_INA_VFB_ATTEN_SW, INA_VFB_ATTEN_SW1_CTL);       // no atten
      io_write(app->spi, REG_INA_VFB_SW, ~INA_VFB_SW1_CTL);                  // x1 direct feedback. works.
      break;


    case vrange_1V:
      usart_printf("1V range \n");
      io_write(app->spi, REG_INA_VFB_ATTEN_SW, INA_VFB_ATTEN_SW1_CTL);       // no atten
      io_write(app->spi, REG_INA_VFB_SW, ~INA_VFB_SW2_CTL);                  // 10x gain
      break;

    case vrange_100mV:

      usart_printf("100mV range \n");
      io_write(app->spi, REG_INA_VFB_ATTEN_SW, INA_VFB_ATTEN_SW1_CTL);       // no atten
      io_write(app->spi, REG_INA_VFB_SW, ~INA_VFB_SW3_CTL);                  // 100x gain
      break;


  case vrange_100V:
      usart_printf("100V range \n");
      // flutters at 4th digit. with mV.  but this is on mV. range... so ok?
      // at 6th digit with V.  eg. 9V and 0.1V. - very good - will work for hv.
      io_write(app->spi, REG_INA_VFB_ATTEN_SW, INA_VFB_ATTEN_SW2_CTL | INA_VFB_ATTEN_SW3_CTL);    // atten = 0.1x
      io_write(app->spi, REG_INA_VFB_SW, ~INA_VFB_SW1_CTL);                  // x1 direct feedback. works.
      break;


    // IMPORTANT - remember have the other attenuation possibility... of just turning on/off sw3.
#if 0
  case vrange_10V_2:
      // flutters at 4th digit.
      io_write(app->spi, REG_INA_VFB_ATTEN_SW, INA_VFB_ATTEN_SW2_CTL | INA_VFB_ATTEN_SW3_CTL);    // atten = 0.1x
      io_write(app->spi, REG_INA_VFB_SW, ~INA_VFB_SW2_CTL);                                   // x10 . works.
      // vmultiplier = 1.f;
      break;
#endif

    // shouldn't have this...
    case vrange_none:
      // assert...
      break;
  };

}


static void range_voltage_iterate(app_t *app, bool dir)
{
  if(dir) {
    switch(app->vrange)
    {
      case vrange_100mV:  range_voltage_set(app, vrange_1V); break;
      case vrange_1V:     range_voltage_set(app, vrange_10V); break;
      case vrange_10V:    range_voltage_set(app, vrange_100V); break;
      case vrange_100V:   break;
      case vrange_none: break;
    };
  } else {
    switch(app->vrange)
    {
      case vrange_100mV:  break;
      case vrange_1V:     range_voltage_set(app, vrange_100mV); break;
      case vrange_10V:    range_voltage_set(app, vrange_1V); break;
      case vrange_100V:   range_voltage_set(app, vrange_10V); break;
      case vrange_none: break;
    };
  }
}




static float range_voltage_multiplier( vrange_t vrange)
{
    // ie expressed on 10V range
    switch(vrange)
    {
      case vrange_100V:   return 10.f;
      case vrange_10V:    return 1.f;
      case vrange_1V:     return 0.1f;
      case vrange_100mV:  return 0.01f;
      case vrange_none:   return -99999;
    }

  // invalid
  return -99999;
}





/*

  done - EXTREME. remove the 10k thick film. in front of the op.
  use jumper to test. or else era thin-film.
  ---
  change 200 and 20ohm to era. thin film. for lower noise.
*/

/*
  Extreme. we should turn on 5V digital. so that dg444 inputs don't just sink.
  eg. turn on 5V.
  then configure.
*/

static void range_current_set(app_t *app, irange_t irange)
{
  /*
    this is doing two things. muxing the sense input. and amplification.

    TODO IMPORTANT.
    if output is on. then we also have to change the output relay...
  */


  app->irange = irange;

  mux_io(app->spi);   // would be better to avoid calling if don't need.



  switch(app->irange)
  {

    case irange_1A:
      // ensure sure the high current relay is on. before switching
      output_set(app, app->irange, app->output);
      msleep(1);
      usart_printf("1A range \n");
      // gain 100x active low
      io_write(app->spi, REG_INA_IFB_SW,  ~INA_IFB_SW3_CTL);
      // turn on sense amplifier 1
      io_write(app->spi, REG_ISENSE_MUX,  ~ISENSE_MUX1_CTL);
      // turn on current range x
      io_write(app->spi, REG_RELAY_COM,  RELAY_COM_X);
      // turn on 2nd switch fets.
      io_write(app->spi, REG_IRANGE_X_SW, IRANGE_X_SW1_CTL);

      // TODO turn off jfets.
      break;



   // using 10ohm resistor. for 10V swing.
    case irange_100mA:
      // ensure sure the high current relay is on. before switching
      output_set(app, app->irange, app->output);
      msleep(1);
      usart_printf("100mA range \n");
      // gain 10x active low
      io_write(app->spi, REG_INA_IFB_SW,  ~INA_IFB_SW2_CTL);
      // turn on sense amplifier 2
      io_write(app->spi, REG_ISENSE_MUX,  ~ISENSE_MUX2_CTL);
      // turn on current range x
      io_write(app->spi, REG_RELAY_COM,  RELAY_COM_X);
      // turn on 2nd switch fets.
      io_write(app->spi, REG_IRANGE_X_SW, IRANGE_X_SW2_CTL);

      // TODO turn off jfets.
      // mult
      // we don't need this... can infer multiplier locally, wfrom irange if needed.
      // imultiplier = 1.f;

      break;



    // using 1k resistor. for 10V swing.
    case irange_10mA:

      usart_printf("10mA range \n");

      // gain 1x active low
      io_write(app->spi, REG_INA_IFB_SW,  ~INA_IFB_SW1_CTL);
      // turn on sense amplifier 3
      io_write(app->spi, REG_ISENSE_MUX,  ~ISENSE_MUX3_CTL);
      // turn on current range x
      io_write(app->spi, REG_RELAY_COM,  RELAY_COM_X);
      // turn on 4th switch fets.
      io_write(app->spi, REG_IRANGE_X_SW, IRANGE_X_SW4_CTL);

      // TODO turn off jfets.
      // mult
      // we don't need this... can infer multiplier locally, wfrom irange if needed.
      // imultiplier = 1.f;

      // if output on, make sure low current relay is on.. only switch after reducing current.
      msleep(1);
      output_set(app, app->irange, app->output);

      break;


    // it's actually tricky to do this....
    // need to turn things off.

    // using 10k resistor. for 10V swing
    case irange_1mA:
    // using 100k resistor with 10V swing.
    case irange_100uA:
      // gain 1x active low
      io_write(app->spi, REG_INA_IFB_SW,  ~INA_IFB_SW1_CTL);
      // turn on sense amplifier 3
      io_write(app->spi, REG_ISENSE_MUX,  ~ISENSE_MUX3_CTL);
      // turn on current range relay y
      io_write(app->spi, REG_RELAY_COM,  RELAY_COM_Y);
      // turn off all fets
      io_write(app->spi, REG_IRANGE_X_SW, 0 );

      switch( app->irange) {
        case irange_1mA:
          // turn on jfet 1
          io_write(app->spi, REG_IRANGE_YZ_SW, IRANGE_YZ_SW1_CTL);
          break;
        case irange_100uA:
          // turn on jfet 2
          io_write(app->spi, REG_IRANGE_YZ_SW, IRANGE_YZ_SW2_CTL);
          break;
        default:;

      }

      msleep(1);
      output_set(app, app->irange, app->output);
      break;


    // shouldn't have this...
    case vrange_none:
      // assert...
    break;

  }


}



static void range_current_iterate(app_t *app, bool dir)
{
  /*
    change anme iterate.
    useful test function.
    IMPORTANT. this does *NOT* take into account output relay switching.
  */
  if(dir) {
    switch(app->irange)
    {
      case irange_100uA:  range_current_set(app, irange_1mA); break;
      case irange_1mA:    range_current_set(app, irange_10mA); break;
      case irange_10mA:   range_current_set(app, irange_100mA); break;
      case irange_100mA:   range_current_set(app, irange_1A); break;
      case irange_1A: break;
      case irange_none: break;
    };
  } else {
    switch(app->irange)
    {
      case irange_1mA:    range_current_set(app, irange_100uA); break;
      case irange_10mA:   range_current_set(app, irange_1mA); break;
      case irange_100mA:  range_current_set(app, irange_10mA); break;
      case irange_1A:     range_current_set(app, irange_100mA); break;

      case irange_100uA: break;
      case irange_none: break;
    };
  }
}



static float range_current_multiplier( irange_t irange)
{
  switch(irange)
  {
    // ie. expressed on 10V range
    case irange_100uA:  return 0.00001f;
    case irange_1mA:    return 0.0001f;
    case irange_10mA:   return 0.001f;
    case irange_100mA:  return 0.01f;
    case irange_1A:     return 0.1f;
    default:
      return -99999;
  };
}



// shoudl we pass the irange?
// we might be calling this due to/ or in preparation to a range change...

static void output_set(app_t *app, irange_t irange, uint8_t val)
{
  /*
    // better name
    EXTR. we could completely bypass the AB section and just use 6090
    for low current ranges. if we wanted.
  */
  // this may be being called in response to range change.
  // we have to check the range...
  // possible we should pass the range also.

  app->output = val;

  /*
  // change to LC only
  if(app->output)
    io_set(app->spi, REG_LED, LED2);
  else
    io_clear(app->spi, REG_LED, LED2);
  */


  if(app->output) {

      usart_printf("switch output on\n");
      // switch( app->irange)
      switch( irange)
      {

        case irange_100uA:
        case irange_1mA:
        case irange_10mA:
          // turn on read relay
          // and turn off the hc relay.
          io_write(app->spi, REG_RELAY_OUT, RELAY_OUT_COM_LC);
          io_set(app->spi, REG_LED, LED2);
          break;

        case irange_100mA:
          // high current relay
          io_write(app->spi, REG_RELAY_OUT, RELAY_OUT_COM_HC);
          io_clear(app->spi, REG_LED, LED2);


      default:
        // ignore
        ;

      }
  }
  else {

    usart_printf("switch output off\n");

    io_write(app->spi, REG_RELAY_OUT, 0 ); // both relays off
    io_clear(app->spi, REG_LED, LED2);
  }
}


static void halt(app_t *app )
{
  // should do the actions here.
  app->state = HALT;


}



// I think, maybe we only need two ranges.
// or maybe even one.


// source=positive current. sink = negative current.
// can source positive voltage. but might be

// whether the value is inverse should not be a property here... i don't think.
// maybe function should always be min... due to negative fb.


/*
  source a voltage - let current be compliance.
  source a current - let voltage be compliance.

  sink a voltage - let current be compliance.
  sink a current - let voltage be compliance.

  when sourcing, (voltage and current are positive) Q1  or (voltage and current are both negative) Q3.
  when sinking,  (voltage pos and current neg)  Q2      or (voltage neg and current pos). Q4

  ------------
  think the main thing. is function( source or sink) then compliance.
  9V battery. set to 1V sink.   is that a short. or is that just letting a small amount
  Do we have to flip the min/max. around at a cross. quite possibly.
  --------------

  function - is either source or sink. but we may have to flip compliance.

  the compliance function should work in the same direction as the source function sign.
  source and compliance.
    eg.
    source positive voltage.  compliance should be positive current limit.
    source negative voltage.  (eg. reverse on a diode). compliance needs to be negative current limit. (test leakage)
    YES.
    source positive current. compiance is positive voltage limit.
    source negative current. compiance is negative voltage limit.


  sink positive voltage



*/

static void clamps_set_source_pve(uint32_t spi)
{
#if 1
  // bahaves correctly relay on or off
  // OK. this can also source or sink... depending on voltage.
  // this sources a positive voltage. eg. the min or +ve v or +ve current.
  io_write(spi, REG_CLAMP1, ~(CLAMP1_VSET_INV | CLAMP1_ISET_INV));
  io_write(spi, REG_CLAMP2, ~CLAMP2_MAX );     // min of current or voltage
                                                // MAX is min. eg. min or 3V,1mA. is 1mA. sourcing.
#endif

#if 0
  // this sources a negative voltage. eg. the min or +ve v or +ve current.
  io_write(spi, REG_CLAMP1, ~(CLAMP1_VSET | CLAMP1_ISET));
  io_write(spi, REG_CLAMP2, ~CLAMP2_MIN );     // min is max due to integration inverter
#endif

// where are our little wires.

#if 0
  // this behaves correctly relay on or off
  // this sinks a positive current.  or sources depending on voltage.
  // not sure. if set to 3V then it will try to sink 3V.
  // OR. set to 1V should be trying to sink everything. which is what it's doing. if set to 3V. it will start sourcing.
  // so i think this might be wrong.

  // not sure.

  io_write(spi, REG_CLAMP1, ~(CLAMP1_VSET_INV | CLAMP1_ISET));
  io_write(spi, REG_CLAMP2, ~CLAMP2_MIN );     // the max of current or voltage. where current is negative
#endif

  /*
    WE SHOULD BE USING WRITE HERE....

    and everything is active lo.  so must use ~ for all arguments.
  */



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


// change name to print_value...
// should construct a string i think...

static void print_value(format_t format, float val)
{
  switch(format)
  {
    case format_mV:
      usart_printf("%fmV", val * 1000.f);
      break;
    case format_V:
      usart_printf("%fV", val);
      break;


    case format_uA:
      usart_printf("%fuA", val * 1000000.f );
      break;
    case format_mA:
      usart_printf("%fmA", val * 1000.f );
      break;
    case format_A:
      usart_printf("%fA", val);
      break;
  };

}



static void update_soft_500ms(app_t *app )
{


  static uint32_t count = 0; // -1
  ++count;    // increment first. because state machine funcs return early.


  // blink mcu led
  led_toggle();

  mux_io(app->spi);

  ////////////////////////////////
  // clear led1


  io_toggle(app->spi, REG_LED, LED1);



#if 1
  if(count % 2 == 0)
  {
    // io_toggle(spi, REG_INA_VFB_SW, INA_VFB_SW1_CTL | INA_VFB_SW2_CTL | INA_VFB_SW3_CTL);
    // io_toggle(spi, REG_INA_DIFF_SW, INA_DIFF_SW1_CTL | INA_DIFF_SW2_CTL);
    // io_toggle(spi, REG_INA_ISENSE_SW,   ISENSE_SW1_CTL | ISENSE_SW2_CTL | ISENSE_SW3_CTL);
    // io_toggle(spi, REG_INA_IFB_SW1_CTL, INA_IFB_SW1_CTL | INA_IFB_SW2_CTL | INA_IFB_SW3_CTL);

    // io_toggle(spi, REG_RELAY_COM, RELAY_COM_X);

    // io_toggle(spi, REG_RELAY_OUT, RELAY_OUT_COM_HC);

    // #define REG_RELAY_OUT         31
    // #define RELAY_OUT_COM_HC    (1<<0)


    // io_toggle(spi, REG_RELAY_VSENSE, RELAY_VSENSE_CTL);



    // io_toggle(spi, REG_RELAY_OUT, RELAY_OUT_COM_HC);
    /////// CAREFUL io_toggle(spi, REG_RELAY_OUT, RELAY_OUT_COM_LC);    // dangerous if on high-current range.

    // io_write(app->spi, REG_IRANGE_YZ_SW, IRANGE_YZ_SW4_CTL);

    // io_toggle(app->spi, REG_IRANGE_YZ_SW, IRANGE_YZ_SW4_CTL); // ok.
    // io_toggle(app->spi, REG_IRANGE_YZ_SW, IRANGE_YZ_SW3_CTL);  ok.
    // io_toggle(app->spi, REG_IRANGE_YZ_SW, IRANGE_YZ_SW2_CTL);    // 1.92V.  and toggles both ina1 and ina2 . bridge on switch or fpga?
    // io_toggle(app->spi, REG_IRANGE_YZ_SW, IRANGE_YZ_SW1_CTL);     // fixed bridge.

  }
#endif

#if 0
  mux_adc03(spi);
  float lp15v = spi_mcp3208_get_data(spi, 0) * 0.92 * 10.;
  float ln15v = spi_mcp3208_get_data(spi, 1) * 0.81 * 10.;
  usart_printf("lp15v %f    ln15v %f\n", lp15v, ln15v);
#endif


 // tests

  // io_write(app->spi, REG_IRANGE_YZ_SW, count);

  // io_write(spi, REG_ISENSE_MUX,  count);    // works
  // io_write(spi, REG_INA_IFB_SW_CTL,  count);    // works
  // io_write(spi, REG_INA_VFB_ATTEN_SW, count);    // works

  // io_write(spi, REG_CLAMP1, count);  // works
  // io_write(spi, REG_CLAMP2, count);  // works
  // io_write(spi, REG_RELAY_COM, count);
  // io_write(spi, REG_IRANGE_X_SW, count);
  // io_write(spi, REG_IRANGE_SENSE, count);


  // io_write(spi, REG_GAIN_IFB, count);
  // io_write(spi, REG_GAIN_VFB, count);

  // test

  // usart_printf("count %d\n", count);
  // io_write(spi, REG_IRANGE_X_SW58, count);

  // io_toggle(spi, REG_RELAY_COM, RELAY_COM_X);
  // io_toggle(spi, REG_RELAY, RELAY_VRANGE);
  // io_toggle(spi, REG_RELAY, RELAY_OUTCOM);
  // io_toggle(spi, REG_RELAY, RELAY_SENSE);


  switch(app->state) {


    case ANALOG_UP: {

      // ... ok.
      // how to return. pass by reference...
      float ar[4];
      spi_adc_do_read(app->spi, ar, 4);


      /*
        ranging and format precision are separate and vary independently.
        so need to use common unit approach.
      */
      float x = 0.435;

      // get values in standard unit. eg. volts or amps.
      // change name range_voltage_si_coeff or similar
      float v = ar[0] * range_voltage_multiplier(app->vrange) * x;
      float i = ar[1] * range_current_multiplier(app->irange) * x;


      if(app->print_adc_values) {

        // when we set the range. we should set the default format.
        // the format prec wants to be able to user modified.
        usart_printf("adc ");

        print_value(format_V , v);
        // print_value(format_mV , v);

        usart_printf("   ");
        print_value(format_mA , i);
        // print_value(format_uA , i);
        usart_printf("\n");


  #if 0
        usart_printf("adc %dV    %dA\n",
          ar[0] ,
          ar[1]
        );
  #endif
      }

      break;
    }

    default: ;
  };
}


/*
  EXTREME.

  we need functions to handle transition.  not set the state variable. and interpret.
  but setting the state variable. at the end of code, for the new state is ok. eg. even if embedded.

  need better error handling.

*/

static void update_console_cmd(app_t *app, CBuf *console_in, CBuf* console_out, CBuf *cmd_in )
{
  /*
    TODO
    put these buffers. in the app structure.
    Actually. no. it's neater that they're not.
  */


  int32_t ch;

  while((ch = cBufPop(console_in)) >= 0) {
    // got a character

    // TODO for single character responses. then we probably don't want to
    // copy to buffer. or output.

    // copy to command buffer
    cBufPut(cmd_in, ch);

    // handling newlines...
    if(ch == '\r') {
      cBufPut(console_out, '\n');
    }
    // output char to console
    cBufPut(console_out, ch);


    // current range iteration
    if(ch == 'u')
        range_current_iterate(app, 1);
    else if(ch == 'd')
        range_current_iterate(app, 0);


    // toggle output... on/off. must only process char once. avoid relay oscillate
    else if( ch == 'o') {
      mux_io(app->spi);
      output_set(app, app->irange, !app->output);
      cBufPut(console_out, '\n');
    }

    // toggle printing of adc values.
    else if( ch == 'p') {
      app->print_adc_values = ! app->print_adc_values;
      cBufPut(console_out, '\n');
    }

    else if(ch == 'h') {
      // go to halt state
      // TODO . This is wrong. should call a function. that handles the transition.
      // not interpret the state variable.
      usart_printf("halt \n");
      halt(app);
      return;
    }

    // so we want the same thing
    // 1) to go up/and down current ranges. and voltage ranges
    //      eg. range_current( dir ); up/down
    // 2) also the formatting ranges.   eg. 5.0V or 05.0V etc.

  }

  // ok. this doesn't quite work.
  // need a variable. in_command. for a long sequence command.


  if(cBufPeekLast(cmd_in) == '\r') {

    // we got a carriage return
    static char tmp[1000];
    size_t n = cBufCopy(cmd_in, tmp, sizeof(tmp));
    tmp[n - 1] = 0;   // drop tailing line feed

    // usart_printf("got command '%s'   %d\n", tmp, n);
    usart_printf("command '%s'\n", tmp);


    if(strcmp(tmp, ":halt") == 0) {
      // go to halt state
      usart_printf("switch off\n");
      app->state = HALT;
      return;
    }


  }
}




static void quadrant_set( app_t *app, bool v, bool i)
{
    // RULES.
    // so. if voltage is positive use clamp max.  clamp min/max follows voltage.
    // negative current. can still be source or sink. depending on polarity.
    // ie. clamp direction min/max following voltage.

    mux_io(app->spi);

    uint32_t vv = v ? CLAMP1_VSET_INV : CLAMP1_VSET;
    uint32_t ii = i ? CLAMP1_ISET_INV : CLAMP1_ISET;


    io_write(app->spi, REG_CLAMP1, ~(vv | ii ));


    uint32_t minmax = v ?  CLAMP2_MAX : CLAMP2_MIN;

    io_write(app->spi, REG_CLAMP2, ~( minmax ) );     // min of current or voltage
}


// so can have another function. that tests the values.... v > 0 etc.

static void core_set( app_t *app, float v, float i)
{

    mux_dac(app->spi);

    // voltage
    spi_dac_write_register(app->spi, DAC_VOUT0_REGISTER, voltage_to_dac( fabs( v)) );
    // current
    spi_dac_write_register(app->spi, DAC_VOUT1_REGISTER, voltage_to_dac( fabs( i)) );


    quadrant_set( app, v > 0.f, i > 0.f ) ;


}



static void update(app_t *app)
{
  // called as often as possible

  /*
    querying adc03 via spi, is slow (eg. we also clock spi slower to match read speed) .
    so it should only be done in soft timer eg. 10ms is probably enough.
    preferrably should offload to fpga with set voltages, -  and fpga can raise an interupt.
  */

  // get supply voltages,
  mux_adc03(app->spi);
  // TODO put cal values in state
  float lp15v = spi_mcp3208_get_data(app->spi, 0) * 0.92 * 10.;
  float ln15v = spi_mcp3208_get_data(app->spi, 1) * 0.81 * 10.;
  UNUSED(lp15v);
  UNUSED(ln15v);
  // usart_printf("lp15v %f    ln15v %f\n", lp15v, ln15v);



  switch(app->state) {

    case FIRST:  {
      // if any of these fail, this should progress to error

      usart_printf("-----------\n");
      usart_printf("digital init start\n" );

      mux_io(app->spi);

      ////////////
      // soft reset is much better here.
      // avoid defining initial condition. in more than one place
      // so define in fpga.
      io_clear(app->spi, CORE_SOFT_RST, 0);    // any value addressing this register.. to clear

      // no. needs dg444/mux stuff. pulled high. for off.
      // BUT I THINK we should probably hold RAILS_OE high / deasserted.


      // test the flash
      // TODO. check responses.
      mux_w25(app->spi);
      spi_w25_get_data(app->spi);

#if 1
      // dac init
      int ret = dac_init(app->spi, REG_DAC); // bad name?
      if(ret != 0) {
        app->state = ERROR;
        return;
      }
#endif

      // TODO remove.... fix regualte on vfb.
      usart_printf("-------------\n" );

      usart_printf("set voltage range\n" );
      range_voltage_set(app, vrange_10V);

      usart_printf("set current range\n" );
      range_current_set(app, irange_10mA);

      // progress to digital up?
      usart_printf("digital init done/ok\n" );
      app->state = DIGITAL_UP;
      break;
    }


    case DIGITAL_UP:


      if(lp15v > 15.0 && ln15v > 15.0 )
      {
        usart_printf("-----------\n");

        usart_printf("doing analog init -  supplies ok \n");
        usart_printf("lp15v %f    ln15v %f\n", lp15v, ln15v);


        usart_printf("turn on lp5v\n" );
        mux_io(app->spi);
        // assert rails oe
        io_clear(app->spi, REG_RAILS_OE, RAILS_OE);

        // turn on 5V digital rails
        io_set(app->spi, REG_RAILS, RAILS_LP5V );
        msleep(50);

        // turn on +-15V rails
        usart_printf("turn on analog rails - lp15v\n" );
        io_set(app->spi, REG_RAILS, RAILS_LP15V );
        msleep(50);

        // LP30 - needed to power the vfb topside op amp. ltc6090/ bootstrapped
        // io_set(spi, REG_RAILS, RAILS_LP30V );
        // msleep(50);
   /*
        io_set(spi, REG_RAILS, RAILS_LP60V );
   */


#if 0
        // TODO EXTREME . set the gain switches before turning on rails.
        // IMPORTANT - should probably do this. before switching on the supplies.
        // so that vrange ops are not high-Z
        usart_printf("turn on voltage range\n" );
        range_voltage_set(spi);

        usart_printf("turn on current range\n" );
        range_current_set(spi);

#endif


        // turn on refs for dac
        //mux_dac(spi);
        usart_printf("turn on ref a for dac\n" );
        mux_io(app->spi);
        io_write(app->spi, REG_DAC_REF_MUX, ~(DAC_REF_MUX_A | DAC_REF_MUX_B)); // active lo

        // dac naked register references should be wrapped by functions
        // unipolar.
        // voltage

        /*
          https://www.youtube.com/watch?v=qFVhe_uzxnE
          source current. 100mA.   with compliance +21V.   DUT resistive load.
          source current -100mA.   with compliance +21V.   with power supply.
        */

        // core_set( app, -5.f , -5.f );    // -5V compliance, -1mA  sink.
        core_set( app, 5.f , 3.f );    // 5V source, 5mA compliance,


        /////////////
        // working as bipolar.
        spi_dac_write_register(app->spi, DAC_VOUT2_REGISTER, voltage_to_dac( -2.f ) );  // outputs -4V to tp15.  two's complement works. TODO but need to change gain flag?
        spi_dac_write_register(app->spi, DAC_VOUT3_REGISTER, voltage_to_dac( 0.f ) );  // outputs 4V to tp11.


        // mux_io(app->spi);
        // clamps_set_source_pve(app->spi);


        range_voltage_set(app, vrange_10V);

        range_current_set(app, irange_10mA);
        // range_current_set(app, irange_1mA);
        // range_current_set(app, irange_100uA);


        // change namem output relay?
        output_set(app, app->irange, true );   // turn on



        /////////////////
        // adc init has to be done after rails are up...
        // but doesn't need xtal, to respond to spi.
        // adc init
        int ret = adc_init(app->spi, REG_ADC);
        if(ret != 0) {
          app->state = ERROR;
          return;
        }


        app->state = ANALOG_UP;
      }
      break ;


    case ANALOG_UP:

#if 1
      if((lp15v < 14.7 || ln15v < 14.7)  ) {

        mux_io(app->spi);
        usart_printf("supplies bad - turn off rails\n");
        usart_printf("lp15v %f    ln15v %f\n", lp15v, ln15v);

        // turn off power
        io_clear(app->spi, REG_RAILS, RAILS_LP15V | RAILS_LP30V | RAILS_LP60V);

        // TODO use the function that turns off both relays.
        // eg. write not read.
        // turn off output relay
        // io_clear(spi, REG_RELAY, RELAY_OUTCOM);

        output_set(app, app->irange, false );

        // go to state error
        app->state = ERROR;
      }
#endif

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
        io_clear(app->spi, REG_RELAY, RELAY_OUTCOM);

        // turn off 5V digital and all analog power
        io_clear(app->spi, REG_RAILS, RAILS_LP5V | RAILS_LP15V | RAILS_LP30V | RAILS_LP60V);
      }
      // stay in error state.
    }
    break;


    case HALT: {

      static int first = 0;
      if(!first) {
        first = 1;
        usart_printf("entered halt state\n" );

        mux_io(app->spi);
        // turn off output relay
        io_clear(app->spi, REG_RELAY, RELAY_OUTCOM);

        // turn off 5V digital and all analog power
        io_clear(app->spi, REG_RAILS, RAILS_LP5V | RAILS_LP15V | RAILS_LP30V | RAILS_LP60V);
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


/*
  TODO.
  Move. these into the app structure.
  and mvoe the app structure off the stack.
*/

// should pass the console to routines that need it...
static char buf1[1000];
static char buf2[1000];

static CBuf console_in;
static CBuf console_out;



static char buf3[1000];
static CBuf cmd_in;


static void loop(app_t *app)
{

  // move this into the app var.
  static uint32_t soft_500ms = 0;



  while(true) {

    // EXTREME - could actually call update at any time, in a yield()...
    // so long as we wrap calls with a mechanism to avoid stack reentrancy
    // led_update(); in systick.
    // but better just to flush() cocnsole queues.conin/out


    // usart_input_update();
    usart_output_update();

    // update_console_cmd(spi, &console_in);
    update_console_cmd(app, &console_in, &console_out, &cmd_in);

    update(app);

    // 500ms soft timer
    if( system_millis > soft_500ms) {
      soft_500ms = system_millis + 500;
      update_soft_500ms(app);
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

  ////////
  // put this in spi1.h.  i think....
  // uint32_t spi = SPI_ICE40;


  app_t app;
  memset(&app, 0, sizeof(app_t));

  app.spi = SPI_ICE40;
  app.state = FIRST;
  app.print_adc_values = true;
  app.output = false;

  loop(&app);

	for (;;);
	return 0;
}


/*
  OLD
  io_write(spi, REG_INA_VFB_SW, INA_VFB_SW1_CTL);
  // io_write(spi, REG_INA_DIFF_SW, INA_DIFF_SW1_CTL); // ina154
  // io_write(spi, REG_INA_DIFF_SW, INA_DIFF_SW2_CTL); // ina143
*/



#if 0

static void range_current_set_1A(uint32_t spi)
{
  // 2V on 1A is 200mA, 5V is 0.5A
  // sense gain = 0.1x  ie. 0.1ohm sense resistor
  // ina gain x10.
  // extra amp gain = x10.

  // write() writes all the bits.

  // turn on current relay range X.
  io_write(spi, REG_RELAY_COM, RELAY_COM_X);


  // turn on 1st b2b fets.
  io_write(spi, REG_IRANGE_X_SW, IRANGE_X_SW1 | IRANGE_X_SW2);

  // turn on current sense ina 1
  io_write(spi, REG_IRANGE_SENSE, ~IRANGE_SENSE1);

  // active lo. turn on ifb gain op1, x10
  io_write(spi, REG_GAIN_IFB, ~GAIN_IFB_OP1);


  imultiplier = 0.1f;
}

static void range_current_set_10A(uint32_t spi)
{
  // 0.1ohm
  // 300mV=3A across 0.1R sense.   could use 3.33 (10/3.3) gain after ina to get to 0-10V..
  // 1 / ( 1 +  2 )  = 0.3333333333333333
  // = divider with r1=1 and r2=2. eg. a 2 to 1.
  // eg. make op2 be

  // 10A is the same as 1A, except no 10x gain
  range_current_set_1A(spi);

  // active lo. turn off both ifb gain stages...
  // using 10x gain from ina, on 0.1R only.
  io_write(spi, REG_GAIN_IFB, GAIN_IFB_OP1 | GAIN_IFB_OP2 );

  imultiplier = 1.0f;
}



static void range_current_set_100mA(uint32_t spi)
{
  // 10ohm.
  // 2V on 100mA range should be 20mA.
  // 0.2V across 10ohm. g=10x, 0.2 / 10 = 0.02A = 20mA.
  // adc imultiplier should be 0.1.

  // turn on current relay range X.
  io_write(spi, REG_RELAY_COM, RELAY_COM_X);


  // turn on 2nd b2b fets.
  io_write(spi, REG_IRANGE_X_SW, IRANGE_X_SW3 | IRANGE_X_SW4);


  // active lo, current sense 2
  io_write(spi, REG_IRANGE_SENSE, ~IRANGE_SENSE2);

  // active lo. turn off both current gain ops
  io_write(spi, REG_GAIN_IFB, GAIN_IFB_OP1 | GAIN_IFB_OP2 );

  imultiplier = 0.01f; // sense gain = x10 (10ohm) and x10 gain.
}




static void range_current_set_10mA(uint32_t spi)
{
  // UNUSED(spi);
  /*
      10V = 0.01A * 1k.
      = 100mW.      maybe ok.  with high-watt 1
     1x gain.
  */


  // turn on current relay range X.
  io_write(spi, REG_RELAY_COM, RELAY_COM_X);

  // turn off other fets
  io_write(spi, REG_IRANGE_X_SW, 0);


  // active lo, current sense 3
  io_write(spi, REG_IRANGE_SENSE, ~IRANGE_SENSE3);

  // active lo, turn off both current gain ops
  io_write(spi, REG_GAIN_IFB, GAIN_IFB_OP1 | GAIN_IFB_OP2 );

  imultiplier = 0.001f;
}


#endif


////////////////////////////


#if 0
static void range_voltage_set_100V(uint32_t spi)
{
  // now using ina 143. with 1:10 divide by default

  io_clear(spi, REG_RELAY, RELAY_VRANGE ); // no longer used. must be off.


  // active lo, turn both vfb gain stages off
  io_write(spi, REG_GAIN_VFB, GAIN_VFB_OP1 | GAIN_VFB_OP2 );

  vmultiplier = 10.f;
}


static void range_voltage_set_10V(uint32_t spi)
{
  // now using ina 143. with 1:10 divide by default

  io_clear(spi, REG_RELAY, RELAY_VRANGE ); // no longer used. must be off.

  // active lo. turn on OP1
  io_write(spi, REG_GAIN_VFB, ~GAIN_VFB_OP1 );

  vmultiplier = 1.f;
}


static void range_voltage_set_1V(uint32_t spi)
{
  // now using ina 143. with 1:10 divide by default

  io_clear(spi, REG_RELAY, RELAY_VRANGE ); // no longer used. must be off.

  // active lo.  turn on both OP1 and OP2
  io_write(spi, REG_GAIN_VFB, ~(GAIN_VFB_OP1 | GAIN_VFB_OP2) );

  vmultiplier = 0.1f;
}
#endif

#if 0

  // io_write(spi, REG_INA_VFB_ATTEN_SW, INA_VFB_ATTEN_SW1_CTL);                         // atten = non = 1x
  io_write(spi, REG_INA_VFB_ATTEN_SW, INA_VFB_ATTEN_SW2_CTL | INA_VFB_ATTEN_SW3_CTL);    // atten = 0.1x


  // fix in fpga code. init should be 0b4
  // io_write(spi, REG_INA_VFB_SW, ~INA_VFB_SW1_CTL);    // x1 direct feedback. works.
  io_write(spi, REG_INA_VFB_SW, ~INA_VFB_SW2_CTL);    // x10 . works.
  // io_write(spi, REG_INA_VFB_SW, ~INA_VFB_SW3_CTL);       // x100  works. 0.1V diff gives  8.75V out.

  // 9.1 - 9.0 -> *.1*100 = 0.818.
  // 1.1 - 1.0 -> *.1 x100 = 0.859
  // 0.1 - 1.0             = 0.845

#endif
  // try it without the atten...
  // io_write(spi, REG_INA_IFB_SW1_CTL,  count);    // works

#if 0
  // turn on sense dual op, for high-current range b2b fets
  io_write(spi, REG_INA_ISENSE_SW,  ~ISENSE_SW1_CTL);

  // turn on no resistor divider fb for gain = 1x.
  // io_write(spi, REG_INA_IFB_SW1_CTL, ~INA_IFB_SW1_CTL); // 1x gain.
  // io_write(spi, REG_INA_IFB_SW1_CTL, ~INA_IFB_SW2_CTL);    // 10x gain.
  io_write(spi, REG_INA_IFB_SW1_CTL, ~INA_IFB_SW3_CTL);    // 100x gain.

#endif
  // io_write(spi, REG_ISENSE_MUX,  ~ ISENSE_MUX1_CTL);    // select dedicated 0.1 ohm sense resistor and op. active lo
  // io_write(spi, REG_ISENSE_MUX,  ~ ISENSE_MUX2_CTL);    // select dedicated 10 ohm sense resistor and op. active lo
  // io_write(spi, REG_ISENSE_MUX,  ~ ISENSE_MUX3_CTL);        // select any other range resistor

  // 100x. is stable flickers at 6th digit. nice!!!...



#if 0
  switch(irange)
  {

    case irange_1x:
      io_write(spi, REG_INA_IFB_SW,  ~INA_IFB_SW1_CTL);   //  active low
      imultiplier = 1.f;
      break;

    case irange_10x:
      io_write(spi, REG_INA_IFB_SW,  ~INA_IFB_SW2_CTL);   //  active low
      imultiplier = 10.f;
      break;


    case irange_100x:
      io_write(spi, REG_INA_IFB_SW,  ~INA_IFB_SW3_CTL);   //  active low
      imultiplier = 100.f;
      break;

    // shouldn't have this...
    case vrange_none:
      // assert...
    break;

  }
#endif


#if 0
      // change name GAIN_IFB_OP1 ... GAIN_VFB_OP2   etcc
      // eg. clear ifb regs.
      io_write_mask(spi, REG_GAIN_FB, GAIN_IFB_OP1 | GAIN_IFB_OP2, GAIN_IFB_OP1 | GAIN_IFB_OP2);
      io_write_mask(spi, REG_GAIN_FB, GAIN_VFB_OP1 | GAIN_VFB_OP2,  GAIN_VFB_OP1 | GAIN_VFB_OP2);
      io_write_mask(spi, REG_GAIN_FB, GAIN_IFB_OP1 | GAIN_IFB_OP2, 0 );
      state = HALT;
      return;
#endif

      // range_voltage_set(spi, vrange_10V_2);
      // range_voltage_set(spi, vrange_100V);
      // range_voltage_set(spi, vrange_1V);
      // range_voltage_set(spi, vrange_100mV);


#if 0
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
        spi_dac_write_register(spi, REG_DAC_VSET, voltage_to_dac( 1.f ) ); // 10V

        mux_io(spi);
        // range_voltage_set_100V(spi);       // ie. 1.2  = 12V, 1.5=15V etc
        range_voltage_set_10V(spi);           // ie 1.2 = 1.2V
        // range_voltage_set_1V(spi);         // ie 1.2 = 0.12V

        // current
        mux_dac(spi);
        spi_dac_write_register(spi, REG_DAC_ISET, voltage_to_dac( 1.f ) );  // 5.f

        mux_io(spi);
        // range_current_set_10A(spi);           // ie 1=1A, 0.5=0.5A, 0.1=0.1V
        // range_current_set_1A(spi);         // ie. 1=0.1A,10=1A
        // range_current_set_100mA(spi);      // 1=10mA, 10=100mA.
        range_current_set_10mA(spi);          // 1=1mA, 10=100mA.
        // range_current_set_none(spi);       // won't work. there's no circuit.

        // turn on output relay
        io_set(spi, REG_RELAY, RELAY_OUTCOM);


        /////////////////
        // adc init has to be done after rails are up...
        // adc init
        int ret = adc_init(spi, REG_ADC);
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
        // io_set(spi, REG_RAILS, RAILS_LP30V );
        io_set(spi, REG_RAILS, RAILS_LP60V );  // actually 15V
        msleep(50);
#endif

        // analog and power... change name?

#endif





#if 0

// whether the value is inverse should not be a property here... i don't think.

static void clamps_set_source_pve(uint32_t spi)
{
/*
  should also have the option to regulate on vfb directly here.
  and errset?
*/
  // TODO needs to take an enum. arguemnt.


  // change name first_quadrant
  // sourcing, charging adc val 1.616501V
  // source +ve current/voltage.
  /*
    TODO change name CLAMP_MUX_MAX, or CLAMP_MAX_MUX
    actually not sure.
  */
#if 0
  // source a positive voltage. on +2mA. off +5V.
  io_write(spi, REG_CLAMP1, CLAMP1_VSET_INV | CLAMP1_ISET_INV);
  io_write(spi, REG_CLAMP2, CLAMP2_MAX);    // TODO this is terrible register naming.
#endif

#if 0
  // this works.
  // source a negative voltage. on its -2mA. off its -5V.
  io_write(spi, REG_CLAMP1, CLAMP1_VSET | CLAMP1_ISET);
  io_write(spi, REG_CLAMP2, CLAMP2_MIN  );    // TODO this is terrible register naming.
#endif

#if 1

  // When using min. it will source or sink depending on voltage.  'not less than'

  // source/ or sink positive.
  // current has to be negative to sink. otherwise it's charging.
  // now sink.
  // sinking current. with battery. on.  1.58V. -2mA.   but off reads -1.58V...

  // lower voltage == sink more current (eg. more of a short). it's confusing.
  // this is correct. limit at the min... means whatever will produce the lowest current. eg. if -2mA is limit use that.
  // io_write(spi, REG_CLAMP1, ~(CLAMP1_VSET_INV | CLAMP1_ISET));
  // io_write(spi, REG_CLAMP2, ~CLAMP2_MIN );     // min is max due to integration inverter


  io_write(spi, REG_CLAMP1, ~(CLAMP1_VSET_INV | CLAMP1_ISET));
  io_write(spi, REG_CLAMP2, ~CLAMP2_MIN );     // min is max due to integration inverter


#endif

  /*
    WE SHOULD BE USING WRITE HERE....

    and everything is active lo.  so must use ~ for all arguments.
  */

  // VERY IMPORTANT. rather than just show the adc values.
  // need to show the set values as well. in the log.

  /////////////////
  // EXTREME. when relay is off. the bad adc code. is because output is at -14V because it's trying to sink. but cannot because relay off. this is correct.
  // eg. we putting -3.2V measured into adc. BAD. rail is -2.5V... eg. ESD diodes are sinking everything. op saturated?
  // eg. 3 / (10 + 3) * 15V = 3.46
  // BUT. can fix. with attenuation? no.
  // if used +-30V output. then it would try to output -30V. and that would be even more...
  // this might be a problem.
  // OR. when sinking. we only set dac values. when relay is on.
  // also. the adc out-of-bounds flag is sticking. because we are not clearing it.
  // voltage is (hopefully) limited by the driving op.
    //
  // the alternative. is that the subtraction should work the other way. so current can be high. but it's limited if voltage goes down to 1V.
  // this could be a mistake in our mux. logic.
  // in which case we do need to invert.
  // OR. we can represent it. but need to flip signs when outputting values.
  //
  // no. think it's ok. eg. 3V.2mA will regulate on 2mA. but 1V,3A  and it won't short any more than 1V. think that is correct.
  //////////////

  /////////////////

  // sink negative. shows negative voltage. and positive current. but limits correctly.
#if 0
  io_write(spi, REG_CLAMP1, CLAMP1_VSET | CLAMP1_ISET_INV);
  io_write(spi, REG_CLAMP2, CLAMP2_MAX );  // retest.

#endif



}

#endif

#if 0
        // source pos voltage, with pos compliance current.  (current can be Q1 positive or Q2 negative to dut battery) depending on DUT and DUT polarity.
        if(false) {
          // ok. this is correct. source 2mA. with compliance of 3V.
          // alternatively can source voltage 1V with compliance of 10mA.
          // and outputs the source voltage 3V compliance when relay is off.
          // limits DUT battery in both polarities.ways
          mux_dac(app->spi);
          // voltage
          spi_dac_write_register(app->spi, DAC_VOUT0_REGISTER, voltage_to_dac( 3.f  ) ); // 3V
          // current
          spi_dac_write_register(app->spi, DAC_VOUT1_REGISTER, voltage_to_dac( 2.0f ) );  // 2mA.
          mux_io(app->spi);
          io_write(app->spi, REG_CLAMP1, ~(CLAMP1_VSET_INV | CLAMP1_ISET_INV));   // positive voltage and current.
          io_write(app->spi, REG_CLAMP2, ~CLAMP2_MAX );     // min of current or voltage
        }

        // Q3  source neg voltage, or neg current.   correct if DUT = resistive load.
        // DUT=battery.  Q4.  this is correct .... positive voltage, and negative current.
        if(false ) {
          // correct sources negative voltage. and negative current compliance or vice versa.
          // relay off shows -3V. correct.
          mux_dac(app->spi);
          // voltage
          spi_dac_write_register(app->spi, DAC_VOUT0_REGISTER, voltage_to_dac( 1.50 /*3.0f*/  ) );     // this has no effect. either below or above dut V. if DUT is battery. ...
          // current
          spi_dac_write_register(app->spi, DAC_VOUT1_REGISTER, voltage_to_dac( 5.0f ) );      // -1mA. resistor or battery


          mux_io(app->spi);
          io_write(app->spi, REG_CLAMP1, ~(CLAMP1_VSET | CLAMP1_ISET));   // positive voltage and current.
          io_write(app->spi, REG_CLAMP2, ~CLAMP2_MIN );     // min of current or voltage
        }
        // source pos voltage. and sink current. for DUT.   Q4.
        // will source neg voltage. sink current. for resistor.   Q3.
        if(true) {

          // OK. this is better for DUT sinking. compliance voltage is positive.  current is negative.
          // -1mA. regardless of DUT polarity.
          // the only way to limit voltage exercusion. is to sink more current. this is correct. why set -100mA. and +21V  compliance
          mux_dac(app->spi);
          // voltage
          spi_dac_write_register(app->spi, DAC_VOUT0_REGISTER, voltage_to_dac( 3.50 /*3.0f*/  ) );     // 1.5 is respected. it will limit voltage. by sinking more current.
          // current
          spi_dac_write_register(app->spi, DAC_VOUT1_REGISTER, voltage_to_dac( 1.0f ) );      // -1mA. resistor or battery

          mux_io(app->spi);
          io_write(app->spi, REG_CLAMP1, ~(CLAMP1_VSET_INV | CLAMP1_ISET));   // positive voltage and current.
          io_write(app->spi, REG_CLAMP2, ~CLAMP2_MAX );     // min of current or voltage

           }
#endif


#if 0
      // RULES.
      // so. if voltage is positive use clamp max.  clamp min/max follows voltage.
      // negative current. can still be source or sink. depending on polarity.

        mux_dac(app->spi);
        // voltage
        spi_dac_write_register(app->spi, DAC_VOUT0_REGISTER, voltage_to_dac( 3.f  ) ); // 3V
        // current
        spi_dac_write_register(app->spi, DAC_VOUT1_REGISTER, voltage_to_dac( 5.0f ) );  // 2mA.
        quadrant_set( app, false, false) ;


       // usart_printf(" -0.123 %f    %f \n",   -0.123,  fabs(-0.123) );
#endif

