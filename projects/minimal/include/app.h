
#pragma once

#include <lib2/cbuffer.h>
#include <lib2/cstring.h>



/*
  there are two main cases to yield.
    - for time,delay.
    - until we have enough measurements in buffer.

    - both cases - should be handled with functions with 'app_yield_wait_dealay( app, mdelay )' ,  'app_yield_wait_measurements( app, 10 ) like naming .
    - don't even really need the indirection of function pointer and context.

  ---
  - yield function - can be used to process/service incomming raw data - eg. convert adc counts to a value.
      while control is blocking waiting for measurement data.
      without needing two stacks.  eg.

      while(!app->data_ready)
        app->yield( app->yield_ctx);


      // wait for obs etc.
      while( queue_count(app->measurements) < 10)
        app->yield( app->yield_ctx);

      // sleep
      system_millis = 0;
      while( system_millis < 500 )
        app->yield( app->yield_ctx);

      etc


      the limitation of a single stack yield(), is no nested co-recursion.
      but thats ok, we only engage in one user task/sequence/ at a time

      actually we can block on more than two events.  we just need to write the poll loop for the tests, and call app->yield.
      ----
      uses -
        1. for blocking when need to wait on some condition eg. measurement data
        2. long running functions, to keep processing.
*/


typedef struct _mode_t _mode_t;

typedef struct data_t data_t;


typedef struct led_t led_t;


typedef struct spi_ice40_t spi_ice40_t;
typedef struct spi_4094_t spi_4094_t;
typedef struct spi_t spi_t;
// typedef struct spi_ad5446_t spi_ad5446_t;



typedef struct interrupt_t interrupt_t;




/*
  TODO nov 2024.

  rather than using pointers. should instantiate the memory properly here
  No. hide state here.
  But instantiate all state in main.c or appc

*/


typedef struct app_t
{
  uint32_t magic;


  // remove. should be able to query the led state  to invert it...
  // no. it's ok. led follows the led_state, more than one thing follows.
  bool led_state ;     // for mcu. maybe change name to distinguish

  // port/pinno
  // uint16_t led_status;
  led_t *led_status;

  uint32_t soft_500ms;

  // updated on interupt. should probably be declared volatile.
  // but functions that use can also declare volatile

  /*
      should be downcount decrement.
      one for 500ms timer. and another for msleep/yield_with_delay.
      needs to be signed.
      eg.
      if(soft_500ms <= 0)
        soft_500ms += 500;
  */
  volatile uint32_t system_millis;

  cbuf_t  console_in;
  cbuf_t  console_out;

  cstring_t     command;


  /* for virtual trigger device. sample mosi, on single clk. can just bit-bang it, from mcu side.
    two d-flip flops. with cs rising edge operating to latch input.
    spi_trigger_t *spi_trigger;

  */

  bool          cdone_u102; // ice40 config done


  spi_ice40_t   *spi_u102;   // need a better name.  ice40 analog board.

  spi_4094_t    *spi_4094;   // eg. separate system.

  spi_t         *spi_ad5446;   // separate system

  spi_ice40_t   *spi_u202;   // separate system.

  interrupt_t   *interrupt_u202;


  // why use pointers here?
  // can we avoid
  const _mode_t *mode_initial;

  _mode_t       *mode_current;



  data_t        *data;

  bool verbose;

} app_t;



#define APP_MAGIC   456



void app_init_console_buffers(app_t *app);
// void app_loop(app_t *app);
void app_systick_interupt(app_t *app);


void app_update_main(app_t *app);


void app_configure( app_t *app );

void app_update_simple_led_blink(app_t *app);
void app_update_simple_with_data(app_t *app);



/*

void app_yield_with_delay(app_t *app, uint32_t delay );
void app_yield(app_t *app);

*/



bool app_repl_statement(app_t *app,  const char *cmd);
void app_repl_statements(app_t *app,  const char *s);

/*
  IMPORTANT - none of these take arguments anymore,
  the precondition state, should be setup in other commands.
  so we could remove the cmd argument.  and do the test in the repl command.
  but leave for moment, in case want to be able to pass flags.
*/


// from /test
bool app_test01( app_t *app , const char *cmd);
bool app_test02( app_t *app , const char *cmd);
bool app_test03( app_t *app , const char *cmd);

// dcvsource
bool app_test10( app_t *app , const char *cmd);
bool app_test11( app_t *app , const char *cmd);

// input mux tests
bool app_test12( app_t *app , const char *cmd);     // formerly test05.
bool app_test14( app_t *app , const char *cmd);
bool app_test15( app_t *app , const char *cmd);

// adc refmux test.
bool app_test19( app_t *app , const char *cmd);


bool app_test20( app_t *app, const char *cmd, void (*yield)( void * ), void * yield_ctx) ;

bool app_test40( app_t *app, const char *cmd, void (*yield)( void * ), void * yield_ctx) ;
bool app_test41( app_t *app, const char *cmd, void (*yield)( void * ), void * yield_ctx) ;
bool app_test42( app_t *app, const char *cmd, void (*yield)( void * ), void * yield_ctx) ;



