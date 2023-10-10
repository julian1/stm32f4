/*
  nix-shell ~/devel/nixos-config/examples/arm.nix
  rlwrap -a picocom -b 115200 /dev/ttyUSB0

  nix-shell ~/devel/nixos-config/examples/arm.nix
  make

  nix-shell ~/devel/nixos-config/examples/arm.nix
  cd smu11
  openocd -f ../../openocd.cfg

  nix-shell ~/devel/nixos-config/examples/arm.nix
  rlwrap nc localhost 4444
  reset halt ; flash write_image erase unlock ./main.elf; sleep 1500; reset run


// vim :colorscheme default. loooks good.

// cjmcu  stm32f407.
// issue. is that board/stlink doesn't appear to reset cleanly. needs sleep.

// - can plug a thermocouple straight into the sense +p, +n, and
// then use the slow (digital) integrator/pid loop - as a temperature controler..


*/


#include <libopencm3/stm32/rcc.h>   // clock
#include <libopencm3/stm32/gpio.h>    // led
#include <libopencm3/stm32/spi.h>   // SPI1

#include <libopencm3/cm3/scb.h>  // reset()


#include <stdio.h>    // printf
#include <string.h>   // strcmp, memset


// library code
#include "usart.h"
#include "assert.h"
// #include "cbuffer.h"
// #include "cstring.h"
#include "streams.h"
#include "util.h"


#include "spi-port.h"
#include "spi-ice40.h"
#include "mux.h"
#include "reg.h"



#include "fbuffer.h"

#include "format.h"   // format_bits()



// app structure
#include "app.h"

// #include "dac8734.h"
#include "4094.h"



static void spi1_interupt(app_t *app)
{
  UNUSED(app);
#if 0
  /*
    interupt context. avoid doing work here...
  */
  if(app->adc_drdy == true) {
    // still flagged from last time, then code is too slow, and we missed an adc read
    ++app->adc_drdy_missed;
  }

  // set adc_drdy flag so that update() knows to read the adc...
  app->adc_drdy = true;
#endif
}



static void sys_tick_interupt(app_t *app)
{
  // interupt context. don't do anything compliicated here.

  ++ app->system_millis;
}




static void update_soft_1s(app_t *app)
{
  // maybe review this...
  UNUSED(app);


}







static void state_format ( uint8_t *state, size_t n)
{
  assert(state);

  char buf[100];
  for(unsigned i = 0; i < n; ++i ) {

    printf("v %s\n",  format_bits(buf, 8, state[ i ]  ));
  }
}


/*
  EXTR. we can do the three step sequence of b2b fets in two steps. relying on faster speed

  1. turn off b2b fets.   turn on relay.   relay slower.
  2. turn off latch to relay. and turn on b2b fets.         <- so this is more than a mask.
  -----------------

  OR.    have two bits. and the interpreter.   will manage

  Eg. b2b-fets stage1. b2b-fets stage2.   - and encode... issue is that noo

  --------
  - So we don't duplicate everything.  just if a relay has a different state.
  - OR we just encode all states - as two states.  transition
  - eg. just double  the bitvector.
  ---
  - then we can encode.   - and don't need messy mask abstractions.

*/

#define MODE_ARR_N    3   // mode array in bytes.


// REMEMBER this is 4094.   not fpga state.

typedef struct X
{
  // U406 4094.
  uint8_t U408_SW_CTL : 1;      // perhaps change to lower case....   eg. u408_sw
  uint8_t U406_UNUSED : 1;
  uint8_t K406_CTL    : 2;        // Be better to encode as 2 bits.   can then assign 0b01 or 0b10 etc.
  uint8_t U406_UNUSED_2 : 4;

  // U401 4094
  uint8_t U401_UNUSED : 8;    // controls U404.


  // U403
  uint8_t U403_UNUSED : 6;
  uint8_t K405_CTL    : 2;


  // U506
  uint8_t U506_UNUSED : 8;

  // jumpered.
  // 600
  // 700

  // U1004  - 5th bit.
  uint8_t U1003   : 4;    // adg1208  4 bits.  EN last. is inconsistent.  with chip pin-order. and 500, 600, 700.  good keep.....
  uint8_t U1006   : 4;    // adg1208  4 bits.


} X;



// mode_t
typedef struct Mode
{
  // put AZ mux here. also.
/*
    Actually can even put a mode.
    here .
    and then switch what gets written.
*/

  X     first;
  X     second;

} Mode;


//////////

/*
      SPI_INTERUPT_OUT,
      MEAS_COMPLETE_CTL,
      CMPR_LATCH_CTL,
      adcmux,                  // 19 bits.

      monitor,                // 15.    bit 14 from 0.. + 8= j    bit 10,    1024.
      LED0,                   // bit 13.  8192.
      SIG_PC_SW_CTL,
      himux2,              // remove the himux2  12.
      himux,
      azmux

  so put in direct mode. then try to bllink the led.
  so try to blink
  assert( sizeof(F) == 4);
*/

//  __attribute__((__packed__))

typedef struct  __attribute__((__packed__))
F
{
  /* this is direct mode state.  TODO rename
      in adc mode,   we would have 2 or four sets of mux registers for the values to switch.
      and the other lines would be given to the fpga to run
      ----
      and this state would be written in the main mode state.
      -------
  */

  uint8_t azmux   : 4;
  uint8_t himux   : 4;
  uint8_t himux2  : 4;     // 12
  uint8_t sig_pc_sw_ctl : 1;
  uint8_t led0    : 1;       // 14   2 bytes.

  uint8_t monitor : 8;    // 22  // this bit vector overflows - so gets aligned on a new byte boundary. which is not what we want...

  uint8_t adcmux : 4;     // 26
  uint8_t cmpr_latch_ctl : 1;
  uint8_t meas_complete_ctl : 1;
  uint8_t spi_interupt_ctl : 1;     // 29bits

  uint8_t dummy   : 3;
} F;





/*
  EXTR.
    don't care about defining inidividual registers for muxes etc.
    instead the entire state representation is considered as register. with a pre-determined set of modes / elements.

    - the clearing mask for relays, is normally always the same. but the need to manipulate b2b fets changes thing.
    - with a straight array.   WE *CAN* also define using a parallel alternative structure with bitfield.
  ----
    - sequencing - may also want to switch relays, wait. then turn on the analog switches.

    - EXTR. THE state of ALL relays must be defined.  0 just means use prior state.  which is wrong.  either L1, or L2.  not 0.

    - It would be easier to do this with a memcpy.

                                      first transition            // second transition
                                      U406    U401
*/

// is wrong. we have to switch all the relays to a defined state


// Mode mode_zero;     //  useful to work out which pin is flipping .
// change name mode_initial. to mode-off/ or disconnect
Mode mode_initial;      // all inputs turned off.

Mode mode_dcv_az ;

/* IMPORTANT the precharge switch relay - is held constant/closed for the mode.
  but we cycle one of the muxes - to read gnd/ to reset the charge on the cap.
  we can also. use dc-source to add an initial voltage bias to cap.
*/

// Mode mode_test_accumulation;


static void init_modes( void )
{
  /*
    instead of having individual registers. we have individual elemental mode states , that exist.

    is there an alignment issue... with setting the bitfield???

  */

  // memset( &mode_zero,    0, sizeof( Mode) );
  memset( &mode_initial, 0, sizeof( Mode) );
  memset( &mode_dcv_az,  0, sizeof( Mode) );
  // memset( &mode_test_accumulation,  0, sizeof( Mode) );

  /*
    ALL relays must be given a pulse here. to get to default state.
    cannot just leave at 0V.
  */

  //  should be explicit for all values  U408_SW_CTL. at least for the initial mode, from which others derive.
  mode_initial.first .K406_CTL  = 0b10;     // accumulation relay off
  mode_initial.first .U408_SW_CTL = 0;      // b2b fets/ input protection off/open
  mode_initial.first. K405_CTL  = 0b01;     // dcv-input relay k405 switch off

  mode_initial.second.K406_CTL  = 0b00;     // clear relay
  mode_initial.second.U408_SW_CTL = 0;
  mode_initial.second.K405_CTL  = 0b00;     // clear relay

  /* EXTR. with series resistors to 4094 - for driving lower coil-voltage relays - there is a drop on the drive side.
      AND a drop on the lo side.  eg. with 50R. this looks like a poor on-pulse

  */

  //////////
  mode_dcv_az = mode_initial;               // eg. turn all relays off
  mode_dcv_az.first.K405_CTL    = 0b10;     // turn dcv-input K405 on.
  mode_dcv_az.first.U408_SW_CTL = 0;        // turn off b2b fets, while switching relay on.
  mode_dcv_az.second.K405_CTL    = 0b00;    // clear relay.  don't really need since inherits from initial.
  mode_dcv_az.second.U408_SW_CTL = 1;       // turn on/close b2b fets.


  ////
  // cap-accumulation mode.
  // mode_test_accumulation = mode_initial;
  // mode_test_accumulation.first .K406_CTL  = 0b01;  // accumulation relay on

}





// flashing of led... is writing ????


static void do_4094_transition( unsigned spi, Mode *mode, uint32_t *system_millis)
{

  // change name   do_state_update_4094 _4094_state_update.
  // change name do_4094_transition. or make_

  // should we be passing as a separate argument
  assert( sizeof(X) == 5 );


  // mux spi to 4094. change mcu spi params, and set spi device to 4094
  mux_4094( spi);


  printf("-----------\n");

  printf("do_4094_transition write first state\n");
  // state_format (  (void *) &mode->first, sizeof(X) );

  // and write device
  spi_4094_reg_write_n(spi, (void *) &mode->first, sizeof( X) );

  // sleep 10ms
  msleep(10, system_millis);


  // and format
  printf("do_4094_transition write second state\n");
  // state_format ( (void *) &mode->second, sizeof(X) );

  // and write device
  spi_4094_reg_write_n(spi, (void *) &mode->second, sizeof(X) );
}




static void update_soft_100ms(app_t *app)
{
  assert(app);

  // use for tests.

  if(app->test_in_progress == 2 ) {

      mux_ice40(app->spi);
      static uint32_t magic = 0;      // static. should put in app state
      uint8_t reg =  REG_LED;

      spi_ice40_reg_write32(app->spi, reg, magic);
      uint32_t ret = spi_ice40_reg_read32( app->spi, reg);

      // perhaps it's taking a long time??
      char buf[ 100] ;
      printf("r %u  v %lu %s %s\n", reg, ret, format_bits(buf, 32, ret), magic == ret ? "ok" : "error");
      ++magic;

  }
}






static void update_soft_500ms(app_t *app)
{
  // UNUSED(app);
  assert(app);

  /*
    function should reconstruct to localize scope of app. and then dispatch to other functions.

  */

  app->led_state = ! app->led_state;



  // blink mcu led
  // be explicit. don't hide top-level state.
  if(app->led_state)
    gpio_clear( app->led_port, app->led_out);
  else
    gpio_set(   app->led_port, app->led_out);




  // printf("count %u\n", ++ app->count);
/*
  // 500ms. heartbeat check here.
  // this works nicely
  {
    uint32_t magic = 0b1010;   // this is returning the wrong value....
    mux_ice40(app->spi);
    spi_ice40_reg_write32( app->spi, REG_LED, magic);
    uint32_t ret = spi_ice40_reg_read32( app->spi, REG_LED);
    if(magic != ret) {
      char buf[ 100] ;

      printf("no comms, wait for ice40 v %s\n",  format_bits(buf, 32, ret ));
      // return
      // or should probably do a reset. when comms re-established
    }
  }
*/

  mux_ice40(app->spi);

  // blink fpga led. will only work if mode. 1.
  spi_ice40_reg_write32( app->spi, REG_LED, app->led_state );
  uint32_t ret = spi_ice40_reg_read32( app->spi, REG_LED);

  if(ret != app->led_state) {
    // comms no good
    char buf[ 100] ;
    printf("no comms, wait for ice40 v %s\n",  format_bits(buf, 32, ret ));
    app->comms_ok = false;
    // return
  } else {
    // comms ok
    if( app->comms_ok == false) {
      // comms have been restored
      printf("comms ok\n");
      printf("> ");
      app->comms_ok = true;
    }
  }




  if(app->test_in_progress == 3 ) {

    // tests b2b and K405 relay sequencing.
    if(app->led_state)
      do_4094_transition( app->spi, &mode_initial, &app->system_millis );
    else
      do_4094_transition( app->spi, &mode_dcv_az, &app->system_millis );
  }


  if(app->test_in_progress == 4 ) {

    // mode is 3. blink led. by writing direct register. don't want to do this in other cases.
    F f;
    memset(&f, 0, sizeof(f));

    f.led0 = app->led_state;

    mux_ice40(app->spi);
    spi_ice40_reg_write_n(app->spi, REG_DIRECT, &f, sizeof(f) );
  }


}


////////////////////

#define CLK_FREQ        20000000

static uint32_t nplc_to_aper_n( double nplc )
{
  double period = nplc / 50.0 ;  // seonds
  uint32_t aper = period * CLK_FREQ;
  return aper;
}


static double aper_n_to_nplc( uint32_t aper_n)
{
  // uint32_t aper  = params->clk_count_aper_n ;
  double period   = aper_n / (double ) CLK_FREQ;
  double nplc     = period / (1.0 / 50);
  return nplc;
}


static double aper_n_to_period( uint32_t aper_n)
{
  double period   = aper_n / (double ) CLK_FREQ;
  return period;
}



static void update_console_cmd(app_t *app)
{


  while( ! cBufisEmpty(&app->console_in)) {

    // got a character
    int32_t ch = cBufPop(&app->console_in);
    assert(ch >= 0);

    if(ch != '\r' && cStringCount(&app->command) < cStringReserve(&app->command) ) {
      // normal character
      cStringPush(&app->command, ch);
      // echo to output. required for minicom.
      putchar( ch);

    }  else {

      // newline or overflow
      putchar('\n');

      char *cmd = cStringPtr(&app->command);

      // printf("cmd whoot is '%s'\n", cmd);


      uint32_t u0, u1;
      int32_t i0;

      ////////////////////


      if( strcmp(cmd, "reset") == 0) {
        printf("reset initial state\n");

        Mode j = mode_initial;
        do_4094_transition( app->spi, &j,  &app->system_millis );

        mux_ice40(app->spi);
        spi_ice40_reg_write32(app->spi, REG_MODE, 0 );  // defaultpattern.

        // make sure relay switching test is off.
        app->test_in_progress = 0;

      }


      else if(strcmp(cmd, "reset mcu") == 0) {
        // reset stm32f4
        // scb_reset_core()
        scb_reset_system();
      }


      else if( sscanf(cmd, "direct %lu", &u0 ) == 1) {

        printf("set direct value to, %lu\n", u0 );

        mux_ice40(app->spi);
        spi_ice40_reg_write32(app->spi, REG_DIRECT, u0 );

        uint32_t ret = spi_ice40_reg_read32(app->spi, REG_DIRECT );
        // printf("reg_direct return value %lu\n", ret);

        char buf[ 100 ] ;
        printf("r %u  v %lu  %s\n",  REG_DIRECT, ret,  format_bits(buf, 32, ret ));
      }


      else if( sscanf(cmd, "mode %lu", &u0 ) == 1) {

        mux_ice40(app->spi);
        spi_ice40_reg_write32(app->spi, REG_MODE, u0 );

        uint32_t ret = spi_ice40_reg_read32(app->spi, REG_MODE );
        printf("reg_mode return value %lu\n", ret);

      }

        // OK, it would be nice to support setting a  vector. over the command line.
        // issue is cannot do the relay switching.
        // there's definitely DA.


      else if( strcmp(cmd, "test02") == 0) {
        // spi stress test in 100ms soft timer
        printf("spi stress test spi comms\n");
        app->test_in_progress = 2;
      }



      else if( strcmp(cmd, "test03") == 0) {

        // don't care about mode. for 4094 stuff.
        printf("test03 use 4094, to click dcv sequencing/ b2b fet/ relays\n");
        app->test_in_progress = 3;
      }

      else if( strcmp(cmd, "test04") == 0) {

        printf("test04 use direct mode - to blink led\n");

        mux_ice40(app->spi);
        spi_ice40_reg_write32(app->spi, REG_MODE, MODE_DIRECT);

        app->test_in_progress = 4;
      }



      else if( strcmp(cmd, "test05") == 0 || strcmp(cmd, "test06") == 0 || strcmp(cmd, "test07") == 0) {

        // test leakage by going to a voltage, holding, then switchoff mux and observe
        // we can have a test and pass an argument if really want. but probably better to have self-contained. to automate

        printf("charge accumulation cap\n");
        app->test_in_progress = 0;
        Mode j = mode_initial;

        if(strcmp(cmd, "test05") == 0) {
          printf("with +10V\n");
          j.second.U1003  = S1 ;       // s1. dcv-source s1. +10V.
          j.second.U1006  = S1 ;       // s1.   follow  .   dcv-mux2
        }
        else if(strcmp(cmd, "test06") == 0) {
          printf("with -10V\n");
          j.second.U1003  = S2 ;       // s2.  -10V.
          j.second.U1006  = S1 ;       // s1.   follow  .   dcv-mux2
        }
        else if(strcmp(cmd, "test07") == 0) {
          printf("with 0V\n");
          j.second.U1003 = S3;          // s3 == agnd
          j.second.U1006 = S6;          // s6 = agnd  .  TODO change to S7 . populate R1001.c0ww
        }
        else assert(0);

        // turn on accumulation relay     RON ROFF.  or RL1 ?  K606_ON
        j.first .K406_CTL  = 0b01;
        j.second.K406_CTL  = 0b00;    // don't need this....  it is 0 by default

        do_4094_transition( app->spi, &j,  &app->system_millis );

        /////////////////
        // make sure we are in direct mode.
        mux_ice40(app->spi);
        spi_ice40_reg_write32(app->spi, REG_MODE, MODE_DIRECT );

        // now control the hi mux.
        F  f;
        memset(&f, 0, sizeof(f));
        f.himux2 = S1 ;    // s1 put dc-source on himux2 output
        f.himux  = S2 ;    // s2 reflect himux2 on himux output

        f.sig_pc_sw_ctl  = 1;  // turn on. precharge.  on. to route signal.

        spi_ice40_reg_write_n(app->spi, REG_DIRECT, &f, sizeof(f) );

        ////////////////////////////
        // so charge cap to the dcv-source, then turn off the mux and see how it drifts.
        // charge for 10sec. for DA....
        printf("sleep 10s\n");  // having a yield would be quite nice here.
        msleep(10 * 1000,  &app->system_millis);
        printf("turn off muxes - to see drift\n");

        /////////
        memset(&f, 0, sizeof(f));         // turn off the muxes  .   we could turn dc-source to 0V.

        f.himux2 = S4 ;                // s4 gnd.
        spi_ice40_reg_write_n(app->spi, REG_DIRECT, &f, sizeof(f) );

        // charge cap +10V hold 10sec. get 0.8mV / 10 s.  0.8pA. leave few minutes. 0.6mV. == 0.6pA.  15mins == 0.4pA.
        // charge cap -10V hold 10sec. get around 2mV / 10s. == 2pA.  input pin, doesn't make much difference to negative leakaave
        // charge cap 0V.  hold 10sec. get around 2.3 -> 2.6mV. == 0.3mV/.  0.3pA.

      }


      else if( strcmp(cmd, "test08") == 0 || strcmp(cmd, "test09") == 0 || strcmp(cmd, "test10") == 0) {

        // test leakage by reset and holding at 0V. while putting a voltage on the cri mux.

        printf("reset accumulation cap to 0V/agnd\n");
        app->test_in_progress = 0;
        Mode j = mode_initial;

        if(strcmp(cmd, "test08") == 0) {
          printf("with +10V\n");
          j.second.U1003  = S1 ;       // s1. dcv-source s1. +10V.
          j.second.U1006  = S1 ;       // s1.   follow  .   dcv-mux2
        }
        else if(strcmp(cmd, "test09") == 0) {
          printf("with -10V\n");
          j.second.U1003  = S2 ;       // s2.  -10V.
          j.second.U1006  = S1 ;       // s1.   follow  .   dcv-mux2
        }
        else if(strcmp(cmd, "test10") == 0) {
          printf("with 0V\n");
          j.second.U1003 = S3;          // s3 == agnd
          j.second.U1006 = S6;          // s6 = agnd  .  TODO change to S7 . populate R1001.c0ww
        }
        else assert(0);

        // turn on accumulation relay     RON ROFF.  or RL1 ?
        j.first .K406_CTL  = 0b01;
        j.second.K406_CTL  = 0b00;    // don't need this....  it is 0 by default

        do_4094_transition( app->spi, &j,  &app->system_millis );

        /////////////////
        // make sure we are in direct mode.
        mux_ice40(app->spi);
        spi_ice40_reg_write32(app->spi, REG_MODE, MODE_DIRECT );

        // now control the hi mux.  to reset the cap.
        F  f;
        memset(&f, 0, sizeof(f));
        f.himux2 = S4 ;    // s4 mux ground to reset cap.
        f.himux  = S2 ;    // s2 reflect himux2 on himux output
        spi_ice40_reg_write_n(app->spi, REG_DIRECT, &f, sizeof(f) );

        // it actually needs a kind of double transition.

        ////////////////////////////
        // so reset cap to 0V/agnd
        printf("sleep 10s\n");  // having a yield would be quite nice here.
        msleep(10 * 1000,  &app->system_millis);
        printf("turn off muxes - to see drift\n");


        // now float/turn of the  himux while leaving the input voltage on the pin.
        memset(&f, 0, sizeof(f));      // turn off everything
        f.himux2 = S1 ;                 // s1 put dc-source on himux2 output
        f.himux = SOFF;                // float himux
        spi_ice40_reg_write_n(app->spi, REG_DIRECT, &f, sizeof(f) );

        // with either +-10V. at input of himux, but with himux turned off,  we get about 1pF. rising.
      }

    /*
      EXTR. - the above tests need to ensure that the bootttrapped pre-charge switch is turned off.
            - else loading on switch.

    */





      else if( strcmp(cmd, "test11") == 0 || strcmp(cmd, "test12") == 0 || strcmp(cmd, "test13") == 0) {

        printf("test normal AZ modulation at 10V,-10V,0V/agnd\n");
        app->test_in_progress = 0;
        Mode j = mode_initial;

        if(strcmp(cmd, "test11") == 0) {
          printf("with +10V\n");
          j.second.U1003  = S1 ;       // s1. dcv-source s1. +10V.
          j.second.U1006  = S1 ;       // s1.   follow  .   dcv-mux2
        }
        else if(strcmp(cmd, "test12") == 0) {
          printf("with -10V\n");
          j.second.U1003  = S2 ;       // s2.  -10V.
          j.second.U1006  = S1 ;       // s1.   follow  .   dcv-mux2
        }
        else if(strcmp(cmd, "test13") == 0) {
          printf("with 0V\n");
          j.second.U1003 = S3;          // s3 == agnd
          j.second.U1006 = S6;          // s6 = agnd  .  TODO change to S7 . populate R1001.c0ww
        }
        else assert(0);

        // accumulation relay is off
        do_4094_transition( app->spi, &j,  &app->system_millis );

        /////////////////
        // setup az mode
        mux_ice40(app->spi);
        spi_ice40_reg_write32(app->spi, REG_MODE, MODE_AZ );  // mode 3. test pattern on sig

        //////////////
        // use DIRECT0 to set lo sample, in azmode.
        F  f;
        memset(&f, 0, sizeof(f));
        f.himux2 = S1 ;       // s1 put dc-source on himux2 output
        f.himux  = S2 ;       // s2 reflect himux2 on himux output
        f.azmux  = S6 ;    // s6 == normal LO for DCV, ohms.
        // f.azmux  = S2 ;    // s2 == BOOT for test.
        // f.azmux  = SOFF ;     // soff for high-z for test.
        spi_ice40_reg_write_n(app->spi, REG_DIRECT, &f, sizeof(f) );

        // use DIRECT 2 to set the hi sample
        memset(&f, 0, sizeof(f));
        f.azmux  = S1 ;         // s1 == PC_OUT (either SIG or BOOT).
        // f.azmux  = S2 ;      // s2 == BOOT for test.
        // f.azmux  = SOFF ;    // soff for high-z for test.
        spi_ice40_reg_write_n(app->spi, REG_DIRECT2, &f, sizeof(f) );

        // for precharge spinning the switch. want azmuxs for both samples to be high-z.
        // OK. want to be able to set the clk_duration.

        // behavior is quite different because of timing.

        // write the frequency. 10MHz. counter freq.
        spi_ice40_reg_write32(app->spi, REG_CLK_SAMPLE_DURATION, CLK_FREQ * 20e-3 );        // 1nplc, 20ms. freq == 25Hz for hi/lo period.
        // spi_ice40_reg_write32(app->spi, REG_CLK_SAMPLE_DURATION, CLK_FREQ * 200e-3 );    // 10nplc 200ms.

      }


    /*
      having arguments is ok. for internal test.  that just prints a result.

    */

    else if( sscanf(cmd, "test14 %ld %lu", &i0, &u1 ) == 2) {

        /*
        // test charge-injection by charging to a bias voltage, holding, then entering az mode.
        // with az-mux held off through cycle - which eliminates effect of leakage of amplifer (if fitted) input jfets.
        // first argument - bias voltage.  10,-10,0
        // second argument is nplc.
        // note t

        */
        printf("test leakage and charge-injection from switching pre-charge switch at different biases\n");
        app->test_in_progress = 0;
        Mode j = mode_initial;

        if(i0 == 10) {
          printf("with +10V\n");
          j.second.U1003  = S1 ;       // s1. dcv-source s1. +10V.
          j.second.U1006  = S1 ;       // s1.   follow  .   dcv-mux2
        }
        else if(i0 == -10) {
          printf("with -10V\n");
          j.second.U1003  = S2 ;       // s2.  -10V.
          j.second.U1006  = S1 ;       // s1.   follow  .   dcv-mux2
        }
        else if(i0 == 0) {
          printf("with 0V\n");
          j.second.U1003 = S3;          // s3 == agnd
          j.second.U1006 = S6;          // s6 = agnd  .  TODO change to S7 . populate R1001.c0ww
        }
        else assert(0);

        // turn on accumulation relay     RON ROFF.  or RL1 ?  K606_ON
        j.first .K406_CTL  = 0b01;
        j.second.K406_CTL  = 0b00;    // don't need this....  it is 0 by default

        do_4094_transition( app->spi, &j,  &app->system_millis );

        /////////////////
        // make sure we are in direct mode.
        mux_ice40(app->spi);
        spi_ice40_reg_write32(app->spi, REG_MODE, MODE_DIRECT );

        // now control the hi mux.
        F  f;
        memset(&f, 0, sizeof(f));
        f.himux2 = S1 ;    // s1 put dc-source on himux2 output
        f.himux  = S2 ;    // s2 reflect himux2 on himux output
        f.sig_pc_sw_ctl  = 1;  // turn on. precharge.  on. to route signal to az mux... doesn't matter.
        spi_ice40_reg_write_n(app->spi, REG_DIRECT, &f, sizeof(f) );

        ////////////////////////////
        // so charge cap to the dcv-source
        // and let settle 10sec. for DA....
        printf("sleep 10s\n");  // having a yield would be quite nice here.
        msleep(10 * 1000,  &app->system_millis);

        /////////////////
        // now change to az mode.
        printf("changing to az mode.\n");  // having a yield would be quite nice here.
        // setup az mode
        mux_ice40(app->spi);
        spi_ice40_reg_write32(app->spi, REG_MODE, MODE_AZ );  // mode 3. test pattern on sig

        //////////////
        // use direct register - for the lo sample, in azmode.
        memset(&f, 0, sizeof(f));
        f.himux2 = SOFF ;
        f.himux  = SOFF ;
        f.azmux  = SOFF ;
        spi_ice40_reg_write_n(app->spi, REG_DIRECT, &f, sizeof(f) );

        // set reg_direct2 controlling azmux hi val, the same. so only switching the pc switch.
        spi_ice40_reg_write_n(app->spi, REG_DIRECT2, &f, sizeof(f) );


        assert(u1 == 1 || u1 == 10 || u1 == 100 || u1 == 1000); // not really necessary. just avoid mistakes

         uint32_t aperture = nplc_to_aper_n( u1 );

        printf("aperture %lu\n",   aperture );
        printf("nplc     %.2lf\n",  aper_n_to_nplc( aperture ));
        printf("period   %.2lfs\n", aper_n_to_period( aperture ));

        spi_ice40_reg_write32(app->spi, REG_CLK_SAMPLE_DURATION, aperture );

        /// EXTR.   AHHHH. a reason the leakage and charge-accumulation results differ - voltage. 5V6 zener for 4.8V..  versus using 5V1 zener, may have been less..
        // can check the codes.


        // with 5V6 zener.  so giving boot supply rail of 4.7V.

        // 0ct 3.
        // test14 at 10V bias.
        // 100nplc / 2000ms   == +32mV.      leakage and charge injection.  similar to no switching.
        // 10nplc / 200ms     == +39mV.
        // 1nplc /20ms.       == +86mV.

        // same - new date. so we could test. soldering a lower voltage 4053. on.

        // oct 4.
        //              20s.  == +28mV     29mV.  23mV.  (may be a difference if pc switch starts on/of )
        // 100nplc / 2000ms   == +22mV.  22mV
        // 10nplc, 200ms.     == +29mV.   29mV.
        // 1nplc,  20ms.      == 82mV.   84mV.

        ////////////
        // 6v2 zener.  with 5.5V out.   5mins after soldering.
        //              20s  ==  47mV.
        // 100nplc / 2000ms   == 59mV.
        // 10nplc, 200ms.     == 43mV.  51mV.
        // 1nplc,  20ms.      == 108mV  107mV. 100mV (20mins after soldering).

        // so it's worse with higher voltage...
        // all incredibly strange.
        // Or perhaps we used a lower voltage zener for previous tests??  ta is our memory

        // 5.1V zener.  weird.   measure 4.7V across zener. giving 4V boot rail <- weird.  code D3L
        // 20s                29mV.
        // 100nplc / 2000ms.  24mV
        // 10nplc / 200ms.    31mV. 32mV.
        // 1nplc / 20ms       81mV. 81mV. 80mV.

        // ok. so not a lot of difference.
        // we should probably replace the dg508.  with a 1208.  that we received.

        // use 1208.  replacing maxmim 508. - don't actually expect much difference.after soldering.

        // 1000nplc / 20s     18mV.  28mV.  29mV.
        // 100nplc / 2000ms.  25mV   23mV
        // 10nplc / 200ms.    30mV  30mV
        // 1nplc / 20ms       78mV. 79mV.

        // after a few hours.

        /*


          Still exploring cmos switches for a bit,

          Briefly for sn74lv4053a one difference with the previous tests - is the zener used to set the boot supply rail.
          but tests show that a bootstrap supply rail between 4V to 5.5V doesn't matter much for leakage or charge injection.
          Also tried another sn74lv4053a, purchased a few years apart from the one used for initial tests, but with the same result.
          So i am not sure how to explain the discrepancy with previous test results.

          Running az modulation. all muxes fitted.
          DC accumulation on 10nF/ over 10s.

          test14.
          sn74lv4053a
          +10V dc bais
          1000nplc/off   20mV. 18mV.
          100nplc/2s     17mV. 17mV.
          10nplc/200ms   21mV. 22mV.
          1nplc/20ms     35mV. 73mV.  70mV.   large measured difference. odd. but was definltey there.


          But max4053 looks a lot better,
          I almost wasn't going to bother re-testing it, based on past resulsts.
          Identical setup as above - accumulation on 10nF/ 10s.

          max4053
          +10V dc bias.
          1000nplc/off   0.3mV. 0.5mV
          100nplc        0.8mV.
          10nplc         3.8mV.   3.6mV.
          1nplc          30mV.   28mV.

          max4053
          -10V dc bias.
          leave five minutes for +4.5mV/10s. cap DA to settle.
          1000nplc/off   2.5mV  2.8mV   - oct 8  2.3mV.
          100nplc        3.0mV. 3.3mV   - oct 8.  2.3mV
          10nplc         5.6mV.  5.7mV  - oct 8. 5.2mV.
          1nplc          30mV   30mV.   - oct 8  29mV.

          max4053
          0V dc bias.
          1000nplc/off   0.8mV.
          100nplc        1.0mV.  1mV.
          10nplc         3.8mV.  3.6mV.
          1nplc          28mV.

          leakage is more controlled -  <1pA for +10V and 0V, and <3pA for -10V dc-bias.

          for charge injection
          ie. 1nplc == 20ms.  10s/0.02s == 500 cycles.
          this is 30mV / 500 == 0.06pC .
          if I have the units correct, through full-cycle switch.

          The above tests were done with the azmux held off, with only the pre-charge switch switching.
          this would eliminate/isolate any leakage through the amplifer input jfets (if fitted) .


          test15.
          When the azmux also changed to for normal sampling beetween PC-OUT (S1) and LO (S6), the result is similar - eg good.

          max4053
          0V dc bias.
          1000nplc/off   1.3mV 1.2mV.
          100nplc        1.8mV
          10nplc         4.8mV
          1nplc          38mV. 37mV.

        */
      }


    else if( sscanf(cmd, "test15 %ld %lu", &i0, &u1 ) == 2) {

        /*
        // test charge-injection by charging to a bias voltage, holding, then entering az mode.
        // with az-mux also switching .
        // first argument - bias voltage.  10,-10,0
        // second argument is nplc.
        // note t

        */
        printf("test leakage and charge-injection from switching pre-charge switch at different biases\n");
        app->test_in_progress = 0;
        Mode j = mode_initial;

        if(i0 == 10) {
          printf("with +10V\n");
          j.second.U1003  = S1 ;       // s1. dcv-source s1. +10V.
          j.second.U1006  = S1 ;       // s1.   follow  .   dcv-mux2
        }
        else if(i0 == -10) {
          printf("with -10V\n");
          j.second.U1003  = S2 ;       // s2.  -10V.
          j.second.U1006  = S1 ;       // s1.   follow  .   dcv-mux2
        }
        else if(i0 == 0) {
          printf("with 0V\n");
          j.second.U1003 = S3;          // s3 == agnd
          j.second.U1006 = S6;          // s6 = agnd  .  TODO change to S7 . populate R1001.c0ww
        }
        else assert(0);

        // turn on accumulation relay     RON ROFF.  or RL1 ?  K606_ON
        j.first .K406_CTL  = 0b01;
        j.second.K406_CTL  = 0b00;    // don't need this....  it is 0 by default

        do_4094_transition( app->spi, &j,  &app->system_millis );

        /////////////////
        // make sure we are in direct mode.
        mux_ice40(app->spi);
        spi_ice40_reg_write32(app->spi, REG_MODE, MODE_DIRECT );

        // now control the hi mux.
        F  f;
        memset(&f, 0, sizeof(f));
        f.himux2 = S1 ;    // s1 put dc-source on himux2 output
        f.himux  = S2 ;    // s2 reflect himux2 on himux output
        f.sig_pc_sw_ctl  = 1;  // turn on. precharge.  on. to route signal to az mux... doesn't matter.
        spi_ice40_reg_write_n(app->spi, REG_DIRECT, &f, sizeof(f) );

        ////////////////////////////
        // so charge cap to the dcv-source
        // and let settle 10sec. for DA....
        printf("sleep 10s\n");  // having a yield would be quite nice here.
        msleep(10 * 1000,  &app->system_millis);

        /////////////////
        // now change to az mode.
        printf("changing to az mode.\n");  // having a yield would be quite nice here.
        // setup az mode
        mux_ice40(app->spi);
        spi_ice40_reg_write32(app->spi, REG_MODE, MODE_AZ );  // mode 3. test pattern on sig

        //////////////
        // use direct register - for the lo sample, in azmode.



        memset(&f, 0, sizeof(f));
        f.himux2 = SOFF ;
        f.himux  = SOFF ;
        f.azmux  = S6 ;    // s6 == normal LO for DCV, ohms.
        spi_ice40_reg_write_n(app->spi, REG_DIRECT, &f, sizeof(f) );

        // set the hi for az.
        f.azmux  = S1 ;         // s1 == PC_OUT (either SIG or BOOT).
        spi_ice40_reg_write_n(app->spi, REG_DIRECT2, &f, sizeof(f) );


        assert(u1 == 1 || u1 == 10 || u1 == 100 || u1 == 1000); // not really necessary. just avoid mistakes

         uint32_t aperture = nplc_to_aper_n( u1 );

        printf("aperture %lu\n",   aperture );
        printf("nplc     %.2lf\n",  aper_n_to_nplc( aperture ));
        printf("period   %.2lfs\n", aper_n_to_period( aperture ));

        spi_ice40_reg_write32(app->spi, REG_CLK_SAMPLE_DURATION, aperture );

        /*
        oct 6, 2023.
        max4053
        +10V dc bias
        1000nplc/off   1.0mV  0.2mV.    oct 8. 0.0mV.
        100nplc        0.2mV  0.2mV     oct 8. 0.3mV
        10nplc         1.0mV  1.2mV     oct 8. 0.7mV
        1nplc          20.5mV.  20.5mV  oct 8.  20mV

        max4053
        -10V dc bias
        wait for DA.
        1000nplc/off   3.8mV. 3.2mV    some DA from +10V test.  oct 8. 3.2mV.
        100nplc        2.5mV                                  oct 8 4.4mV.   3.9mV.
        10nplc         10mV. 10mV.                            oct 8 10mV.
        1nplc          56mV.  55mV.  56mV                     oct 8 56mV.

        max4053
        0V dc bias.
        1000nplc/off   1.3mV 1.2mV.                           oct 8. 1.6mV.
        100nplc        1.8mV                                  oct 8. 1.6mV
        10nplc         4.8mV                                  oct 8. 5.3mV.
        1nplc          38mV. 37mV.                            oct 8. 39mV.

        ------------
        oct 8.
        swap precharge polarity.
        1nplc          dc bias.
          0V            30mV.         32mV.  35mV.
          +10V          -2.6V  wow.
          -10V          +3.1V.


        I think it might make sense to try to trim - it with a capacitor.

        when the precharge phase is increased from 500us to 5ms.  sensitivity to dc-bias is reduced.

        +10V bias   1nplc    25mV. 25mV.
        0V   bias   1nplc    30mV. 30mV.
        -10V bias.  1nplc.   38mV. 38mV.

        spread == 38/25mV = 1.5x.

        using 3p trimmer.   measures 2.5p with LCR meter.

        bias.   accumulation.

        +10V     26mV.  26mV
        0V      32mV. 33mV.
        -10V    39mV. 37mV.

        10p.

        +10V    26mV.
        0V.    32mV.  33mV..
        -10V   40mV.

          ok. it's not working.
            needs to be sequenced in the state machine.
        */

/*
        ///////////////////
         oct 9.

         test15.
         revert to baseline. 500us precharge.
         max4053, 1nplc,
         +10V   21mV,  21mV
         0V     37mV.   37mV.
         -10V.  55mV.   58mV.

         I felt for-sure that tie-ing the bootin guard (currently floating) to gnd would do something, and change the loading.
         since there is quite a bit of copper sufrace area and proximity to mux-out.
         BOOTIN tied to gnd. also pin7 azmux. - no difference
         +10V.   20mV
         0V      39mV.  39mV.
         -10V.   55mV   56mV.

         remove cap C430. slowing pre-charge switching. - no difference.
         +10V    20.5mV.
         0V.     37mV.  39mV.
         -10V.   56mV.

         change lo source-resistor from 0R jumper. to 4.7k to match schematic.
         resoldered 4.7k. resistor again. with heat gun instead of soldering iron. much better. after only couple minutes.
          but no difference.
         +10V  19mV. 19mV.
         0V.   38mV.  38mV.
         -10V. 58mV.

         tie-off any unused azmux inputs (dci-lo, 4w-lo) to gnd. all azmux inputs are now defined.

         +10V  20mV.
         0V.   39mV.
         -10V  57mV.

         it is impressively consistent and stubbon.
         although it is still open to attack the magnitude of the charge, which will also trim the difference.
          --------
          - change goto in state machine.
          +10  19mV.
          0    37mV.
         -10V   57mV>
          ------

          As a test increasing max4053 supply rail, from 4V to 4.7V.  has a strong effect on charge injection

          +10   31mV.  31mV.
          0     51mV.  51mV.
          -10V  71mV.  74mV

          The datasheet is characterized down to 3V single-supply. and can operate as low as 2.7V.
          If I can find some zeners, I'll try reducing the supply rail.
          -------
          With max4053 supply at 2.70V.

          1nplc.
          +10V.      -12mV.  -12mV.            note negative .
          0V        7.5mV.  7mV.
          -10V.      26mV   27mV.

          leakage is still controlled.

          +10V   1000nplc/off -0.4mV
          -10V  1000nplc / off 3.8mV.

          But it is still a 26 - -12 = 39mV. difference. So the charge is the same, it is just centred differently.

          //////////////////////
          // oct 10.
          // test 15.

          baseline max4053, 2.7V supply. 500us precharge. 1nplc
          +10V        -12mV
          0V          7.0mV.
          -10V        26mV.


          temproary test for curiosity - using exaggerated 5ms precharge duration to observe effect.
          precharge duration has strong influence, which suggests something apart from the floating 4053 charge-injection, as Kleinstein notes.
          +10V.       -0.5mV  -0.6mV
          0V.         6.1mV   5.8mV.
          -10V        13.8mV  13.2mV.


          revert to 500us precharge.
          +10V      -12mV.
          etc.

          ------------------
          lift azmux out pin - so not

          leakage 1000nplc/off
          +10V.  -0.2mV.
          0V.     1.7mV. 1.0mV. 0.1mV.  ??
          -10V    2.2mV. 2.1mV.

            ---------------------
          1nplc
          +10V   2.6mV. 2.6mV      Wow. GOOD.
          0V.    5.0mV. 5.0mV.
          -10V.  8.5mV. 8.4mV.

          ---------
          repeat.
          +10V  2.2mV.
          0     4.7mV.
          -10.  8.9mV.   9.2mV


*/

      }

      else if( strcmp( cmd , "") == 0) {

      }

      else {

            printf("unknown of bad argument '%s'\n", cmd );

      }



      // reset buffer
      cStringClear( &app->command);

      // issue new command prompt
      printf("> ");
    }
  }
}



static void loop(app_t *app)
{
  // TODO move to the app var structure ?.
  static uint32_t soft_100ms = 0;
  static uint32_t soft_500ms = 0;
  static uint32_t soft_1s = 0;


/*
 - EXTR. having separate 4094, from fpga controlled outputs. is good.
 because we don't have to pulse the relays.

  - when need to detect coming out of loss of comms - and redo the 4094.
*/

  /*
    do start up. eg. check rails
    initial state - do once.

  */
  printf("writing initial 4094 state\n");
  do_4094_transition( app->spi, &mode_initial,  &app->system_millis );


  // TODO should test that the 4094 state write  succeeded before turning on 4094 OE.
  printf("turning on 4094 OE\n");
  spi_ice40_reg_write32( app->spi, REG_4094, GLB_4094_OE);




  // when mcu restarts, but fpga is not restarted, put fpga in default mode.
  mux_ice40(app->spi);
  spi_ice40_reg_write32(app->spi, REG_MODE, 0 );





  printf("enter main loop\n");
  printf("> ");

  while(true) {


    /*
      // EXTR. - could actually call update at any time, in a yield()...
      // so long as we wrap calls with a mechanism to avoid stack reentrancy
      // led_update(); in systick.
      // but better just to flush() cocnsole queues.conin/out
      // ----
      // no better for a callee to yield back to update(), while setting up a dispatch callback to get control.
    */

    update_console_cmd(app);



    // 100s soft timer
    if( (app->system_millis - soft_100ms) > 100) {
      soft_100ms += 100;
      update_soft_100ms(app);
    }


    // 500ms soft timer
    if( (app->system_millis - soft_500ms) > 500) {
      soft_500ms += 500;
      update_soft_500ms(app);
    }

    // 1000ms soft
    if( (app->system_millis - soft_1s) > 1000 ) {
      soft_1s += 1000;
      update_soft_1s(app);
    }


    /*
      // simpler approach to threads, cooperative threading/co-routines.
      // we've done everythinig needed.  now check if there is a yielded function, and then pass control back out to it.

      if(appp->yielded_function) {

        app->yielded_function( app->spi );
        app->yielded_function( app->spi,  app->yielded_ctx );
      }

      where the yielded function. is a state machine. that does the yield in a long running sequence,
      and which handle re-entry by using state, and a switch/case statement.

    */

  }
}









/////////////////////////
/*
  TODO.
  Maybe move raw buffers into app structure?


  Why not put on the stack? in app_t ?
*/

static char buf_console_in[1000];
static char buf_console_out[1000];

// static char buf_cmds[1000];

static char buf_command[1000];




// move to main ?
// no reason to be static.
// no... because we collect/assemble dependencies. ok in main()
static app_t app;



#if 0

static void spi_ice40_wait_for_ice40( uint32_t spi)
{
  // we don't need this anymore.
  // TODO better ame doto

  mux_ice40(spi);

  printf("wait for ice40\n");
  uint32_t ret = 0;
  // uint8_t magic = 0b0101; // ok. not ok now.  ok. when reset the fpga.
  uint8_t magic = 0b1010;   // this is returning the wrong value....
  do {
    // printf(".");

    spi_ice40_reg_write32( spi, REG_LED, magic);
    msleep( 50);
    ret = spi_ice40_reg_read32( spi, REG_LED);

    char buf[ 100] ;
    printf("wait for ice40 v %s\n",  format_bits(buf, 32, ret ));

    msleep( 50);
  }
  while( ret != magic );
  printf("\n");

}

#endif






int main(void)
{
  // high speed internal!!!
  // TODO. not using.

	rcc_clock_setup_pll(&rcc_hse_8mhz_3v3[RCC_CLOCK_3V3_84MHZ] );  // stm32f411  upto 100MHz.


  // clocks
  rcc_periph_clock_enable(RCC_SYSCFG); // maybe required for external interupts?

  // LED
  // rcc_periph_clock_enable(RCC_GPIOE);
  rcc_periph_clock_enable(RCC_GPIOA);

  // USART
  rcc_periph_clock_enable(RCC_GPIOB); // F410 / f411
  rcc_periph_clock_enable(RCC_USART1);


  // spi / ice40
  rcc_periph_clock_enable(RCC_SPI1);

  /*
    Do led first, even if need update() and systick loop() to blink it.
  */

#define LED_PORT  GPIOA
#define LED_OUT   GPIO9

  gpio_mode_setup(LED_PORT, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, LED_OUT);

  // setup external state for critical error led blink
  // because assert() cannot pass a context
  critical_error_led_setup(LED_PORT, LED_OUT);


  // this is the mcu clock.  not the adc clock. or the fpga clock.
  // systick_setup(16000);

  // extern void systick_setup(uint32_t tick_divider, void (*pfunc)(void *),  void *ctx);
  systick_setup(84000,  (void (*)(void *)) sys_tick_interupt, &app);  // 84MHz.


  //////////////////////
  // main app setup

  memset(&app, 0, sizeof(app_t));


  // uart/console
  cBufInit(&app.console_in,  buf_console_in, sizeof(buf_console_in));
  cBufInit(&app.console_out, buf_console_out, sizeof(buf_console_out));

  cbuf_init_stdout_streams(  &app.console_out );
  cbuf_init_stdin_streams( &app.console_in );


  cStringInit(&app.command, buf_command, buf_command + sizeof( buf_command));

  //////////////
  // initialize usart before start all the app constructors, so that can print.
  // uart
  // usart1_setup_gpio_portA();
  usart1_setup_gpio_portB();

  usart1_set_buffers(&app.console_in, &app.console_out);



  printf("\n\n\n\n--------\n");
  printf("addr main() %p\n", main );

  printf("\n--------");
  printf("\nstarting\n");


  assert( sizeof(bool) == 1);
  assert( sizeof(float) == 4);
  assert( sizeof(double ) == 8);


  // sep 16, 2023.
  // set ER-CRESET-  hi.  not to reset.   This needs to have an external  pullup fitted.
#define ER_CRESET_PORT  GPIOA
#define ER_CRESET_PIN   GPIO1

  gpio_mode_setup(ER_CRESET_PORT, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, ER_CRESET_PIN);
  gpio_set(ER_CRESET_PORT, ER_CRESET_PIN);




  ////////////////
  spi1_port_cs1_setup();
#if 0
  spi1_special_gpio_setup();
#endif
  // adc interupt...
  spi1_port_interupt_gpio_setup( (void (*) (void *))spi1_interupt, &app);


  ////////////////////

  // TODO . this is very specific. not sure this should be here.
  // should have separate app_t setup.

  app.spi = SPI1 ;


  app.led_port = LED_PORT;
  app.led_out  = LED_OUT;




  // app.print_adc_values = true;
  // app.output = false;



#if 0
  // do fpga reset
  spi1_port_cs1_cs2_gpio_setup();

  msleep(1000);

  //while(1)
  {
    // set cs lo, to pull creset lo
    printf("assert cs\n");
    spi1_port_cs1_enable();
    spi1_port_cs2_enable();
    msleep(100);
    printf("deassert cs\n");
    spi1_port_cs1_disable();
    spi1_port_cs2_disable();
    msleep(100);
  }

#endif



#if 0

  // we don't need to wait anymore... we have heartbeat
  // mux spi to ice40
  mux_ice40(app.spi);
  spi_ice40_wait_for_ice40( app.spi );


  /*
  TODO
    this is not quite right.
    we must write /reset 4094 state before turning on OE.
    else 4094 flip/flops may come up in any state,

    should also verify that value was correct. by writing.  and without asserting strobe.
    relays latch could be caught in on state.
  */

  printf("turning on 4094 OE\n");
  // output enable 4094
  spi_ice40_reg_write32( app.spi, REG_4094,  GLB_4094_OE );

#endif


  printf("sizeof X    %u\n", sizeof(X));
  printf("sizeof Mode %u\n", sizeof(Mode ));
  printf("sizeof F    %u\n", sizeof(F));

  assert(sizeof(F) == 4);

  assert( (1<<3|(6-1)) == 0b1101 );

  init_modes();

  // go to main loop
  loop(&app);

	for (;;);
	return 0;
}


/*
  probably from printf
  _close_r  _fstat_r _getpid_r _kill_r _isatty_r

  see.
    -specs=nano.specs -specs=nosys.specs

    arm-none-eabi-gcc: fatal error: /nix/store/3ydyllv3y22qpxcgsf9miwq4dkjwjcj2-gcc-arm-embedded-12.2.rel1/bin/../lib/gcc/arm-none-eabi/12.2.1/../../../../arm-none-eabi/lib/nosys.specs: attempt to rename spec 'link_gcc_c_sequence' to already defined spec 'nosys_link_gcc_c_sequence'


    --specs=rdimon.specs

  https://stackoverflow.com/questions/73742774/gcc-arm-none-eabi-11-3-is-not-implemented-and-will-always-fail
*/

#if 1

void _close_r( void );
void _fstat_r( void );
void _getpid_r( void);
void _kill_r( void);
void _isatty_r( void);
void _lseek_r( void);
void _read_r( void);
void _write_r( void);


void _close_r( void )
{
  assert(0);
}

void _fstat_r( void )
{
  assert(0);
}

void _getpid_r( void)
{
  assert(0);
}

void _kill_r( void)
{
  assert(0);
}

void _isatty_r( void)
{
  assert(0);
}

void _lseek_r( void)
{
  assert(0);
}

void _read_r( void)
{
  assert(0);
}

void _write_r( void)
{
  assert(0);
}

#endif







#if 0
  {
    char buf[100];

      // printf("val %s\n",  format_bits(buf, 8,  write_val8( 0xff, 2, 1, 0xff ) ));

    // printf("val %s\n",  format_bits(buf, 8,  write_val8( 0x0, 1, 5, 0xff ) ));

    uint8_t v[ 3 ];
    memset(v, 0xff, sizeof(v));
    state_write ( v, sizeof(v), 16, 3, 0x0 );

    printf("v[0] %s\n",  format_bits(buf, 8, v[0 ]));
    printf("v[1] %s\n",  format_bits(buf, 8, v[1 ]));
    printf("v[2] %s\n",  format_bits(buf, 8, v[2 ]));
  }
#endif

  //////////////////////////////





/*
// chage name spi_ice40_stress test.

static void spi_ice40_stress_test_spi( uint32_t spi)
{
  // TODO better name. move code to separate test folder.
  // prefix with test ? perhaps

  printf("stress test spi comms\n");
  mux_ice40(spi);
  uint8_t magic = 0;

  uint8_t reg =  REG_LED;

  while(1) {
    spi_ice40_reg_write32(spi, reg, magic++);
    // spi_ice40_reg_write( spi, reg, magic ++ );
    uint32_t ret = spi_ice40_reg_read32( spi, reg);
    char buf[ 100] ;
    printf("r %u  v %lu  %s\n",  reg, ret,  format_bits(buf, 32, ret ));
    msleep( 150);
  }
}


static void spi_ice40_just_read_reg ( uint32_t spi)
{
  mux_ice40(spi);

  while(1) {
    uint32_t v = spi_ice40_reg_read32(spi, REG_LED);
    // uint32_t v = spi_ice40_reg_read32(spi, 0b00111111 );
    char buf[32+1];
    printf("value of led %lu %s\n", v, format_bits(buf, 32, v ));
    msleep( 500 );
  }
}


*/





#if 0


static uint32_t write_bits8( uint8_t v, uint8_t set, uint8_t clear )
{
  // *v = ~(  ~(*v | set ) | clear);  // clear has priority over set

  return ~( ~v | clear)  | set;    // set has priority
}


static uint8_t create_mask8( uint8_t pos, uint8_t width)
{
  uint8_t val = (1 << (pos + width )) - 1;   // set all bits below pos + width.
  uint8_t val2 = (1 << pos) - 1;            // set all bits below pos

  return val & ~val2;
}




static uint8_t write_val8 ( uint8_t v, uint8_t pos, uint8_t width, uint8_t value)
{
  // write value into v - at pos with width
  assert(pos < 8);
  assert(width < 8);

  uint8_t mask = create_mask8( pos, width);

  return write_bits8( v, (value << pos) & mask, mask);
}


// we manipulate the state. then write it.
// there is writing state to state var. then there is spi writing it.

static void state_write ( uint8_t *state, size_t n, unsigned pos, uint8_t width, uint8_t value)
{
  // determine index to use into byte array, and set value
  unsigned idx = pos >> 3 ;   // div 8
  assert( idx < n );
  uint8_t *v = & state[ idx ];

  *v = write_val8 ( *v, pos % 8, width, value);
}


#endif




#if 0

static void assert_app(app_t *app, const char *file, int line, const char *func, const char *expr)
{
  /*
    note the usart tx interupt will continue to flush output buffer,
    even after jump to critical_error_blink()
  */
  printf("\nassert_app failed %s: %d: %s: '%s'\n", file, line, func, expr);

  state_change(app, STATE_HALT );

#if 1
  // we have to do a critical error here... else caller code will just progress,
  // if we were in a state transition, then it will continue to just progress...
  critical_error_blink();
#endif
}

#endif




/*
      else if( strcmp(cmd, "relay on") == 0) {  // output on

        relay_set( app->spi, app->state_4094, sizeof(app->state_4094), REG_K406,  1 );
      }
      else if( strcmp(cmd, "relay off") == 0) {  // output off

        relay_set( app->spi, app->state_4094, sizeof(app->state_4094), REG_K406,  0 );
      }
*/


#if 0
      else if(strcmp(cmd, "mon") == 0) {

        // this will powerdown rails
        mux_ice40(app->spi);
        uint8_t val = ice40_reg_read( app->spi, REG_MON_RAILS );
        char buf[10];
        printf("reg_mon_rails read bits %s\n", format_bits(buf, 4, val) );

      }


      else if(strcmp(cmd, "reset") == 0) {

        // this will powerdown rails
        printf("reset ice40\n");
        mux_ice40(app->spi);
        ice40_reg_set(app->spi, CORE_SOFT_RST, 0 );
      }


      else if(strcmp(cmd, "powerup") == 0) {

        // ok without dac being initialized?
        // powerup 5 and +-15V rails and set analog switches

        printf("powerup ice40\n");
        mux_ice40(app->spi);

        // check rails monitor.
        uint8_t val = ice40_reg_read( app->spi, REG_MON_RAILS );
        // char buf[10];
        // printf("reg_mon_rails read bits %s\n", format_bits(buf, 4, val) );

        if(val == 1) {
          ice40_reg_set(app->spi, 6 , 0 );
        }  else {
          printf("problem with rails monitor\n");
        }
      }


      else if( strcmp(cmd, "on") == 0) {  // output on

        ice40_reg_set(app->spi, REG_RELAY_OUT, REG_RELAY_OUT_COM_HC_CTL);
        ice40_reg_set(app->spi, REG_LED, LED1);
      }
      else if( strcmp(cmd, "off") == 0) {  // output off

        ice40_reg_clear(app->spi, REG_RELAY_OUT, REG_RELAY_OUT_COM_HC_CTL);
        ice40_reg_clear(app->spi, REG_LED, LED1);
      }

      else if( strcmp(cmd, "guard on") == 0) {  // output on

        ice40_reg_set(app->spi, REG_RELAY_OUT, REG_RELAY_GUARD_CTL);
      }
      else if( strcmp(cmd, "guard off") == 0) {  // output off

        ice40_reg_clear(app->spi, REG_RELAY_OUT, REG_RELAY_GUARD_CTL);
      }


      else if( sscanf(cmd, "sense ext %lu", &u0 ) == 1) {

        if(u0) {
          ice40_reg_clear(app->spi, REG_RELAY_OUT, REG_RELAY_SENSE_INT_CTL);    // FIXME. write not set // perhaps push to own register to make exclusive.
          ice40_reg_set(app->spi, REG_RELAY_OUT, REG_RELAY_SENSE_EXT_CTL);
        } else {
          ice40_reg_set(app->spi, REG_RELAY_OUT, REG_RELAY_SENSE_INT_CTL);
          ice40_reg_clear(app->spi, REG_RELAY_OUT, REG_RELAY_SENSE_EXT_CTL);
        }

      }






#endif



#if 0

static void relay_set( unsigned spi, uint8_t *state_4094, size_t n, unsigned reg_relay,   unsigned relay_state )
{


  // mux spi to 4094. change mcu spi params, and set spi device to 4094
  mux_4094( spi);


  // set two relay bits, according to dir
  state_write ( state_4094, n, reg_relay, 2, relay_state ? 0b01 : 0b10 );

  // output/print - TODO change name state_format_stdout, or something.
  state_format ( state_4094, n);

  // and write device
  spi_4094_reg_write_n(spi, state_4094, n );

  // sleep 10ms
  msleep(10);

  // clear the two relay bits
  state_write ( state_4094, n, reg_relay, 2, 0b00 );   // clear

  // and write device
  spi_4094_reg_write_n(spi, state_4094, n );


  // turn off spi muxing
  mux_no_device(spi);

}
#endif




#if 0
// position in state.
/*
#define REG_K406    2
*/

#define REG_K406    2   // pins 2 and 3.  works well.


// #define REG_K401    (17  ) // third 4094.


// #define REG_K405_4094    2 (16 + 6  ) // third 4094.     L0.

#define REG_K405    (16 + 6  ) // third 4094.     L0.

#define REG_K405_L1    (2 * 8 + 6)
#define REG_K405_L2    (2 * 8 + 7)



#define REG_U404      8   // 2nd 4094.  4 bits.  works. well.

#endif

// we have to OR together an array....
// mode or state.

// so it would be easier to create values in functions.  where we can use an OR function - and record the bit position jj.
// Need to be able to indicate if the relay
// Does C99 have static initializers?
// static void my_or( uint8_t *a, uint8_t *b
// HANG on. it is still tricky. even with functions.... No with functions.
// mode_latch_relay_mask








#if 0
  // 4094 OE should have been configured already,
  uint32_t v = spi_ice40_reg_read32( app->spi, REG_4094);
  UNUSED(v);

  // assert(v == 1);
  // char buf[32+1];
  // printf("4094 state %lu %s\n", v, format_bits(buf, 32, v ));


  //////////////////////////////////
  // EXTR. THE specific led should be injected into APP state on construction.
  // not accessed as a global macro. makes it hard to test.
  //////////////////////////////////
#endif


#if 0
  // spi_ice40_reg_write32(app->spi, REG_LED, count );   // we don't have the set and clear bits...

/*
  uint32_t v = spi_ice40_reg_read32(app->spi, REG_LED);
  char buf[32+1];
  printf("value of led %lu %s\n", v, format_bits(buf, 32, v ));
*/


  // set mcu led state
  led_set( led_state );

#endif


  // toggles relay ok.
  // relay_set( app->spi, app->state_4094, sizeof(app->state_4094), REG_K406  , led_state );


  // toggles relay K405 - works well..
  // relay_set( app->spi, app->state_4094, sizeof(app->state_4094), REG_K405, led_state );
//


/*
  /// toggle ctrl pins of U404
  mux_4094( app->spi);
  // set two relay bits, according to dir.  is 4 is the number of bits.
  state_write ( app->state_4094, sizeof(app->state_4094), REG_U404 , 4, led_state ? 0b1000 : 0b0000 );
  // output/print - TODO change name state_format_stdout, or something.
  state_format ( app->state_4094, sizeof(app->state_4094));
  // and write device
  spi_4094_reg_write_n(app->spi, app->state_4094, sizeof(app->state_4094) );
*/

  /*
      - every state change has to be encoded - as two states.  eg. transition state, and resulting state.
          and has to be handled as a sequency.
          this is a little complicated.

      - But can still be encoded/recorded as a fixed string.

      - Eg. for DCV.
      - - have intermittent relay state.

      - perhaps even three states. eg. for dcv on. the sequencing of turn off b2b fets, then manipulate the relay, then turn on b2b fets.
      - but we may not need this.
      ------------------

      - EXTR.   AND we should encode the MUX registers to use. in the 4094 string. to make it single authoritative source.
                even if use a different way to write these to the fpga.
      -------
      - possible to also use maskable writes.

      - should be a single function that handles the 20ms timing. regardless of which state we want to use
      - we don't need to encode much.

      - we don't necessarily have to record the current state.  unless we
      - transition (  STATE_DCV  )    - and this would have the three transitions .
      - we don't keep individual registers arounds.   only some defines.
      -------------------

      Also the ability to copy.  with memcpy.   and then modify.

      - OR use single byte-array.    and store the two states end-to-end.   eg. if have 5 bytes for state. then use 10 bytes. for 2 transition states.

      - Also can store the mask.

      - latching relays have to use the transition data.   but analog switches need to remain.   Also makes sense to switch relays - before doing analog switches.

      - muxing has the 3x4 muxes = 12 bits. repeated for the signal, and zero.   so these would both be embedded in the string.  perhaps with a number representing sequence.
      --------
      EXTR.
      - eg. don't have separate registers for analog switches etc.  instead entire unit state encoded as single register.

      EXTR.
      - it may be possible to encode everything as single transition - if we manage how to .   eg. a relay set mask.  at least for   .

      - eg. relay clear mask.   this would be fixed/static   that would be or'ed,

      - initialize in a main functino    MODE_DCV_AZ
      - the writing of these bit valuees can be done in a function.
      - and post state mask - to clear relays.
      - transition(   state0, state1, state2 ).

      ------
      - simplest is to have the transition function manage - the latch clear state -  just with a fixed.

  */






/*
  if(led_state)
    spi_4094_reg_write(app->spi , 0b11111111 );
  else
    spi_4094_reg_write(app->spi , 0 );
*/

  // we can factor this fairly easily into a single function -- with the delay.
  // actualyl variable name probably not great. because will chain them.
/*
  if(led_state) {
    // hmmmmm we need a local
    app->u304 |=  U304_K302_L1_CTL;
    spi_4094_reg_write(app->spi, app->u304 );
    msleep(10);
    app->u304 &= ~ U304_K302_L1_CTL ;
    spi_4094_reg_write(app->spi, app->u304 );
  }
  else {

    app->u304 |= U304_K302_L2_CTL ;
    spi_4094_reg_write(app->spi, app->u304 );
    msleep(10);
    app->u304 &= ~ U304_K302_L2_CTL ;
    spi_4094_reg_write(app->spi, app->u304 );

  }
  */

  /*
  // the issue is the verilog 4094 vector - will continue to follow cs2 - when we change from gpio back to spi alternate function.
      eg. and it will go high - as the ordinary parking state of cs.

      options - invert the connection to the output.
      use fpga register to control cs of the 4094. so do write. and then toggle the register.

      eg. an independent approach.

      maybe the whole thing would be easier - if just used ice40 regsisters for CS.
      issue is. the sequence. set to mux ice40 and assert. then set to peripheral peripheral, write. set to muxice 40 and to cs deassert.
      -----
      what if changed. so on mcu side.

      WE don't alternate the cs pins.   instead we just assert an extra pin/ to disambiguate target  using gpio .

      eg. SO ALL WRITES USE ordinary CS1.
      Whether it's too the fpga or the fpga peripheral.  is whether the additional gpio is hi or lo.
      there are no synchronization issues with mcu spi.

      and it avoids constantly having to reconfigure the spi-ports
      --------------

      Doesn't fix. the issue for 4094.  but simplifying things - would make inverting the strobe simpler. / or assign
      --------------

      lets try the invert trick as is. to see if can get work. and test relay.
      -------------
      EXTR - there may be synchronization issues. eg. CS not deasserted. before gpio is swapped.
  */

#if 0
  mux_4094(app->spi);
  spi_4094_reg_write(app->spi , 0b01010101 );

  // msleep(1);    // if we put a sleep here we get a diffferent read value?????

  /*
  // perhaps the action of reading is also setting the value????
      becasue it's an 8 bit register and the bit is in the clear bits....
    -----
    Or it's an overflow....   on read.

    SOLUTION - might be not to remove all the bit setting - but have a parallel register that is direct write.
    -------

    - try writing something that fits in 3 bits. and see if that gets overwritten.
    - maybe change the 16bit width to 24 bit.
  */
  mux_ice40(app->spi);

  val = ice40_reg_read( app->spi, REG_SPI_MUX);
  printf("reg_spi_mux read bits %u %s\n", val, format_bits(buf, 8, val) );

  msleep(1);

  val = ice40_reg_read( app->spi, REG_SPI_MUX);
  printf("reg_spi_mux read bits %u %s\n", val, format_bits(buf, 8, val) );

#endif


