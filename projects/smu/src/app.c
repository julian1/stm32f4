
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

  app->state = STATE_HALT;
}




void app_initialize( app_t * app )
{
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
    app_goto_fail_state(app);
    return;
  }


  // now we would turn on 5V and +-15V.

  printf("turn on lp5v and lp15v rails\n" );

  ice40_reg_clear(app->spi, REG_RAILS_OE, RAILS_OE);

  ice40_reg_set(app->spi, REG_RAILS, RAILS_LP5V | RAILS_LP15V);

 
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
