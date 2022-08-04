
#include <stdio.h>    // printf
#include "assert.h"

#include "app.h"


// #include "spi1.h"
#include "mux.h"
#include "reg.h"
#include "spi-ice40.h"

#include "util.h"   // msleep

#include "dac8734.h"
#include "format.h"   // format_bits()


void app_goto_fail_state( app_t * app )
{
  printf("app goto fail state\n");
  mux_ice40(app->spi);
  ice40_reg_set(app->spi, CORE_SOFT_RST, 0 );

  /*
    We need to change the state. to app.state_power. or similar.
  */
  app->state = STATE_HALT;
}






// static void app_mux_quadrant_set( app_t *app, bool v, bool i)

// spi_mux

static void spi_mux_quadrant_set( uint32_t spi, bool v, bool i)
{
  // RULES.
  // so. if voltage is positive use clamp max.  clamp min/max follows voltage.
  // negative current. can still be source or sink. depending on polarity.
  // ie. clamp direction min/max following voltage.

  mux_ice40(spi);

  uint32_t vv = v ? CLAMP1_VSET_INV : CLAMP1_VSET;
  uint32_t ii = i ? CLAMP1_ISET_INV : CLAMP1_ISET;


  ice40_reg_write(spi, REG_CLAMP1, ~(vv | ii ));

  // rembmer inverse
  uint32_t minmax = v ? CLAMP2_MAX : CLAMP2_MIN;

  ice40_reg_write(spi, REG_CLAMP2, ~( minmax ) );     // min of current or voltage
}


// so can have another function. that tests the values.... v > 0 etc.
// need to hide

/*
  clamp direction follows voltage.

              clamp min  <->  clamp max

                           |+i
             (2)           |           (1)
             sink          |           source
                           |
                           |
                           |
         -ve --------------+-------------- +ve
                           |
                           |
                           |
             source        |           sink
             (3)           |           (4)
                           |-i

*/




void app_initialize( app_t * app )
{
  /*
    Having a very simple example of code that - brings everything  up . is really good and should keep.
    even if add extra complicating code

  */

  /*
    - there is no reason to have separate states for the different condition of the rails.
    - using the soft reset of fpga - to go to initial condition is good - avoid representing this state in more than one place.
    - not sure if this function needs more.
    ----


  */

  // assert(app->state == APP_STATE_FIRST);
  assert(app->state == STATE_FIRST);
  UNUSED(app);

  // we start with 3.3V up.

  // do ice40 soft reset, to init ice40 state
  printf("ice40 soft core reset\n");
  mux_ice40(app->spi);
  ice40_reg_set(app->spi, CORE_SOFT_RST, 0 );


  msleep(50);

  // test have comms with the ice40...

  // dac init
  int ret = dac_init(app->spi, REG_DAC); // bad name?
  if(ret != 0) {
    printf("dac init failed");
    app_goto_fail_state(app);
    return;
  }


  msleep(50);

  // should check that the 15V rails are up.

  mux_ice40(app->spi);
  uint8_t val = ice40_reg_read( app->spi, REG_MON_RAILS );

  char buf[100];
  printf("reg_mon_rails read bits %s\n", format_bits(buf, 4, val) );

  if(val != 0b0011 ) {
    printf("+-15V rails not up\n");
    app_goto_fail_state(app);  // set_fail_state() ... do a full reset...
    return;
  }


  // now we would turn on 5V and +-15V.

  printf("turn on lp5v and lp15v rails\n" );
  ice40_reg_clear(app->spi, REG_RAILS_OE, RAILS_OE);
  ice40_reg_set(app->spi, REG_RAILS, RAILS_LP5V | RAILS_LP15V);

  msleep(50);

  // turn on refs for dac
  //mux_dac(spi);
  printf("turn on ref a for dac\n" );
  mux_ice40(app->spi);
  ice40_reg_write(app->spi, REG_DAC_REF_MUX, ~(DAC_REF_MUX_A | DAC_REF_MUX_B)); // active lo

  msleep(50);

  // output 3V. vset.

  ///////////////
  //  dac set voltages.

  printf("output votlage on dac0\n" );
  mux_dac(app->spi);
  spi_dac_write_register(app->spi, DAC_DAC0_REGISTER, voltage_to_dac( 0.5f ) ); // 5V with atten
                                                                                // outputs 3.5V???
  // remember these are not negative
  spi_dac_write_register(app->spi, DAC_DAC1_REGISTER, voltage_to_dac( 4.f ) );

  msleep(50);

  //////////////
  // function
  spi_mux_quadrant_set( app->spi, true, true );     // source positive voltage. max
  // spi_mux_quadrant_set( app->spi, false, false );    // source negative voltage,  min

  /////////////
  // voltage feedback
  // select voltage sense internal

  // TODO.
  // move relay sense ext/in to own register. in order to do write.
  // not sure. all ext/in/out/guard relays - are user controllable. and everything should still work.
  // but exclusivity for int/ext would be good.



  ice40_reg_set(app->spi, REG_RELAY_OUT, REG_RELAY_SENSE_INT_CTL);    // FIXME. write not set // perhaps push to own register to make exclusive.
  ice40_reg_clear(app->spi, REG_RELAY_OUT, REG_RELAY_SENSE_EXT_CTL);


  // ice40_reg_write(app->spi, REG_INA_VFB_ATTEN_SW, INA_VFB_ATTEN_SW1_CTL);       // vfb divider. set no atten
  ice40_reg_write(app->spi, REG_INA_VFB_ATTEN_SW, INA_VFB_ATTEN_SW2_CTL);       // vfb divider. set with atten

  msleep(50);

  ////////////
  // current feedback
  // these should all be write() not set() to be exclusive.

  // use com x relay
  ice40_reg_write(app->spi, REG_RELAY_COM, RELAY_COM_X_CTL);

  // set fet switches for 1k.
  ice40_reg_write(app->spi, REG_IRANGE_X_SW, IRANGE_X_SW4_CTL);

  // set current op amp.
  ice40_reg_write(app->spi, REG_ISENSE_MUX, ~ISENSE_MUX3_CTL); // active lo. set is high.

  msleep(50);


  // turn on output power
  ice40_reg_set(app->spi, REG_RAILS, RAILS_LP24V | RAILS_LP50V );  // set not write

  // OK. it works... to source 3V.




}













/*
  Why isn't this just called update()
  so that it progresses as required.
  ---
  rather than have

  -----
  - I don't think it's correct. to have a general state change. with implication
  that can change to any state.

  - 1. progress through startup initialization.
  - 2. instead should just monitor and handle shutdown. and on bad conditions.

  - analog up. is going to do a lot of things.

*/

#if 0
void state_change(app_t *app, enum state_t state )
{

  // first thing to do - should be test the rails voltages.
  // and if fault. then change the state.
  // Eg. do in all states. so factor code once.



  uint8_t val = ice40_reg_read( app->spi, REG_MON_RAILS );
  UNUSED(val);

  // printf("reg_mon_rails read bits %s\n", format_bits(buf, 4, val) );



  switch(state) {

    case STATE_FIRST: {
      printf("-------------\n" );
      printf("state first\n" );

      app->state = STATE_FIRST;
      break;
    }



    case STATE_ANALOG_UP: {


      printf("turn on lp5v\n" );
      mux_ice40(app->spi);
      // assert rails oe
      ice40_reg_clear(app->spi, REG_RAILS_OE, RAILS_OE);

      // turn on 5V digital rails
      ice40_reg_set(app->spi, REG_RAILS, RAILS_LP5V );
      msleep(50);

#if 1
      // turn on +-15V rails
      printf("turn on analog rails - lp15v\n" );
      ice40_reg_set(app->spi, REG_RAILS, RAILS_LP15V );
      msleep(50);
#endif

#if 0
      // turn on +-30V rails. think this is ok here...
      printf("turn on power rails \n" );
      ice40_reg_set(app->spi, REG_RAILS, RAILS_LP30V );
      msleep(50);
#endif

      app->state = STATE_ANALOG_UP;

  }




    default:
      ;

  } // switch
}
#endif
