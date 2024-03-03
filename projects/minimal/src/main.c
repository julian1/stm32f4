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

  reset halt ; flash write_image erase unlock ./projects/minimal/main.elf; sleep 1500; reset run


// vim :colorscheme default. loooks good.

// cjmcu  stm32f407.
// issue. is that board/stlink doesn't appear to reset cleanly. needs sleep.

// - can plug a thermocouple straight into the sense +p, +n, and
// then use the slow (digital) integrator/pid loop - as a temperature controler..




*/


/*
  EXTR. we shouldn't need to include low-level peripheral headers stuff here.
  instead peripheral should be configured externally.
  ---

  we could put low-level peripheral stuff in peripheral directory.

*/
#include <libopencm3/stm32/rcc.h>   // for clock initialization
#include <libopencm3/cm3/scb.h>  // reset()
#include <libopencm3/stm32/spi.h>   // SPI1


#include <stdio.h>    // printf, scanf
#include <string.h>   // strcmp, memset
// #include <strings.h>   // strcasecmp()
#include <ctype.h>    // isspace
#include <assert.h>
#include <malloc.h> // malloc_stats()
#include <stdlib.h> // strtolu



// library code
#include <lib2/usart.h>
#include <lib2/streams.h>
#include <lib2/util.h>   // msleep()
#include <lib2/cbuffer.h>
#include <lib2/cstring.h>
#include <lib2/format.h>   // trim_whitespace()



#include <peripheral/led.h>
#include <peripheral/spi-port.h>
#include <peripheral/ice40-extra.h>


#include <spi-ice40.h>
#include <spi-4094.h>
#include <spi-ice40-bitstream.h>
#include <spi-dac8811.h>
#include <spi-ad5446.h>

#include <lib2/format.h>   // format_bits()

#include <ice40-reg.h>

#include <mode.h>
#include <app.h>



// fix me
int flash_lzo_test(void);



















static void app_update_soft_500ms_configured(app_t *app)
{
  /* we may want to devote a fpga led - to fpga comms
    just flip the state - for any spi sequence.
      - just to catch any spurious transfers.
    -----
    should see if we can catch something.... with simple fpga code - and without a timer.  eg. just an active slave select on cs1 or cs2.
  */


  return;


  /*
      consider rename test_led_blink  and disable by default to aoid
      spurioius spi transmissions during acquisition
      potentially move into /src/test
  */
  if(app->led_blink) { // rename test_led_blink()
    // we need to not blink the led, if we want to use repl to write directly.

    /* EXTR - avoid electrical/comms activity of a heart-beat/led blink, during sample acquisition.  only use as test.
    */
    spi_mux_ice40( app->spi );


    // uint32_t magic = app->led_state ? 0b01010101 : 0b10101010 ;
    /*
    keep
    15   always@(posedge clk) begin
    16     counter <= counter + 1;
    17     outcnt <= counter >> LOG2DELAY;
    18   end
    19
    20   assign { LED1, LED2} = outcnt ^ (outcnt >> 1);
    */

/*
    static uint32_t counter = 0;
    ++counter;
    uint32_t magic = counter  ^ (counter >> 1 );
*/


    static uint32_t magic = 0;
    ++magic;


    // blink led... want option. so can write reg_direct
    // note - led will only, actually light if fpga in default mode. 0.
    spi_ice40_reg_write32( app->spi, REG_DIRECT, magic << 1 );

    // check the magic numger
    uint32_t ret = spi_ice40_reg_read32( app->spi, REG_DIRECT);
    ret >>= 1;
    if(ret != magic ) {
      // comms no good
      char buf[ 100] ;
      printf("comms failed, returned reg value %s\n",  str_format_bits(buf, 32, ret ));
    } else {
      // printf("comms ok\n");
    }
  }


  if(app->test_relay_flip) {

    static bool flip = 0;
    flip = ! flip;


#if 0
      Mode mode;
      memset(&mode, 0, sizeof(mode));

      // mode.first.K701 =  flip ? 0b01 : 0b10;
      mode.first.K404 =  flip ? 0b01 : 0b10;
      mode.first.K407 =  flip ? 0b01 : 0b10;

      mode.second.U1003 = flip ? 0b1111 : 0b000;

      spi_mode_transition_state( app->spi, &mode, &app->system_millis);
#endif

#if 0
      // click the relays, and analog switch.
      _4094_state_t mode;
      memset(&mode, 0, sizeof(mode));

      mode.K701 =  flip ? 0b01 : 0b10;
      mode.K404 =  flip ? 0b01 : 0b10;
      mode.K407 =  flip ? 0b01 : 0b10;
      mode.U1003 = flip ? 0b1111 : 0b000;

      // make sure we are muxing spi,
      spi_mux_4094( app->spi );

      // can probe 4094 signals - by connecting scope to 4094 extension header pins.
      // write single byte - should be enough to flip a relay.
      // JA spi_4094_reg_write_n(app->spi, (uint8_t *)& magic , 1 );
      spi_4094_reg_write_n(app->spi, (uint8_t *)& mode, sizeof(mode) );

      // sleep 10ms.
      msleep(10, &app->system_millis);

      // now clear the relays
      mode.K701 = 0b00;
      mode.K404 = 0b00;
      mode.K407 = 0b00;
      spi_4094_reg_write_n(app->spi, (uint8_t *)& mode, sizeof(mode) );


      /* EXTR. IMPORTANT. must call spi_mux_ice40 again
            - to prevent spi emission on 4094 spi clk,data lines.
            - when reading the adc counts
      */
      spi_mux_ice40(app->spi);
#endif

    }


}

static void app_update_soft_500ms(app_t *app)
{
  assert(app);

  /*
    function should reconstruct to localize scope of app. and then dispatch to other functions.
  */


  /*
    blink mcu led
  */
  app->led_state = ! app->led_state;

  if(app->led_state)
    led_on();
  else
    led_off();

  /*
      - if fpga cdone() is lo, then try to configure fpga.
  */
  if(! ice40_port_extra_cdone_get()) {

    spi_ice40_bitstream_send(app->spi, & app->system_millis );

    assert( ! spi_ice40_reg_read32( app->spi, REG_4094 ));

    // we should write the initial mode.
    // before we assert 40

    // HERE


    printf("spi_mode_transition_state() for muxes\n");
    spi_mode_transition_state( app->spi, app->mode_initial, &app->system_millis);


    /* OK. this is tricky.
      OE must be enabled to pulse the relays. to put them in initial state.
    */

    printf("asserting 4094 OE\n");

    // assert 4094 OE
    // should check supply rails etc. first.
    spi_ice40_reg_write32( app->spi, REG_4094, 1 );

    // ensure 4094 OE asserted
    assert( spi_ice40_reg_read32( app->spi, REG_4094 ));

    // now  call transition state again.
    printf("spi_mode_transition_state() for relays\n");
    spi_mode_transition_state( app->spi, app->mode_initial, &app->system_millis);
  }


  if(ice40_port_extra_cdone_get()) {

    app_update_soft_500ms_configured( app);


  }


}




static unsigned str_decode_int( const char *s, uint32_t *val  )
{
  // decode int literal
  // set/reset  for relay.


  // reset == default position in the schematic.

  if (strcmp(s, "on") == 0
    || strcmp(s, "set") == 0
    || strcmp(s, "true") == 0)
    *val = 1;

  else if(strcmp(s, "off") == 0
    || strcmp(s, "reset") == 0
    || strcmp(s, "false") == 0)
    *val = 0;




  // 1 of 8 mux values.
  else if(strcmp(s, "s8") == 0 )
    *val = S8;
  else if(strcmp(s, "s7") == 0 )
    *val = S7;
  else if(strcmp(s, "s6") == 0 )
    *val = S6;
  else if(strcmp(s, "s5") == 0 )
    *val = S5;
  else if(strcmp(s, "s4") == 0 )
    *val = S4;
  else if(strcmp(s, "s3") == 0 )
    *val = S3;
  else if(strcmp(s, "s2") == 0 )
    *val = S2;
  else if(strcmp(s, "s1") == 0 )
    *val = S1;

/*
  // 2 of 4 mux values

*/


  // we could factor all this handling.
  // read_int.
  else if( s[0] == '0' && s[1] == 'x' && sscanf(s, "%lx", val) == 1) {
    // printf("got hex\n" );
  }
  else if( s[0] == '0' && s[1] == 'o' && sscanf(s + 2, "%lo", val) == 1) {
    // for octal, sscanf doesn't like/accept a prefix
    // printf("got octal\n" );
  }
  else if( s[0] == '0' && s[1] == 'b') {
    // binary is very useful for muxes
    *val = strtoul(s + 2, NULL, 2);
    // char buf[100];
    // printf("got binary %s\n", format_bits(buf, 32, val ) );
  }
  else if( isdigit( (unsigned char) s[0] ) && sscanf(s, "%lu", val) == 1) {
    // printf("got decimal\n" );
  }

  else {
    printf("bad val arg\n" );
    return 0;   // fail
  }

  // OK.
  return 1 ;
}






/*
  keep general repl stuff (related to flashing, reset etc) here,
  put app specific/ tests in a separatefile.

*/


static void app_repl_statement(app_t *app,  const char *cmd)
{
  /*
    eg. statement without semi-colon.

  */

  UNUSED(app);

  // useful for debug
  // printf("cmd '%s'  %u\n", cmd, strlen(cmd) );



  char s0[100 + 1 ];
  char s1[100 + 1 ];


  // uint32_t u0 , u1;
  uint32_t u0;


  ////////////////////


  if(strcmp(cmd, "") == 0) {
    // ignore
    printf("empty\n" );
  }



  else if(strcmp(cmd, "help") == 0) {

    printf("help <command>\n" );
  }

/*
  - some of these are stateful. and will be done out of sequence.
  - mode changes acccumulate until there is a new-line.

  - way to handle it would be better.
  - we could introduce a different separator  eg. ';' which would aggregate

  - or else -  if have stateful command - like sleep. or reset mcu.  then just return a flag.
  - or test whether the mode value changed.

*/

  else if( sscanf(cmd, "sleep %lu", &u0 ) == 1) {
    // should be done on separate line?

    msleep(u0 , &app->system_millis);
  }

  else if(strcmp(cmd, "reset mcu") == 0) {
    printf("perform mcu reset\n" );
    // reset stm32f4
    // scb_reset_core()
    scb_reset_system();
  }


  else if(strcmp(cmd, "reset fpga") == 0) {

    ice40_port_extra_creset_enable();
    // wait
    msleep(1, &app->system_millis);
    ice40_port_extra_creset_disable();
  }

  else if(strcmp(cmd, "reset") == 0) {

    // reset the mode.
    *app->mode_current = *app->mode_initial;
  }



  // need to add reset fpga.  using external creset pin.


  else if(strcmp(cmd, "assert 0") == 0) {
    // test assert(0);
    assert(0);
  }
  else if(strcmp(cmd, "mem?") == 0)
  {
    printf("malloc\n");
    malloc_stats();

    print_stack_pointer();
    // return 1;
  }


  else if(strcmp(cmd, "flash unlock ") == 0) {

  }
  else if(strcmp(cmd, "flash write ") == 0) {
    // wants to be good enough for mcu boot code, mcu code, and fpga code.
    /*

        if use base64 transfer - then a terminal sequence of whitespace - means we get the size of the bitstream.
        without having to encode a header with the size.
        and can calculate the crc.

        the size could also be stored separately. or else assumed.
    */

  }
  else if(strcmp(cmd, "flash crc ") == 0) {
    // need size to compute.

  }






  else if(strcmp(cmd, "flash lzo test") == 0) {
    flash_lzo_test();
    // int flash_raw_test(void);
  }


  else if( sscanf(cmd, "blink %lu", &u0 ) == 1) {
    // turn off fpga blink in mode 0, avoid spi transmission, during acquisition.
    app->led_blink = u0;
  }

  else if( sscanf(cmd, "test relay flip %lu", &u0 ) == 1) {

    app->test_relay_flip = u0;
  }




  // change name fpga bitstrea load/test
  else if(strcmp(cmd, "bitstream test") == 0) {

    spi_ice40_bitstream_send(app->spi, & app->system_millis );
  }

  // don't we have some code - to handle sscan as binary/octal/hex ?




  else if( sscanf(cmd, "direct %100s", s0) == 1
    && str_decode_int( s0, &u0)
  ) {
    /*
      IMPORTANT - to properly sequence, in a set of repl commands,
      Or just use 'set' direct.
      Or allow set direct bits.
    */

    // set the direct register.
    printf("set direct value to, %lu\n", u0 );

    spi_mux_ice40(app->spi);
    spi_ice40_reg_write32(app->spi, REG_DIRECT, u0 );
    // confirm.
    uint32_t ret = spi_ice40_reg_read32(app->spi, REG_DIRECT );
    char buf[ 100 ] ;
    printf("r %u  v %lu  %s\n",  REG_DIRECT, ret,  str_format_bits(buf, 32, ret ));
  }


#if 0
  else if( sscanf(cmd, "direct bit %lu %lu", &u0, &u1 ) == 2) {

    // modify direct_reg and bit by bitnum and val
    /* eg.
          OLD.

        mode direct
        direct 0         - clear all bits.
        direct bit 13 1  - led on
        direct bit 13 0  - led off.
        direct bit 14 1  - mon0 on
        direct bit 22 1  - +ref current source on. pushes integrator output lo.  comparator pos-out (pin 7) hi.
        direct bit 23 1  - -ref current source on. pushes integrator output hi.  comparator pos-out lo
        --
        for slow run-down current. turn on bit 23 1. to push integrator hi.
        then add bit 22 1.  for slow run-down. works, can trigger on scope..about 2ms. can toggle bit 22 off against to go hi again.

        direct bit 25   - reset. via 20k.
        direct bit 26   - latch.  will freeze/latch in the current comparator value.

      - note. run-down current creates integrator oscillation when out-of-range.
    */

    spi_mux_ice40(app->spi);
    uint32_t ret = spi_ice40_reg_read32(app->spi, REG_DIRECT );
    if(u1)
      ret |= 1 << u0 ;
    else
      ret &= ~( 1 << u0 );

    char buf[ 100 ] ;
    printf("r %u  v %lu  %s\n",  REG_DIRECT, ret,  str_format_bits(buf, 32, ret ));
    spi_ice40_reg_write32(app->spi, REG_DIRECT, ret );
  }
#endif


  else if( strcmp( cmd, "direct?") == 0) {

    spi_mux_ice40(app->spi);
    uint32_t ret = spi_ice40_reg_read32(app->spi, REG_DIRECT );
    char buf[ 100];
    printf("r %u  v %lu  %s\n",  REG_DIRECT, ret,  str_format_bits(buf, 32, ret ));
  }
  else if( strcmp( cmd, "status?") == 0) {

    spi_mux_ice40(app->spi);
    uint32_t ret = spi_ice40_reg_read32(app->spi, REG_STATUS);
    char buf[ 100];
    printf("r %u  v %lu  %s\n",  REG_STATUS, ret,  str_format_bits(buf, 32, ret ));
  }


  ////////////////////

  // could probably


  else if( sscanf(cmd, "mode %lu", &u0 ) == 1) {

    // set the fpga mode.
    spi_mux_ice40(app->spi);
    spi_ice40_reg_write32(app->spi, REG_MODE, u0 );

    uint32_t ret = spi_ice40_reg_read32(app->spi, REG_MODE );
    printf("reg_mode return value %lu\n", ret);
  }

  else if( strcmp(cmd, "mode?") == 0) {

    // TODO add some decoding here.
    spi_mux_ice40(app->spi);
    uint32_t ret = spi_ice40_reg_read32(app->spi, REG_MODE );
    printf("reg_mode return value %lu\n", ret);

    // Mode *mode = app->mode_current;
    // printf("app      return value %lu\n", mode->reg_mode );

  }

#if 0
/*
  -- don't really need, just query direct reg for monitor and right shift.
*/
  else if( strcmp( cmd, "monitor?") == 0) {

    // this is no longer corrent. should query the REG_STATUS. which includes the monitor
    // regardless of the mode.
    spi_mux_ice40(app->spi);
    uint32_t ret = spi_ice40_reg_read32(app->spi, REG_DIRECT);
    ret >>= 14;
    char buf[ 100];
    printf("r %u  v %lu  %s\n",  REG_DIRECT, ret, str_format_bits(buf, 8, ret ));
  }
#endif


  ///////////////////////////////////////////////////


#if 0
  // for test only. use the mode transitino function instead.

  else if( sscanf(cmd, "dac %s", s0 ) == 1
    && str_decode_int( s0, &u0)
  ) {
       // spi_mux_dac8811(app->spi);
      spi_mux_ad5446(app->spi );

      // eg. 0=-0V out.   0xffff = -7V out. nice.
      spi_dac8811_write16( app->spi, u0 );

      spi_mux_ice40(app->spi);
    }
#endif

  ///////////////////////


  // strcasecmp() from strings.h.
  // or just force lower case first.


  else if( sscanf(cmd, "set %100s %100s", s0, s1) == 2
    && str_decode_int( s1, &u0)
  ) {

      assert(app->mode_current);
      Mode * mode = app->mode_current;


      printf("set %s0 %lu\n", s0, u0);

      // cannot manage pointer to bitfield. so have to hardcode.

      if(strcmp(s0, "u1003") == 0) {
        mode->second.U1003 = u0;
      }
      else if(strcmp(s0, "u1006") == 0) {
        mode->second.U1006 = u0;
      }
      else if(strcmp(s0, "u1012") == 0) {
        mode->second.U1012 = u0;
      }

      else if( strcmp(s0, "dac") == 0 || strcmp(s0, "u1016") == 0 || strcmp(s0, "u1014") == 0) {
        // let the mode update - determine setting up spi params.
        mode->dac_val = u0;
      }

      /*
          handle latch relay pulse encoding here, rather than at str_decode_int() time.
          valid values are 1 (0b01)  and 2 (0b10). not 1/0.
          reset is default schem contact position.
      */
      else if(strcmp(s0, "k407") == 0) {
        mode->first.K407 = u0 ? LR_SET: LR_RESET ;      // 0 == reset
      }
      else if(strcmp(s0, "k406") == 0) {
        mode->first.K406 = u0 ? LR_SET: LR_RESET;
      }
      else if(strcmp(s0, "k405") == 0) {
        mode->first.K405 = u0 ? LR_SET: LR_RESET;
      }


      // this is nice.
      else if(strcmp(s0, "leds_o") == 0) {
        mode->reg_direct.leds_o = u0;
      }
      else if(strcmp(s0, "monitor_o") == 0) {
        mode->reg_direct.monitor_o = u0;
      }


      // sample acquisition - direct state
      else if(strcmp(s0, "sig_pc1_sw_o") == 0) {
        mode->reg_direct.sig_pc1_sw_o= u0;
      }
      else if(strcmp(s0, "sig_pc2_sw_o") == 0) {
        mode->reg_direct.sig_pc2_sw_o = u0;
      }
      else if(strcmp(s0, "azmux_o") == 0) {
        mode->reg_direct.azmux_o = u0;
      }






      else {

        printf("unknown target %s\n", s0);
      }


  }





  else {

    printf("unknown cmd, or bad argument '%s'\n", cmd );

  }


}




static void app_update_console_cmd(app_t *app)
{

  // move this into a separate function...
  // so can be called from in code
  // or perhaps use the clause



  while( ! cbuf_is_empty(&app->console_in)) {

    // got a character
    int32_t ch = cbuf_pop(&app->console_in);
    assert(ch >= 0);

    // only read as much of console_in


    if (ch == ';' || ch == '\r' )
    {
      // a separator, - process what we command so far.

      char *cmd = cstring_ptr(&app->command);
      cmd = str_trim_whitespace_inplace( cmd );
      printf("\n");
      app_repl_statement(app, cmd);

      // clear the current command buffer,
      // there is still more data to process in console_in
      cstring_clear( &app->command);
    }
    else if( cstring_count(&app->command) < cstring_reserve(&app->command) ) {

      // must accept whitespace here, since used to demarcate args
      // normal character
      cstring_push(&app->command, ch);
      // echo to output. required for minicom.
      putchar( ch);
    } else {
      // overflow

    }


    if(ch == '\r')
    {
      printf("calling spi_mode_transition_state()");

      spi_mode_transition_state( app->spi, app->mode_current, &app->system_millis);

      // issue new command prompt
      printf("\n> ");
    }

  }   // while

}






static void app_loop(app_t *app)
{

  while(true) {

    // handle console
    app_update_console_cmd(app);

    // 500ms soft timer
    if( (app->system_millis - app->soft_500ms) > 500) {
      app->soft_500ms += 500;
      app_update_soft_500ms(app);
    }

  }
}





static void sys_tick_interupt(app_t *app)
{
  // interupt context. don't do anything compliicated here.

  ++ app->system_millis;
}




static void spi1_interupt(app_t *app)
{
  UNUSED(app);
  // now on a positive transition.

/*
  // if flag is still active, then record we missed processing some data.
  if(app->adc_measure_valid == true) {
    app->adc_measure_valid_missed = true;
    // ++app->adc_measure_valid_missed;     // count better? but harder to report.
  }

  // set adc_measure_valid flag so that update() knows to read the adc...
  app->adc_measure_valid = true;
*/
}








/////////////////////////
/*
  TODO.
  Maybe move raw buffers into app structure?


  Why not put on the stack? in app_t ?
*/

static char buf_console_in[1000];
static char buf_console_out[1000];    // changing this and it freezes. indicates. bug

// static char buf_cmds[1000];

static char buf_command[1000];


// move to main ?
// no reason to be static.
// no... because we collect/assemble dependencies. ok in main()
static app_t app;






// static Mode mode_current;



static const Mode mode_initial =  {



  .first .K407  = LR_SET,     // disconnect dcv-source
  .first .K406  = LR_SET,     // accumulation cap off
  .first .K405  = LR_RESET,   // mux the himux2.



#if 0
  /*
    all relays have to be defined. not left default initialization of 00 which means
    they don't get an initial pulse.
  */

  //  maybe make explicit all values  U408_SW_CTL. at least for the initial mode, from which others derive.

  .first .K406_CTL  = LR_SET,     // accumulation relay off

  .first. K405_CTL  = LR_RESET,     // dcv input relay k405 switch off - works.
  .first. K402_CTL  = LR_RESET,     // dcv-div/directz relay off
                                // must match app->persist_fixedz

  .first. K401_CTL  = LR_SET,     // dcv-source relay off.
  .first. K403_CTL  = LR_RESET,     // ohms relay off.

  .first .U408_SW_CTL = 0,      // b2b fets/ input protection off/open


  // AMP FEEDBACK SHOULD NEVER BE TURNED OFF.
  // else draws current, and has risk damaging parts. mux pin 1. of adg. to put main amplifier in buffer/G=1 configuration.
  .first. U506 =  W1,     // should always be on

  // .second.K406_CTL  = LR_OFF,     // clear relay. default.
  // .second.K405_CTL  = LR_OFF,     // clear relay
  .second.U408_SW_CTL = 0,

  .second.U506 =  W1,           // amplifier should always be on.



  .first. K603_CTL  = LR_RESET,     // ohms relay off.


  /////////////////////////
  // 700
  // has inverting cmos buffer
  .first. K702_CTL  = LR_RESET,
  .second.K702_CTL  = 0b11,

  // 0.1R shunt off. has inverting cmos buffer
  .first. K703_CTL  = LR_RESET,
  .second.K703_CTL  = 0b11,

  // shunts / TIA - default to shunts
  .first. K709_CTL  = LR_SET,

  // agn200 shunts are off.
  .first. K707_CTL  = LR_SET,
  .first. K706_CTL  = LR_SET,
  .first. K704_CTL  = LR_SET,
  .first. K705_CTL  = LR_SET,



  .reg_mode = MODE_LO,                                  // default, blink led according to mcu

  .reg_sa_p_clk_count_precharge = CLK_FREQ * 500e-6,             //  `CLK_FREQ * 500e-6 ;   // 500us.

  .reg_adc_p_aperture = CLK_FREQ * 0.2,   // 200ms. 10nplc 50Hz.  // Not. should use current calibration?  // should be authoritative source of state.

  .reg_adc_p_reset = CLK_FREQ * 500e-6                // 500us.

#endif

};



static Mode mode_current;


int main(void)
{
  // hse
	rcc_clock_setup_pll(&rcc_hse_8mhz_3v3[RCC_CLOCK_3V3_84MHZ] );  // stm32f411  upto 100MHz.

  // clocks
  rcc_periph_clock_enable(RCC_SYSCFG); // maybe required for external interupts?

  // gpio

  // rcc_periph_clock_enable(RCC_GPIOA  | RCC_GPIOB | RCC_GPIOC);
  rcc_periph_clock_enable(RCC_GPIOA);
  rcc_periph_clock_enable(RCC_GPIOB);
  rcc_periph_clock_enable(RCC_GPIOC);
  // rcc_periph_clock_enable(RCC_GPIOE);

  // rcc_periph_clock_enable(RCC_USART1 | RCC_SPI1 | RCC_ADC1);

  // USART
  rcc_periph_clock_enable(RCC_USART1);

  // spi / ice40
  rcc_periph_clock_enable(RCC_SPI1);

  // adc/temp
  rcc_periph_clock_enable(RCC_ADC1);



  /////////////////////////////
  /*
    peripheral/ports setup
  */

  led_setup();

  // setup external state for critical error led blink
  // because assert() cannot pass a context
  assert_critical_error_led_setup(LED_PORT, LED_OUT);


  // this is the mcu clock.  not the adc clock. or the fpga clock.
  // systick_setup(16000);

  // extern void systick_setup(uint32_t tick_divider, void (*pfunc)(void *),  void *ctx);
  systick_setup(84000,  (void (*)(void *)) sys_tick_interupt, &app);  // 84MHz.


  //////////////////////
  // main app setup

  memset(&app, 0, sizeof(app_t));

  // would be neater/possible, to use a static initializer for app ?

  app.spi = SPI1 ;
  app.led_blink = true;
  app.test_relay_flip = false;    // TODO remove

  app.mode_initial =  &mode_initial;
  app.mode_current =  &mode_current;


  /////////////////


  // uart/console
  cbuf_init(&app.console_in,  buf_console_in, sizeof(buf_console_in));
  cbuf_init(&app.console_out, buf_console_out, sizeof(buf_console_out));

  cbuf_init_stdout_streams(  &app.console_out );
  cbuf_init_stdin_streams( &app.console_in );


  cstring_init(&app.command, buf_command, buf_command + sizeof( buf_command));

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




  printf("sizeof app_t %u\n", sizeof(app_t));



  ////////////////
  // spi1, for adum/ice40

  spi1_port_cs1_cs2_setup();

  spi1_port_interupt_setup( (void (*) (void *))spi1_interupt, &app);

  ice40_port_extra_setup();


 // spi_ice40_setup( app.spi );




  // modes_init();

  // go to main loop
  app_loop(&app);

  for (;;);
  return 0;


}




