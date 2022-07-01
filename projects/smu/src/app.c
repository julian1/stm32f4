
#include <stdio.h>    // printf

#include "app.h"


// #include "spi1.h"
#include "mux.h"
#include "reg.h"
#include "spi-ice40.h"

#include "util.h"   // msleep


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
