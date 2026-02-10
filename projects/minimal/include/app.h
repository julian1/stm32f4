
#pragma once

#include <lib2/cbuffer.h>
#include <lib2/cstring.h>

// #include <devices.h>


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


// typedef struct devices_t devices_t;

typedef struct _mode_t _mode_t;

typedef struct data_t data_t;
typedef struct gpio_t gpio_t;


typedef struct spi_t spi_t;
typedef struct spi_ice40_t spi_ice40_t;
typedef struct interrupt_t interrupt_t;







/*
  TODO nov 2024.

  rather than using pointers. should instantiate the memory properly here
  No. hide state here.
  But instantiate all state in main.c or appc

*/


#define APP_MAGIC   456




typedef struct app_t
{
  uint32_t magic;


  bool led_state ;          // for mcu. maybe change name to distinguish
                            // TODO consider remove.  and query the state of status_led .  eg. gpio_read( )
                            // not sure the point is to communicate it.

  bool led_blink_enable;    // for analog board. whether to blink the led on the analog board.
                            // useful activity indicator, but also need to be able to to suppress


  gpio_t        *gpio_status_led;

  uint32_t      soft_500ms;

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


  /*
    TODO.  rename tick_millis .  for system tick.
    separate out the tick_millis . that always updates with += 1000;
    from the sleep. that can be set where used.

    And - perhaps hide it.
  */

  volatile uint32_t system_millis;

  // volatile int32_t sleep_millis;  // signed count down.


  // TODO - consider make pointers if want opaque structure .  instantiate in main.
  // rename cbuf_cbuf_console_in
  cbuf_t        cbuf_console_in;
  cbuf_t        cbuf_console_out;

  cstring_t     command;




  ////////////////////////////////

  // analog board devices
  // devices_t   devices;

  spi_ice40_t   *spi_fpga0_pc;    //  fpga pre-configuration

  spi_t         *spi_fpga0;       // fpga post-configuration - register set etc

  interrupt_t   *fpga0_interrupt; // TODO review. not clear if really belongs here.
                                  // depends if we configured more than once
  spi_t         *spi_4094;

  spi_t         *spi_mdac0;     // consider rename sts_mdac?

  spi_t         *spi_mdac1;     // rename iso_sts_mdac?

  gpio_t      *gpio_trigger_internal;

  /* trigger selection - ext/int belongs here, as a device.
    it should be managed by the mode.

    - trigger should perhaps be put here.
  */
  gpio_t        *gpio_trigger_selection;

  ////////////////////////////////





  ////////////////////////////////

  // power board devices

  // separate system.
  spi_ice40_t   *spi_u202;        // rename spi_fpga1?.

  interrupt_t   *interrupt_u202;

  ////////////////////////////////



  // feb 2026.  current mode
  _mode_t       *mode;


  //////////////////////////////////

  // consider moving to app.

  volatile bool  adc_interupt_valid;

  bool adc_interupt_valid_missed; // could make a count



  data_t        *data;

  // buffer_t  *buffer;

  bool verbose;

} app_t;





void app_rdy_interupt( app_t *app, interrupt_t *x);
// void app_rdy_clear( app_t *app);




void app_beep( app_t * app, uint32_t n);
void app_led_dance( app_t * app );



void app_init_console_buffers(app_t *app);
// void app_loop(app_t *app);
void app_systick_interupt(app_t *app);


void app_update_main(app_t *app);


void app_configure( app_t *app );

void app_update_simple_led_blink(app_t *app);
void app_update_simple_with_data(app_t *app);



void app_transition_state( app_t *app  /*, uint32_t update_flags */);



void app_cal( app_t *app);


void app_cal2( app_t *app );









/*

void app_yield_with_delay(app_t *app, uint32_t delay );
void app_yield(app_t *app);

*/


// aug 2025.
// using this, means we dont need to include peripheral/gpio.h everywhere
void app_trigger_internal( app_t *app, bool val );



bool app_repl_statement(app_t *app,  const char *cmd);
void app_repl_statements(app_t *app,  const char *s);

/*
  IMPORTANT - none of these take arguments anymore,
  the precondition state, should be setup in other commands.
  so we could remove the cmd argument.  and do the test in the repl command.
  but leave for moment, in case want to be able to pass flags.
  --------

  should pass a more limited structure than app.

    eg. not sure.
*/


// from /test
bool app_test01( app_t *app , const char *cmd);
bool app_test02( app_t *app , const char *cmd);


// adc refmux test.
bool app_test08( app_t *app , const char *cmd);

// sa/adc test
bool app_test09( app_t *app , const char *cmd);


// dcvsource
bool app_test10( app_t *app , const char *cmd);
bool app_test11( app_t *app , const char *cmd);

// input mux tests
bool app_test12( app_t *app , const char *cmd);     // formerly test05.
bool app_test14( app_t *app , const char *cmd);
bool app_test15( app_t *app , const char *cmd);



bool app_test20( app_t *app, const char *cmd, void (*yield)( void * ), void * yield_ctx) ;

bool app_test40( app_t *app, const char *cmd, void (*yield)( void * ), void * yield_ctx) ;
bool app_test41( app_t *app, const char *cmd, void (*yield)( void * ), void * yield_ctx) ;
bool app_test42( app_t *app, const char *cmd, void (*yield)( void * ), void * yield_ctx) ;



