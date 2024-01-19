

#pragma once

#include "lib2/cbuffer.h"
#include "lib2/cstring.h"
#include "lib2/fbuffer.h"

// needed for forward declaration of MAT. it's a typedef anonymous struct. hard to declare.
// https://stackoverflow.com/questions/10249548/forward-declaring-a-typedef-of-an-unnamed-struct
#include "mesch12b/matrix.h"

typedef struct Mode Mode;




typedef struct H
{
  /* state, that needs to be persisted - . not managed on 4094 or fpga.  required for range changes. etc.
  // actually should probably
  // could be placed in app_t.   or here. or somewhere else.
    -------------
    EXTR. should be grouped regardless. to make easy to move. restrict scope.
    actually think it should be in app.

    - this state is not necessary to set up - complete function - only to - so it's bb.
  */


  // bool     persist_fixedz;   // default off.
  // uint32_t lfreq;


} H;








typedef struct app_t
{

  /*
    keeping the mode vectors out of here. is nice.

  */

  CBuf console_in;
  CBuf console_out;



  ////
  CString     command;


  uint32_t spi;

  uint32_t led_port;
  uint32_t led_out;

  // updated on interupt. should probably be declared volatile.
  // but functions that use can also declare volatile
  volatile uint32_t system_millis;


  // for soft timer functions
  uint32_t soft_100ms ;
  uint32_t soft_500ms ;
  uint32_t soft_1s ;



  ////////
  // not sure what the best way is to handle this state.
  // could be interesting to have an uptime.
  // unsigned count;

  bool led_state ;     // should rename, or just use the last bit of the count .


  /*
    rather than an enum for the test - this could be changed to a dispatch function
    that return contexts back  - to a long running function, that yielded control.
    eg. quite simple and effective.
    -------

    - Or more simply. just take over the main loop.
    - and process it.
  */

  // void (*yielded_function)( app_t * ) ;

  /*
    - need to remove this. causes too much confusion with our other loop stuff.
    - just need a lightweight buffer - to capture reset/stop.
    - or do it three times and exit.
    - rewrite test03, to click the relays. to run in a non-yielding loop.

  // TODO remove.
  */
  unsigned test_in_progress; // enum. for test type.


  // fpga comms is active/ok
  bool comms_ok;


  // we don't/shouldn't even need the current 4094/fpga state recorded/duplicated here.
  // not not want more than single authoritative source for 4094 state.

  /*
    mode_inthese are here to aid  access.
    use pointer to keep opaque and easier

  */

  const Mode *mode_initial;
  Mode *mode_current;


  /*
      linefreq is environment-state.
      and 10Meg. is operator desirable state that presists across range changes.
      nplc - is independent. but setting aperture requires linefreq. (perhaps fpga should deal in nplc, rather than aperture?).

      - we need to see how the scpi does it. does nplc have to be set each time change a range?

      -------
      - state that persists across range changes
      - perhaps belongs in separate struct.
      - could even be stored on fpga if
      ----
      - alterantively nplc - can be read from fpga.
      - and 10Meg. can be read from mode_current.

      - yes. dig it out of mode_current.   to persist. - at least for nplc.
      - and linefreq - should be added - even if not written.
      - actually cannot rely on relay state for 10Meg input impedance. because may be 1kV. range.
      - but just needs a flag. in mode.
      -----
      - we need to see how the scpi does it.

      linefreq

      // maintainin here - means that we don't have to, copy out the value from fpga, when setting dc range. or testing if unset.
      nplc
      // we cannot disginguish - hv range, and whether 10Meg. should persist across range change. so persist this state.
      10Meg.
  */

  uint32_t lfreq;


  /*
      info that has to be persisted across range changing/state change.
      but which doesn't belong  on fpga/4094.
      and we cannot determine in other ways..
  */

  bool persist_fixedz ;             // fixedz to use, when changing from a different voltage range.

  uint32_t persist_azmux_val;      // azmux lo to use if using azmode. eg. when change from a non-az mode. 
                                  // TODO change to uint8_t. it's only 4 bits.



  //////////////////////////

  uint32_t last_reg_status ;  // to detect status change, from fpga.


  /* have a variable to encode the sample_acquisition. that codes himux,lowmux etc.
    enables switching between az, noaz, boot.
    Not. sure it can all be encoded in F register.  as combinations.

    BUT we do want a way to choose the sample more easily. and switch between az, and no-az, electrom.
            eg. dcv,dev-source, dci. ref-lo, ref-hi etc.

  */

  // TODO reanme adc_valid . eg. same identifier / meaning as fpga code.
  // could also put flags/ for adc state in the status register. eg. the monitor pins.
  volatile bool  adc_measure_valid;

  bool adc_measure_valid_missed; // could be made a count


  unsigned verbose;     // treated as bool at the moment

  /////////////////
  unsigned model_cols; // to use.

  MAT *b;           // cal_b or model_b ?

  //////////////////

  // maybe change name  sa_buffer  acquis_buffer  meas_buffer.
  // change name sa_buffer
  MAT  *sa_buffer;

  // overflow index. rename _overflow, _wraparound
  unsigned sa_count_i;    /* has two needs.
                                   - for modulo indexing sa_buffer
                                    - determining when to stop.
                                    - actually we - may want to stamp - from the fpga .
                                    // wrap around can be handled. if == MAX( ) .
                            */

  // unsigned sa_count_stop;       // sample acquisition terminate condition. change name sa_count_i. change name done. sa_p_count_finish


  // we need behavior on overflow. no.
  // continuous/stopping sampling after buffer is full. is independent of whether we want to keep populating. i thinik we do.


  /*  there is no reason for complicated stamping of sample data.   eg. hi, lo, hi, lo.
      instead just convert early.
  */

  double   hi;        // for az mode. the last hi measure TODO change name az_hi. and az_lo ?
  double   lo[2];   // for az mode. the last two lo signals.


} app_t;




// better name
// app_transition_state() ?
void app_transition_state( unsigned spi, const Mode *mode, volatile uint32_t *system_millis);

// TODO - move . these are not app functions.
// consider moving to calc.
uint32_t nplc_to_aper_n( double nplc, uint32_t lfreq );
double aper_n_to_nplc( uint32_t aper_n, uint32_t lfreq);
double aper_n_to_period( uint32_t aper_n);
uint32_t period_to_aper_n(  double period );
bool nplc_valid( double nplc );


const char * azmux_to_string( uint8_t azmux );
const char * himux_to_string( uint8_t himux, uint8_t himux2 );



// helper, util. better name.  pass the filestream explicitly?
void aper_n_print( uint32_t aperture,  uint32_t lfreq);




bool test05( app_t *app , const char *cmd);
bool test06( app_t *app , const char *cmd);

bool test08( app_t *app , const char *cmd);

bool test11( app_t *app , const char *cmd);

bool test15( app_t *app , const char *cmd);    // not sure if good to mode_initial here,
bool test14( app_t *app , const char *cmd);    // not sure if good to pass here,

bool test16( app_t *app , const char *cmd);    // not sure if good to pass here,


bool app_functions( app_t *app , const char *cmd);

////////////


/*
  doing cal is  not really a loop. it is a long running function.
    - and we get timer interupts,
    - and data ready interupts.

  only if we pump the queue.

  TODO. so change name it is not a loop.
      it is only  loop in the sense that it is slow running. and blocks waiting  to get data.

*/

void app_cal( app_t *app/*, Loop3 *loop3 */);



