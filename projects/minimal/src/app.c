


#include <stdio.h>    // printf, scanf
#include <string.h>   // strcmp, memset
#include <assert.h>
#include <malloc.h> // malloc_stats()
#include <stdlib.h>   // abs()



#include <libopencm3/stm32/rcc.h>   // for clock initialization
#include <libopencm3/cm3/scb.h>  // reset()
#include <libopencm3/stm32/spi.h>   // SPI1

#include <lib2/util.h>   // msleep(), UNUSED
#include <lib2/format.h>   // trim_whitespace()  format_bits()

#include <peripheral/led.h>
#include <peripheral/spi-port.h>
#include <peripheral/ice40-extra.h>
#include <peripheral/spi-ice40.h>
#include <peripheral/spi-4094.h>
#include <peripheral/spi-ice40-bitstream.h>
#include <peripheral/spi-dac8811.h>
#include <peripheral/spi-ad5446.h>



#include <mode.h>
#include <app.h>
#include <util.h>


#include <ice40-reg.h>




// fix me
int flash_lzo_test(void);


/*
  keep general repl stuff (related to flashing, reset etc) here,
  put app specific/ tests in a separatefile.

*/


void app_repl_statement(app_t *app,  const char *cmd)
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
  double f0;

  int32_t i0;

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


  else if( strcmp(cmd, "test01") == 0) {


      app_repl_statements(app, "set k405 set; set k406 reset; set k407 reset\n" );

  }



  else if( sscanf(cmd, "sleep %100s", s0) == 1
    && str_decode_float( s0, &f0))
  {
    // allows 1 or 1000m  etc. not sure,
    msleep( (uint32_t ) (f0 * 1000), &app->system_millis);
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


#if 0
  else if( sscanf(cmd, "blink %lu", &u0 ) == 1) {
    // turn off fpga blink in mode 0, avoid spi transmission, during acquisition.
    app->led_blink = u0;
  }

  else if( sscanf(cmd, "test relay flip %lu", &u0 ) == 1) {

    app->test_relay_flip = u0;
  }
#endif



  // change name fpga bitstrea load/test
  else if(strcmp(cmd, "bitstream test") == 0) {

    spi_ice40_bitstream_send(app->spi, & app->system_millis );
  }

  // don't we have some code - to handle sscan as binary/octal/hex ?


#if 0

  else if( sscanf(cmd, "direct %100s", s0) == 1
    && str_decode_uint( s0, &u0)
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
#endif


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

#if 0
  /* not useful. since is immediately written over - in the spi transition.
      use 'set mode x'. insteadd
  */

  else if( sscanf(cmd, "mode %lu", &u0 ) == 1) {

    // set the fpga mode.
    spi_mux_ice40(app->spi);
    spi_ice40_reg_write32(app->spi, REG_MODE, u0 );

    uint32_t ret = spi_ice40_reg_read32(app->spi, REG_MODE );
    printf("reg_mode return value %lu\n", ret);
  }
#endif




  else if( strcmp(cmd, "mode?") == 0) {

    // TODO add some decoding here.
    spi_mux_ice40(app->spi);
    uint32_t ret = spi_ice40_reg_read32(app->spi, REG_MODE );
    printf("reg_mode return value %lu\n", ret);

    // _mode_t *mode = app->mode_current;
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
    && str_decode_uint( s0, &u0)
  ) {
       // spi_mux_dac8811(app->spi);
      spi_mux_ad5446(app->spi );

      // eg. 0=-0V out.   0xffff = -7V out. nice.
      spi_dac8811_write16( app->spi, u0 );

      spi_mux_ice40(app->spi);
    }
#endif

  ///////////////////////


  else if(strcmp(cmd, "reset") == 0) {

    // reset the mode.
    *app->mode_current = *app->mode_initial;
  }



  else if( app_test05( app, cmd  )) { }
  else if( app_test14( app, cmd  )) { }


/*
   set dcv-source 10; set mode 5; set nplc 1;
    nice.

  do the same except with the dac.
  eg. dac-source
*/

  // +10,0,-10.    if increment. then could use the dac.
  else if( sscanf(cmd, "set dcv-source %ld", &i0 ) == 1) {

      printf("setup dcv-source, input relays, on current_mode\n");

      _mode_t *mode = app->mode_current;


      if(i0 == 10) {
        printf("with +10V\n");
        mode->second.U1003  = S1 ;       // s1. dcv-source s1. +10V.
      }
      else if(i0 == -10) {
        printf("with -10V\n");
        mode->second.U1003  = S2 ;       // s2.  -10V.
      }
      else if(i0 == 0) {
        printf("with 0V\n");
        mode->second.U1003 = S3;          // s3 == agnd
      }
      else {
        printf("bad arg\n");
        return;
      }

      mode->second.U1006  = S1 ;          // s1.   follow  .   dcv-mux2

      // setup input relays.
      mode->first .K405 = LR_SET;     // select dcv
      mode->first .K406 = LR_SET;   // accum relay off
      mode->first .K407 = LR_RESET;   // select dcv-source
  }



  // our str_decode_uint function doesn't handle signedness...
  // and we want the hex value.
  // but we could

  else if( sscanf(cmd, "set dac dcv-source %100s", s0) == 1
    && str_decode_uint( s0, &u0)) {

      // should
      // eg. set dac dcv-source 0x3fff

      _mode_t *mode = app->mode_current;

      if(u0 > 0) {
        printf("with +10V\n");
        mode->second.U1003  = S1 ;       // s1. dcv-source s1. +10V.
      }
      else {
        // TODO. handle signedness in str_decode_uint.
        assert( 0 );
        printf("with -10V\n");
        mode->second.U1003  = S2 ;       // s2.  -10V.
      }

      mode->second.U1006  = S3;          // s1.   follow  .   dcv-mux2

      // range check.

      mode->dac_val = u0;// abs( u0 );

      // setup input relays.
      mode->first .K405 = LR_SET;     // select dcv
      mode->first .K406 = LR_SET;   // accum relay off
      mode->first .K407 = LR_RESET;   // select dcv-source
  }


  /*
      we have to disambiguate values with float args explicitly...
      because float looks like int
  */

  else if( sscanf(cmd, "set aper %100s", s0) == 1
    && str_decode_float( s0, &f0))
  {
    assert(app->mode_current);
    _mode_t * mode = app->mode_current;

    printf("set aperture\n");
    uint32_t aperture = period_to_aper_n( f0 );
    // assert(u1 == 1 || u1 == 10 || u1 == 100 || u1 == 1000); // not really necessary. just avoid mistakes
    aper_n_print( aperture,  app->line_freq);
    mode->adc.reg_adc_p_aperture = aperture;
  }


  else if( sscanf(cmd, "set nplc %100s", s0) == 1
    && str_decode_float( s0, &f0))
  {
    // use float here, to express sub 1nplc periods
    if( ! nplc_valid( f0 ))  {
        printf("bad nplc arg\n");
        // return 1;
    } else {

      assert(app->mode_current);
      _mode_t * mode = app->mode_current;

      // should be called cc_aperture or similar.
      uint32_t aperture = nplc_to_aper_n( f0, app->line_freq );
      aper_n_print( aperture,  app->line_freq);
      mode->adc.reg_adc_p_aperture = aperture;
    }
  }


  else if( strcmp(cmd, "nplc?") == 0 || strcmp(cmd, "aper?") == 0) {
    // query fpga directly. not mode
    spi_mux_ice40(app->spi);
    uint32_t aperture = spi_ice40_reg_read32(app->spi, REG_ADC_P_CLK_COUNT_APERTURE );
    aper_n_print( aperture,  app->line_freq);
  }




  else if( sscanf(cmd, "set %100s %100s", s0, s1) == 2
    && str_decode_uint( s1, &u0)
  ) {

      assert(app->mode_current);
      _mode_t * mode = app->mode_current;


      printf("set %s %lu\n", s0, u0);

      // cannot manage pointer to bitfield. so have to hardcode.

      // ice40 mode.
      if(strcmp(s0, "mode") == 0) {
        mode->reg_mode = u0;
      }
      else if(strcmp(s0, "direct") == 0) {
        assert(sizeof(mode->reg_direct) == 4);
        assert(sizeof(u0) == 4);
        memcpy( &mode->reg_direct, &u0, sizeof(mode->reg_direct));
      }

      else if(strcmp(s0, "u1003") == 0) {
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
          handle latch relay pulse encoding here, rather than at str_decode_uint() time.
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

      ////////////

      // this is nice.
      else if(strcmp(s0, "leds_o") == 0) {
        mode->reg_direct.leds_o = u0;
      }
      else if(strcmp(s0, "monitor_o") == 0) {
        mode->reg_direct.monitor_o = u0;
      }
      // sample acquisition - direct state
      else if(strcmp(s0, "sig_pc_sw_o") == 0) {
        mode->reg_direct.sig_pc_sw_o= u0;
      }
/*
      else if(strcmp(s0, "sig_pc2_sw_o") == 0) {
        mode->reg_direct.sig_pc2_sw_o = u0;
      }
*/
      else if(strcmp(s0, "azmux_o") == 0) {
        mode->reg_direct.azmux_o = u0;
      }

      ////////////
#if 0
      // adc precharge.
      // want a separate decoder.  to handle ms. us. suffix.
      // probably should move out of here...
      else if(strcmp(s0, "aperture") == 0) {
        mode->adc.reg_adc_p_aperture = u0;
      }
      else if(strcmp(s0, "precharge") == 0) {
        mode->sa.reg_sa_p_clk_count_precharge = u0;
      }
#endif

      else {

        printf("unknown target %s\n", s0);
      }


  }





  else {

    printf("unknown cmd, or bad argument '%s'\n", cmd );

  }
}




void app_repl_statements(app_t *app,  const char *s)
{
  assert(app);
  assert(s);


  cstring_t stmt;
  char buf_stmt[ 1000 ];  // stack allocation...
  cstring_init(&stmt, buf_stmt, buf_stmt + sizeof( buf_stmt));

  while(*s) {

    int32_t ch = *s;
    assert(ch >= 0);

    if(ch == ';' || ch == '\n')
    {
      // a separator, then apply what we have so far.
      char *cmd = cstring_ptr( &stmt);
      cmd = str_trim_whitespace_inplace( cmd );
      app_repl_statement(app, cmd);
      cstring_clear( &stmt);
    }
    else if( cstring_count(&stmt) < cstring_reserve(&stmt) ) {
      // push char, unless overflow
      cstring_push_back(&stmt, ch);
    } else {
      // ignore overflow chars,
      printf("too many chars!!\n");
    }

    // nice to be able to span multiple commands. so ignore *s == 0
    if(ch == '\n')
    {
      printf("calling spi_mode_transition_state()");
      spi_mode_transition_state( app->spi, app->mode_current, &app->system_millis);
    }

    ++s;
  }

}


