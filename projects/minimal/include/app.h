
#pragma once

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>   // size_t


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

      while(!app->decode_ready)
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


typedef struct cbuf_t cbuf_t;
typedef struct cstring_t cstring_t;

typedef struct gpio_t gpio_t;
typedef struct spi_t spi_t;
typedef struct spi_ice40_t spi_ice40_t;
typedef struct interrupt_t interrupt_t;
typedef struct cal_t cal_t;
typedef struct range_t range_t;
typedef struct _mode_t _mode_t;
typedef struct data_t data_t;
typedef struct decode_t decode_t;
typedef struct buffer_t buffer_t;


typedef struct vfd_t vfd_t;
typedef struct display_vfd_t display_vfd_t;

typedef struct tft_t tft_t;
typedef struct agg_test_t agg_test_t;



#define APP_MAGIC   456




typedef struct app_t
{
  uint32_t magic;


  bool led_state ;          // for digital/mcu board.
                            // TODO consider remove.  and just query for the current state of status_led .  eg. gpio_read( )
                            // not sure the point is to communicate it.

  // bool led_blink_enable;    // for analog board. whether to blink the led on the analog board.
                            // useful activity indicator, but also need to be able to to suppress


  gpio_t        *gpio_status_led;

  uint32_t      soft_500ms;

  // updated on interrupt. should probably be declared volatile.
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

    or millis_up. for timing code sections.  and millis_down for timers.
    handle rollover, by just adjusting at code point where no that nothing depends on the code.
    millis_down_500.  wants to be dedicated.
    millis_down_yield   yield or sleep
    -------------

    rename the timer. include soft. tick implies soft_ count
    or just tick_up.  and perhaps tick_down;  as general purpuse.
        soft_millis_up;     for timing functions,
        soft_millis_yield;   make down counter.
  */


  /* consider make system_millis a pointer in app_t.
    since it is passed/shared to other structures at _init (eg. agg_test).
    and instantiate it here in main.c

  */
  volatile uint32_t *system_millis;

  // volatile int32_t sleep_millis;  // signed count down.

  ////////


  cbuf_t        *cbuf_console_in;
  cbuf_t        *cbuf_console_out;
  cstring_t     *command;


  interrupt_t   *interrupt_systick;


  ////////////////////////////////


  spi_ice40_t   *spi_fpga0_pc;    //  fpga pre-configuration

  spi_t         *spi_fpga0;       // fpga post-configuration - register set etc

  interrupt_t   *interrupt_fpga0;


  spi_t         *spi_4094;

  spi_t         *spi_mdac0;     // consider rename sts_mdac?

  spi_t         *spi_mdac1;     // rename iso_sts_mdac?

  gpio_t        *gpio_trigger;


  /* trigger selection is just a regular device.
      even if it is on the digital board, not analog board.
      state is managed on the mode.
      TODO .  consider rename.   gpio_trigger_selection. less confusion.
  */
  gpio_t        *gpio_trigger_source;



  ////////////////////////////////

  // power board devices

  // separate system.
  spi_ice40_t   *spi_fpga1_pc;

  spi_t         *spi_fpga1;

  interrupt_t   *interrupt_u202;    // TODO rename

  ////////////////////////////////



  // feb 2026.  current mode
  _mode_t       *mode;


  //////////////////////////////////


  volatile bool adc_interrupt_valid;

  bool          adc_interrupt_valid_missed; // could be a count



  ////////////////////////

  // line_freq is environment property
  // does it belong in cal or data?
  // issue is that the tests code wants easy access
  uint32_t      *line_freq;

/*
  // not sure these will ever be used outside the context of cal_t
  // so could move them there.
  unsigned      cal_id;
  double        cal_w;
*/

  //////////////

  range_t       *ranges;      // including cal co-efficients
  size_t        ranges_sz;
  unsigned      *range_idx;    // active range TODO rename active_range_idx.

  ////////////////////////


  // there is a bit of confusion (or potential for state mismatch) between range and the active mode.
  // but we need to permit the mode to deviate (eg. for adc settings/nplc, LTS,  etc).

  ////////////

  cal_t         *cal;     // array

  decode_t      *decode;

  buffer_t      *buffer;


  // not sure belongs here
  // bool verbose;


  // state variable persists
  bool          repl_trigger_val;

  // TODO consider rename repl_force_trigger.
  bool          repl_retrigger;

  /* choice, is putting this in app or in mode.
    - it is bad to put it mode, because it is not actually state written to the board.
    - for mode_reset, we really expect a clear/fixed state point. and putting it in mode ensures this.
    - but this is a rangeing concept, and there are only a few places where use ranges, and need to consider it
    */
  bool          range_10Meg ;


  vfd_t         *vfd0;    // required for call to init.  after fpga initialized.

  display_vfd_t *display_vfd;   // rename display0 ?

  tft_t         *tft;


  agg_test_t    *agg_test;

} app_t;



// void app_init( app_t *app);

void app_update( app_t *app);


void app_interrupt_systick( app_t *app, void *arg);
void app_interrupt_data_rdy( app_t *app, void *arg);


void app_yield( app_t *app);
void app_msleep( app_t *app, uint32_t delay);


/*
  consider should type these two funcs on spi_t and system_millis
  - just the same as spi_print_register()_
    and spi_repl_reg_query()
  ------
  except sleep() also yields which needs additional context from app.

*/
void app_beep( app_t * app, uint32_t n);
void app_led_dance( app_t * app );


void app_configure( app_t *app );

void app_transition_state( app_t *app  /*, uint32_t update_flags */);

bool app_repl_statement(app_t *app,  const char *cmd);
void app_repl_statements(app_t *app,  const char *s);


////////


// range functions
bool app_range_dir_valid( app_t *app, uint32_t range_idx, bool dir);
void app_range_switch( app_t *app, uint32_t range_idx);
void app_range_switch1( app_t *app, const char *name, const char *arg);
bool app_repl_range( app_t *app, const char *cmd);


// consider move to test/test.h.

bool app_test_repl_statement( app_t *app,  const char *cmd);


