
// change name repl.c ? commands.c
// need "cal load %u;",  and "cal save %u;"  for adc.


#include <math.h>      // fabs
#include <stdio.h>      // printf, scanf
#include <string.h>     // strcmp, memset
#include <assert.h>
#include <ctype.h>      // isdigit

#include <malloc.h> // malloc_stats()

// lib
#include <lib2/util.h>       // print_stack_pointer()
#include <lib2/usart.h>      // flush
#include <lib2/format.h>   // format_bits()



// local
#include "reg.h"
#include "mux.h"        // mux_ice40()
#include "spi-ice40.h"  // spi_ice40_reg_write32()


#include "mcu-temp.h"


#include <libopencm3/stm32/flash.h>

#include "stream-flash.h"
#include "file-blob.h"   // change name stream-cal.

#include "app.h"
// modes of operation.
#include "mode.h"


#include "calc.h"     // m_truncate_rows() name to general purpose matrix stuff.



/*
  Ok, we want to be able to set the input source. and then the amplifier gain.
  extra_functions.


  we can make it easier.

  have amp...  and input commands
  rather than range.
  ---
  not sure - it would be nice to work - 2W. and 22. then don't have

  Or dcv10s  - meaning using local soucr.   and dcv10 as terminal <- YES.
*/




/*
  EXTR.

    for stuff like stddev, date etc.  of cal.
    just store it all in a single instead . it's enough structure
    like - can we can record this when we do a cal in a string. or string buffer.
    ----
    rename file_blob_serialize() d

    ---------

    for the cal slot/name .  use a string identifier. not a number.
    good convenience.
*/

static void my_file_cal_write( FILE *f, Header *header, MAT *b )      // should pass app. to allow stor may be better t
{
  assert(f);
  assert(header);
  assert(b);

  // set the blob id
  assert(header->len == 0 && header->magic == 0);   // these are not set yet
  header->id = 106;

  // write the data
  m_foutput_binary( f, b);
}



static void my_file_blobs_scan( FILE *f, Header *header, app_t *app )
{
  assert(header);
  assert(app);
  // ctx should be the app structure.. to save.

  printf("whoot got blob id=%u len=%u\n", header->id, header->len );

  if(header->id == 106) {
    // it should be readable.

    // OK. hang on. it's not a file sturcture...
    // we should load it from memory????
    app->b = m_finput_binary(f, MNULL );

  }
}



bool app_functions( app_t *app , const char *cmd)
{
  /*
      - this modifies current-state.
      - actually could/ should be placed on app. be p

      - commands to set dcv-source
      - commands to set range.
      - commands to set mode az, nplc. etc
        -----------

      - to persist changinig input range with change dc-source we have to persist the mode.
        actually maybe not. maybe not.


  */


  assert(app);
  assert(cmd);

  double    f0;
  uint32_t  u1; // change name u0.


  // https://stackoverflow.com/questions/24746111/scanf-field-width-string-overflow
  char s0[100 + 1];
  char s1[100 + 1];



  // is there a way to represent/ aor force variable whitespace? sscanf?

  if( sscanf(cmd, "lfreq %lu", &u1 ) == 1) {

    if(  !(u1 == 50 || u1 == 60)) {
      // be safe for moment.
      printf("bad lfreq arg\n" );
      return 1;
    }

    printf("set lfreq\n" );
    app->lfreq = u1;

    return 1;
  }


  if( sscanf(cmd, "verbose %lu", &u1 ) == 1) {

    printf("set verbosity %lu\n", u1  );
    app->verbose = u1;
    return 1;
  }


  if( sscanf(cmd, "buffer %lu", &u1 ) == 1) {

    // if(u1 < 2 || u1 > 500 ) {
    if(u1 < 1 || u1 > 10000 ) {
      printf("set buffer size bad arg\n" );
      return 1;
    }

    printf("set buffer size %lu\n", u1  );

    // the issue is that once the buffer is full.. it won't keep filling it.
    // actually it will.
    // and we can specify what to - when buffer is full.

    /* we should free and recreate buffer here - in order to free the memory.
        otherwise it ends up being allocated oversized.

      - on a large matrix,
        this frees the mesch data structure.
        but malloc() still hangs on to the reserved heap it took, like a page.
    */

    M_FREE(app->sa_buffer);

    app->sa_buffer = m_resize( app->sa_buffer, u1 , 1 );

    app->sa_buffer = m_truncate_rows( app->sa_buffer, 0 );

    assert(m_rows( app->sa_buffer) == 0);

    return 1;
  }


  else if(strcmp(cmd, "null") == 0) {
    // todo
    return 1;
  }
  else if(strcmp(cmd, "div") == 0) {
    // div by gain.before
    // should probably be gain and offset for the calibration.
    // todo
    return 1;
  }



  /*
      - perhaps rename 'acquire off/arm'.  acquire trigger/on
  */

  else if(strcmp(cmd, "arm") == 0 || strcmp(cmd, "a") == 0
      || strcmp(cmd, "halt") == 0 || strcmp(cmd, "h") == 0) {

      printf("set arm\n" );
      // run/pause, stop/go, reset,set etc.
      // edge triggered. so must perform in sequence
      // make sure fpga is in a default mode.
      spi_ice40_reg_write32(app->spi, REG_SA_ARM_TRIGGER, 0 );
      return 1;
  }
  else if(strcmp(cmd, "trig") == 0 || strcmp(cmd, "t") == 0) {
      printf("trigger sample acquisition\n" );

      /* clear the sample buffer
        alternatively could use a separate command,  'buffer clear'
        have an expected buffer - means can stop when finished.
      */
      assert(app->sa_buffer);
      app->sa_buffer      = m_zero( app->sa_buffer ) ;    // we don't really even have to zero the buffer.
                                                          // we shouldn't need this.

      m_truncate_rows( app->sa_buffer, 0 );               // truncate vertical length.


      // app->sa_buffer_full = false;
      app->sa_count_i  = 0;

      // trigger the sample acquisition
      spi_ice40_reg_write32(app->spi, REG_SA_ARM_TRIGGER, 1 );
      return 1;
  }


  else if( sscanf(cmd, "print %100s", s0) == 1
    || sscanf(cmd, "echo %100s", s0) == 1) {

    printf( s0  );
    printf("\n");
    return 1;
  }

  /*
    It would be very nice, if we could manage all relays for testing.
    Kind of like the direct register, for fpga outputs.

  */





  else if( sscanf(cmd, "set %100s %100s", s0, s1) == 2) {

    // light weight interface to control relays and muxes.
    // printf("s1 is %s\n", s1 );

    uint32_t val = 0;

    if (strcmp(s1, "on") == 0 || strcmp(s1, "true") == 0 || strcmp(s1, "bot") == 0)
      val = 1;
    else if(strcmp(s1, "off") == 0 || strcmp(s1, "false") == 0 || strcmp(s1, "top") == 0)
      val = 0;

    // 1 of 8 mux values.
    else if(strcmp(s1, "s8") == 0 )
      val = S8;
    else if(strcmp(s1, "s7") == 0 )
      val = S7;
    else if(strcmp(s1, "s6") == 0 )
      val = S6;
    else if(strcmp(s1, "s5") == 0 )
      val = S5;
    else if(strcmp(s1, "s4") == 0 )
      val = S4;
    else if(strcmp(s1, "s3") == 0 )
      val = S3;
    else if(strcmp(s1, "s2") == 0 )
      val = S2;
    else if(strcmp(s1, "s1") == 0 )
      val = S1;






    // we could factor all this handling.
    // read_int.
    else if( s1[0] == '0' && s1[1] == 'x' && sscanf(s1, "%lx", &val) == 1) {
      printf("got hex\n" );
    }
    else if( s1[0] == '0' && s1[1] == 'o' && sscanf(s1 + 2, "%lo", &val) == 1) {
      // octal doesn't accept a prefix
      // printf("got octal\n" );
    }
    else if( s1[0] == '0' && s1[1] == 'b') {
      // binary is very useful for muxes
      val = strtoul(s1 + 2, NULL, 2);
      // char buf[100];
      // printf("got binary %s\n", format_bits(buf, 32, val ) );
    }
    else if( isdigit( (unsigned char) s1[0] ) && sscanf(s1, "%lu", &val) == 1) {
      // printf("got decimal\n" );
    }
    else {
      printf("bad val arg\n" );
      return 1;
    }




    Mode *mode = app->mode_current;

    uint8_t rval = val ?  LR_BOT : LR_TOP;


    if(strcmp(s0, "dummy") == 0) {
      printf("dummy val is %lu\n", val );
    }
    else if(strcmp(s0, "u1003") == 0) {

      mode->first.U1003 = val ;
      mode->second.U1003 = val ;
      printf("setting u1003 %b\n", mode->second.U1003 );
    }
    else if(strcmp(s0, "k406") == 0 || strcmp(s0, "accum") == 0) {
      mode->first.K406_CTL = rval ;     //  off should be top.
    }


    // 500 ohms
    else if(strcmp(s0, "u605") == 0) {
      mode->first.U605 = val ;
      mode->second.U605 = val ;
      printf("setting u605 %b\n", mode->first.U605 );
    }

    // current
   else if(strcmp(s0, "u703") == 0) {
      mode->first.U703= val ;
      mode->second.U703 = val ;
      printf("setting u703 %b\n", mode->first.U703);
    }
   else if(strcmp(s0, "u702") == 0) {
      mode->first.U702= val ;
      mode->second.U702 = val ;
      printf("setting u702 %b\n", mode->first.U702);
    }





    else if(strcmp(s0, "k601") == 0) {
      mode->first.K601_CTL = rval;
    }
    else if(strcmp(s0, "k602") == 0) {
       mode->first.K602_CTL = rval;
    }
    else if(strcmp(s0, "k603") == 0) {
      mode->first.K603_CTL = rval;
    }


    /////////////////
    // current
    // discrete cmos inverting.
    else if(strcmp(s0, "k702") == 0)
      mode->first.K702_CTL = rval;

    // discrete cmos inverting.
    else if(strcmp(s0, "k703") == 0)
      mode->first.K703_CTL = rval;


    else if(strcmp(s0, "k709") == 0)
      mode->first.K709_CTL = rval;

    else if(strcmp(s0, "k707") == 0)
      mode->first.K707_CTL = rval;
    else if(strcmp(s0, "k706") == 0)
      mode->first.K706_CTL = rval;
    else if(strcmp(s0, "k704") == 0)
      mode->first.K704_CTL = rval;
    else if(strcmp(s0, "k705") == 0)
      mode->first.K705_CTL = rval;



    else {

      printf("bad target arg\n" );
      return 1;
    }

    // do the state transition
    // should only do this on trig.
    app_transition_state( app->spi, mode,  &app->system_millis );
    return 1;
  }


#if 0

  now use set accum on/off.

  else if( sscanf(cmd, "accum %100s", s0) == 1) {

    // manual ontrol over test charge-accumulation relay - can be useful for quick checks. but a bit out-of-band for dcv operation.
    // eg. can control other relays.
    // what happens if add 10nF. to gnd. to the signal path.

    if (strcmp(s0, "off") == 0) {
      printf("set accum off\n" );
      app->mode_current->first.K406_CTL = LR_TOP;
    }
    else if(strcmp(s0, "on") == 0) {
      printf("set accm on\n" );
      app->mode_current->first.K406_CTL = LR_BOT;
    }
    else {
      printf("bad accum arg\n" );
      return 1;
    }
    // do the state transition
    app_transition_state( app->spi, app->mode_current,  &app->system_millis );
    return 1;
  }
#endif




  else if( sscanf(cmd, "fixedz %100s", s0) == 1) {

    // can only be for dcv10,dcv1,dcv01.  ranges. but doesn't matter to turn on for dcv1000,dcv
    // but needs to persist.

    Mode *mode = app->mode_current;

    if(strcmp(s0, "on") == 0) {

      printf("set persist_fixedz on\n" );
      app->persist_fixedz = true;
    } else if (strcmp(s0, "off") == 0) {

      printf("set persist_fixedz off\n" );
      app->persist_fixedz = false;
    } else {
      printf("bad persist_fixedz arg\n" );
      return 1;
    }

    // follow persist_fixedz for 10Meg/high-z.
    mode->first.K402_CTL = app->persist_fixedz ?  LR_TOP :  LR_BOT ;

    // do the state transition
    app_transition_state( app->spi, mode,  &app->system_millis );
    return 1;
  }


  /*
      - switching between azero and non-az, and boot is hard.
      - becase we have to remember/persist the lo to use.
      - persist_azmux_val.
    */
  else if( sscanf(cmd, "azero %100s", s0) == 1) {

    Mode *mode = app->mode_current;

    if(strcmp(s0, "on") == 0) {

      printf("set azero on, using app.persist_azmux_val \n" );
      mode->reg_mode = MODE_AZ;
      mode->reg_direct.azmux  = app->persist_azmux_val;    // lo
    }
    else if (strcmp(s0, "off") == 0) {

      printf("set azero off - muxing signal through az mux\n" );
      // this is a constant configuration.
      mode->reg_mode = MODE_NO_AZ;
      mode->reg_direct.sig_pc_sw_ctl  = SW_PC_SIGNAL;   // pc switch muxes signal.
      mode->reg_direct.azmux          = AZMUX_PCOUT;             // azmux muxes pc-out
    }
    else {
      printf("bad azero arg\n" );
      return 1;
    }
    // do the state transition
    app_transition_state( app->spi, mode,  &app->system_millis );
    return 1;
  }




  else if( sscanf(cmd, "electro %100s", s0) == 1) {

    // perhaps easier as meta command, expressed in terms of other parameters.

    Mode *mode = app->mode_current;

    if(strcmp(s0, "on") == 0) {

      printf("set electro on, muxing boot\n" );
      mode->reg_mode = MODE_NO_AZ;
      mode->reg_direct.sig_pc_sw_ctl  = SW_PC_BOOT;   // reduce leakage
      mode->reg_direct.azmux          = AZMUX_BOOT;           // azmux muxes boot directly-out
    }
    else if (strcmp(s0, "off") == 0) {
      // what state do we revert to when coming out of electrometer mode?
      // this is tricky.  also acal.
      // normal az mode perhaps?

      printf("set electro off, switch to no-az mode\n" );
      mode->reg_mode = MODE_NO_AZ;
      mode->reg_direct.sig_pc_sw_ctl  = SW_PC_SIGNAL;   // pc switch muxes signal.
      mode->reg_direct.azmux          = AZMUX_PCOUT;             // azmux muxes pc-out
    }
    else {
      printf("bad electro arg\n" );
      return 1;
    }
    // do the state transition
    app_transition_state( app->spi, mode,  &app->system_millis );
    return 1;
  }



  ////////////////////////////////////////////////////////


  /* TODO

      change name sa_precharge and adc_reset
      or sa precharge  ,  adc reset
  */
  else if( sscanf(cmd, "precharge %lu", &u1 ) == 1) {

    printf("set precharge period in clk counts \n");
    Mode *mode = app->mode_current;
    mode->reg_sa_p_clk_count_precharge  = u1;
    // do the state transition
    app_transition_state( app->spi, mode,  &app->system_millis );
    return 1;
  }
  else if( strcmp(cmd, "precharge?") == 0) {

    mux_ice40(app->spi);
    printf("prechare aperture %lu\n", spi_ice40_reg_read32(app->spi, REG_SA_P_CLK_COUNT_PRECHARGE ));
    return 1;
  }



  else if( sscanf(cmd, "adc reset %lu", &u1 ) == 1) {

    // need a better name. this is the adc integrator reset period, not command to reset the adc.
    printf("set adc reset in clk counts \n");
    Mode *mode = app->mode_current;
    mode->reg_adc_p_reset = u1;
    // do the state transition
    app_transition_state( app->spi, mode,  &app->system_millis );
    return 1;
  }
  else if( strcmp(cmd, "adc reset?") == 0) {

    mux_ice40(app->spi);
    printf("adc reset %lu\n", spi_ice40_reg_read32(app->spi, REG_ADC_P_CLK_COUNT_RESET));
    return 1;
  }







  else if( sscanf(cmd, "nplc %lf", &f0 ) == 1) {

    if( ! nplc_valid( f0 ))  {
        printf("bad nplc arg\n");
        return 1;
    };

    uint32_t aperture = nplc_to_aper_n( f0, app->lfreq );
    printf("aperture %lu\n",   aperture );
    printf("nplc     %.2lf\n",  aper_n_to_nplc( aperture, app->lfreq ));
    printf("period   %.2lfs\n", aper_n_to_period( aperture ));

    // set new aperture
    app->mode_current->reg_adc_p_aperture = aperture ;

    // do the state transition
    app_transition_state( app->spi, app->mode_current,  &app->system_millis );

    // EXTR. i think this may be overwriting mode - which is only being set with direct fpga calls. and not via the current_mode.

    return 1;
  }

  else if( strcmp(cmd, "nplc?") == 0 || strcmp(cmd, "aper?") == 0) {

    mux_ice40(app->spi);
    uint32_t aperture = spi_ice40_reg_read32(app->spi, REG_ADC_P_APERTURE );
    printf("aperture %lu\n",   aperture );
    printf("nplc     %.2lf\n",  aper_n_to_nplc( aperture, app->lfreq ));
    printf("period   %.2lfs\n", aper_n_to_period( aperture ));

    return 1;
  }

  else if( sscanf(cmd, "aper %lf", &f0 ) == 1) {

    // period. aperture in seconds. period to aperature n
    printf("set aperture\n");
    uint32_t aperture = period_to_aper_n( f0 );
    // assert(u1 == 1 || u1 == 10 || u1 == 100 || u1 == 1000); // not really necessary. just avoid mistakes

    printf("aperture %lu\n",   aperture );
    printf("nplc     %.2lf\n",  aper_n_to_nplc( aperture, app->lfreq ));
    printf("period   %.2lfs\n", aper_n_to_period( aperture ));

    app->mode_current->reg_adc_p_aperture = aperture ;
    app_transition_state( app->spi, app->mode_current,  &app->system_millis );
    return 1;
  }


#if 0
  else if( sscanf(cmd, "sample %100s", s0) == 1) {

    /* set sample acquisition input source.
        idea is to make it simple for common tests, instead of setting set himux and himux2. and lomux. and azmode etc.
        -----
        EXTR. we want to support BOOT mode as well.
        And sample ref-lo
    */


    Mode *mode = app->mode_current;

    // EXTR. - set these using initial and a bitmask?
    // turn external inputs off.
    mode->first .K406_CTL  = LR_TOP;     // accumulation relay off
    mode->first. K405_CTL  = LR_BOT;     // dcv input relay off
    mode->first. K402_CTL  = LR_BOT;     // dcv-div/directz relay off
    mode->first. K401_CTL  = LR_TOP;     // dcv-source relay off.
    mode->first. K403_CTL  = LR_BOT;     // ohms relay off.
    mode->first .U408_SW_CTL = 0;        // b2b fets/ input protection off/open


    if (strcmp(s0, "dcv-source") == 0) {
      mode->reg_direct.himux  = HIMUX_HIMUX2;
      mode->reg_direct.himux2 = HIMUX2_DCV_SOURCE;
      app->persist_azmux_val =  AZMUX_REF_LO;     // dcv-source uses ref-lo for now.
    }
    else if (strcmp(s0, "dcv") == 0) {

      // turn dcv input relay  on
      // need to sequence b2b fets
      mode->first. K405_CTL  = LR_TOP,
      mode->reg_direct.himux  = HIMUX_DCV_HI;
      mode->reg_direct.himux2 = HIMUX2_STAR_LO;
      app->persist_azmux_val =  AZMUX_STAR_LO;     // star-lo
    }
    else if(strcmp(s0, "ref-lo") == 0) {
      // sample ref-lo from the himux.
      mode->reg_direct.himux  = HIMUX_HIMUX2;
      mode->reg_direct.himux2 = HIMUX2_REF_LO ;
      app->persist_azmux_val =  AZMUX_REF_LO;
    }



    /* how we do this depends on current azmode or not.
        think we actually have to persist
        -------
        we kind of want the other case where we sample ref-lo from the lo mux without .
        for noise tests of amp and adc.
        -------
        SAMPLE_MODE_.  SAMPLE_MODE_AZ, SAMPLE_MODE_NOAZ , SAMPLE_MODE_REFLO_LOMUX  etc.
        REALLY NOT SURE ABOUT THIS. it's a lot of damn state.

        rather than store persist. why not just swap the azmux lo to use.
        OR.
        change the structure.
        -------

        EXTR. why not just the lo in the reg_direct_azmux. ?
        if switch to AZ it will be used.
        - because we have to switch to pcout.
    */

    // this can be factored into a function.
    // to make it easier

    if(app->persist_sample_mode == SAMPLE_MODE_AZ) {
      printf("set azero on, using app.persist_azmux_val \n" );
      mode->reg_mode = MODE_AZ;
      mode->reg_direct.azmux  = app->persist_azmux_val;    // lo
    }
    else if(app->persist_sample_mode == SAMPLE_MODE_NOAZ) {

      printf("set azero off - muxing signal through az mux\n" );
      // this is a constant configuration.
      mode->reg_mode = MODE_NO_AZ;
      mode->reg_direct.sig_pc_sw_ctl  = SW_PC_SIGNAL;   // pc switch muxes signal.
      mode->reg_direct.azmux          = AZMUX_PCOUT;             // azmux muxes pc-out
    }
    else if(app->persist_sample_mode == SAMPLE_MODE_BOOT ) {   // sample from the lomux.
      // electrometer/ boot
      mode->reg_mode = MODE_NO_AZ;
      mode->reg_direct.sig_pc_sw_ctl  = SW_PC_BOOT;   // reduce leakage
      mode->reg_direct.azmux          = AZMUX_BOOT;    // azmux muxes boot directly-out
    }
    else if(app->persist_sample_mode == SAMPLE_MODE_REF_LO ) {   // sample from the azmux
      // ref-lo from the lo-mux
      mode->reg_mode = MODE_NO_AZ;
      mode->reg_direct.azmux          = AZMUX_REF_LO;
    }
  }
#endif



  else if( sscanf(cmd, "himux %100s", s0) == 1) {

    // generally we want input relays to be off - except if using dcv.  should we set that here???

    Mode *mode = app->mode_current;

/*
    // REMOVE.
    // EXTR. - these should not really be set here.
    // use reset command instead.

    // turn external inputs off.
    mode->first .K406_CTL  = LR_TOP;     // accumulation relay off
    mode->first. K405_CTL  = LR_BOT;     // dcv input relay off
    mode->first. K402_CTL  = LR_BOT;     // dcv-div/directz relay off
    mode->first. K401_CTL  = LR_TOP;     // dcv-source relay off.
    mode->first. K403_CTL  = LR_BOT;     // ohms relay off.
    mode->first .U408_SW_CTL = 0;        // b2b fets/ input protection off/open
*/
/*
#define HIMUX_HIMUX2          S2
#define HIMUX_DCV_DIV         S3
#define HIMUX_4W-HI           S4
#define HIMUX_DCV_HI             S7
#define HIMUX_DCI_HI             S8

*/

    // himux
    if (strcmp(s0, "dcv-hi") == 0) {               // change name dcv-hi
      mode->reg_direct.himux  = HIMUX_DCV_HI;
      mode->reg_direct.himux2 = HIMUX2_STAR_LO;
    }
    else if (strcmp(s0, "dci-hi") == 0) {
      mode->reg_direct.himux  = HIMUX_DCI_HI;
      mode->reg_direct.himux2 = HIMUX2_STAR_LO;
    }
   else if (strcmp(s0, "dcv-div") == 0) {
      mode->reg_direct.himux  = HIMUX_DCV_DIV;
      mode->reg_direct.himux2 = HIMUX2_STAR_LO;
    }
    else if (strcmp(s0, "4w-hi") == 0) {
      mode->reg_direct.himux  = HIMUX_4W_HI;
      mode->reg_direct.himux2 = HIMUX2_STAR_LO;
    }

    // himux2. stuff.

    else if (strcmp(s0, "dci-tia") == 0) {
      mode->reg_direct.himux  = HIMUX_HIMUX2;
      mode->reg_direct.himux2 = HIMUX2_DCI_TIA;
    }
    else if (strcmp(s0, "ref-hi") == 0) {
      mode->reg_direct.himux  = HIMUX_HIMUX2;
      mode->reg_direct.himux2 = HIMUX2_REF_HI;
    }
    else if (strcmp(s0, "dcv-source") == 0) {
      mode->reg_direct.himux  = HIMUX_HIMUX2;
      mode->reg_direct.himux2 = HIMUX2_DCV_SOURCE;
    }
    else if(strcmp(s0, "ref-lo") == 0) {
      // don't use in normal case. take ref-lo from the lo-mux.
      mode->reg_direct.himux  = HIMUX_HIMUX2;
      mode->reg_direct.himux2 = HIMUX2_REF_LO ;
    }
    else if (strcmp(s0, "star-lo") == 0) {        // disginguished from a lo.  change name to lo,star.
      mode->reg_direct.himux  = HIMUX_HIMUX2;
      mode->reg_direct.himux2 = HIMUX2_STAR_LO;
    }
    else {
      printf("bad himux arg\n" );
      return 1;
    }

    // do the state transition
    app_transition_state( app->spi, app->mode_current,  &app->system_millis );
    return 1;
  }




  else if( sscanf(cmd, "pc %100s", s0) == 1) {

    // this allows us to set up electrometer mode.
    Mode *mode = app->mode_current;

    if(strcmp(s0, "signal") == 0) {
      mode->reg_direct.sig_pc_sw_ctl  = SW_PC_SIGNAL;
    }
    else if(strcmp(s0, "boot") == 0) {
      mode->reg_direct.sig_pc_sw_ctl  = SW_PC_BOOT;
    }
    else {
      printf("bad pc arg\n" );
      return 1;
    }
    // do the state transition
    app_transition_state( app->spi, app->mode_current,  &app->system_millis );
    return 1;
  }



  else if( sscanf(cmd, "azmux %100s", s0) == 1) {

    // EXTR. important. must specify pc-out - if used for non-az mode.

    if(strcmp(s0, "pcout") == 0 || strcmp(s0, "pc-out") == 0) {
      app->persist_azmux_val = AZMUX_PCOUT;
    }
    else if(strcmp(s0, "boot") == 0) {
      app->persist_azmux_val = AZMUX_BOOT;
    }
    else if (strcmp(s0, "star-lo") == 0) {
      app->persist_azmux_val =  AZMUX_STAR_LO;
    }
    else if (strcmp(s0, "ref-lo") == 0) {
      app->persist_azmux_val =  AZMUX_REF_LO;
    }
    // dci_lo, 4w_lo
    else {
      printf("bad azmux arg\n" );
      return 1;
    }

    app->mode_current->reg_direct.azmux  = app->persist_azmux_val ;    // lo

    // do the state transition
    app_transition_state( app->spi, app->mode_current,  &app->system_millis );
    return 1;
  }


  // amp gain.
  else if( sscanf(cmd, "gain %lu", &u1 ) == 1
    || sscanf(cmd, "amp %lu", &u1 ) == 1
    ) {

    Mode *mode = app->mode_current;

    if(  u1 == 1 ) {
      mode->first. U506 =  W1;
      mode->second.U506 =  W1;
    }
    else if(  u1 == 10 ) {
      mode->first. U506 =  W2;
      mode->second.U506 =  W2;
    }
    else if(  u1 == 100 ) {
      mode->first. U506 =  W3;
      mode->second.U506 =  W3;
    }
    else {
      printf("bad gain arg\n" );
      return 1;
    }

    // do the state transition
    app_transition_state( app->spi, app->mode_current,  &app->system_millis );
    return 1;
  }




  else if( sscanf(cmd, "dcv-source %lf", &f0) == 1) {

    // have a separate commad for temp. etc.
    Mode *mode = app->mode_current;
    if(f0 >= 0)
      mode->second.U1003 =  U1003_POS;
    else
      mode->second.U1003 =  U1003_NEG;

    // todo -  need a float compare. for rounding?.
    if(fabs(f0)  == 10.0)
      mode->second.U1006 =  U1006_REF10V;
    else if(fabs(f0) == 1.0)
      mode->second.U1006 =  U1006_REF1V;
    else if(fabs(f0) == 0.1)
      mode->second.U1006 =  U1006_REV0V1;
    else if(fabs(f0) == 0)
      mode->second.U1006 =  U1006_DCV_LO;
    else
      printf("bad dcv-source arg\n");

    app_transition_state( app->spi, app->mode_current,  &app->system_millis );

    return 1;
  }

  else if( sscanf(cmd, "dcv-source %100s", s0) == 1) {

    // have a separate commad for temp. etc.
    Mode *mode = app->mode_current;

    mode->second.U1003 =  U1003_AGND;

    // todo -  need a float compare. for rounding?.
    if(strcmp(s0, "temp") == 0 )
      mode->second.U1006 =  U1006_TEMP;
    else if(strcmp(s0, "tp") == 0 )
      mode->second.U1006 =  U1006_TP;
    else if(strcmp(s0, "star-lo") == 0 )
      mode->second.U1006 =  U1006_STAR_LO;
    else if(strcmp(s0, "agnd") == 0 )
      mode->second.U1006 =  U1006_AGND;
    else
      printf("bad dcv-source arg\n");

    app_transition_state( app->spi, app->mode_current,  &app->system_millis );

    return 1;
  }

  else if( sscanf(cmd, "dci-source %100s", s0) == 1) {

    Mode *mode = app->mode_current;

    // TODO could simplify case handling, by dealing with 10mA case first.
    if(strcmp(s0, "1mA") == 0 || strcmp(s0, "100uA") == 0
      || strcmp(s0, "10uA") == 0 || strcmp(s0, "1uA") == 0
      || strcmp(s0, "100nA") == 0 || strcmp(s0, "10nA") == 0
    ) {

      // >  set k603 bot; set k602 bot; set k601 bot;  set u605 s6 ;
      mode->first.K603_CTL = LR_BOT;    // 3V 40k.
      mode->first.K602_CTL = LR_BOT;    // intermediate shunts
      mode->first.K601_CTL = LR_BOT;    // low-current output.

      if(strcmp(s0, "1mA") == 0 )
        mode->second.U605 = S6;
      else if (strcmp(s0, "100uA") == 0 )
        mode->second.U605 = S5;
      else if (strcmp(s0, "10uA") == 0 )
        mode->second.U605 = S4;
      else if (strcmp(s0, "1uA") == 0 )
        mode->second.U605 = S3;
      else if (strcmp(s0, "100nA") == 0 )
        mode->second.U605 = S2;
      else if (strcmp(s0, "10nA") == 0 )
        mode->second.U605 = S1;
      else
        assert(0);
    }
    else if (strcmp(s0, "10mA") == 0 ) {
      // >   set k603 bot >  set k602 top >  set k601 top >   set u605 s8

      mode->first.K603_CTL = LR_BOT;    // 3V 40k.
      mode->first.K602_CTL = LR_TOP;    // relay driven shunt
      mode->first.K601_CTL = LR_TOP;    // hi-current output. - doesn't matter. if both the same.

      mode->second.U605 = S8;
    }
    else
      printf("bad dci-source arg\n");

    app_transition_state( app->spi, app->mode_current,  &app->system_millis );

    return 1;
  }


  else if( sscanf(cmd, "irange-gain %100s", s0) == 1) {

    // same argument but switch gain.
    //
    // eg. 100x for shunts.
    // except 10A.
    // 1x for TIA
  }

  else if( sscanf(cmd, "irange %100s", s0) == 1) {

    // change name dci-range.  no because should work for aci-range also.
    // we don't set amp gain here.  should we?  depending on the range?
    // doing acal we don't
    /*
      extr. a separaet command irange-gain .  and pass the same
    */

    Mode *mode = app->mode_current;

    // use shunts
    mode->first. K709_CTL  = LR_TOP;

    // rset.
    // 0.1R.
    mode->first. K703_CTL  = LR_BOT;
    // agn200 shunts are off.
    mode->first.K707_CTL  = LR_TOP;
    mode->first.K706_CTL  = LR_TOP;
    mode->first.K704_CTL  = LR_TOP;
    mode->first.K705_CTL  = LR_TOP;


    if(strcmp(s0, "10A") == 0 || strcmp(s0, "1A") == 0 || strcmp(s0, "100mA") == 0
        || strcmp(s0, "10mA") == 0 || strcmp(s0, "1mA") == 0 || strcmp(s0, "100uA") == 0) {

      // use shunts
      mode->first. K709_CTL  = LR_TOP;


      if(strcmp(s0, "10A") == 0 || strcmp(s0, "1A") == 0) {
        mode->first. K703_CTL  = LR_TOP;
        mode->second.U703 = S1;
      }
      else if(strcmp(s0, "100mA") == 0) {
        mode->first. K704_CTL  = LR_BOT;
        mode->second.U703 = S2;
      }
      else if(strcmp(s0, "10mA") == 0) {
        mode->first. K705_CTL  = LR_BOT;
        mode->second.U703 = S3;
      }
      else if(strcmp(s0, "1mA") == 0) {
        mode->first. K706_CTL  = LR_BOT;
        mode->second.U703 = S4;
      }
      else if(strcmp(s0, "100uA") == 0) {
        mode->first. K707_CTL  = LR_BOT;
        mode->second.U703 = S5;
      }
      else assert(0);

    } else {

      // use TIA
      mode->first. K709_CTL  = LR_BOT;

      // mux lo on shunt sence mux, to reduce leakage on himux
      mode->second.U703 = S8;

      if(strcmp(s0, "10uA") == 0)
        mode->second.U702 = W1;
      else if(strcmp(s0, "1uA") == 0)
        mode->second.U702 = W2;
      else if(strcmp(s0, "100nA") == 0)
        mode->second.U702 = W3;
       else if(strcmp(s0, "10nA") == 0)
        mode->second.U702 = W4;
      else {

        printf("bad irange arg\n");
        return 0;
      }
    }

    app_transition_state( app->spi, app->mode_current,  &app->system_millis );

    return 1;
  }


  else if(strcmp(cmd, "reset") == 0) {

    // turn off any concurrent test.
    app->test_in_progress = 0;

    // ok. this is working.
    printf("perform reset\n");

    // reset mode to initial
    *app->mode_current = *app->mode_initial;

    // do the state transition
    app_transition_state( app->spi, app->mode_current,  &app->system_millis );

    return 1;
  }



  else if( sscanf(cmd, "cols %lu", &u1 ) == 1) {

    if(u1 >= 2 && u1 <= 4) {
      printf("set cols %lu\n", u1 );
      app->model_cols = u1;
    }
    else
      printf("bad cols arg\n");
    return 1;
  }
  else if( strcmp( cmd, "cols?") == 0) {

    printf("cols %u\n", app->model_cols );
    return 1;
  }



  // temp show.
  else if(strcmp(cmd, "temp?") == 0) {  // mcu show.

    double val = adc_temp_read10();
    printf("temp %.1fC\n", val);

    return 1;
  }


  /*
      can we print the heap also?
  */
  else if(strcmp(cmd, "mem stack?") == 0) {

      printf("-------\n");
      print_stack_pointer();
      return 1;
  }

  else if(strcmp(cmd, "mem malloc?") == 0) {

      printf("-------\n");
      printf("malloc\n");
      malloc_stats();
      return 1;
  }

  else if(strcmp(cmd, "mem mesch?") == 0
        || strcmp(cmd, "mem matrix?") == 0
      ) {

      // Note that not all allocations are visible to mallinfo(); see BUGS and consider using malloc_info(3) instead.
      // mallinfo(stdout );
      //  The malloc_info() function is designed to address deficiencies in
      // malloc_stats(3) and mall

      printf("-------\n");
      printf("mesch\n");
      printf("mesch mem_info_is_on() %u\n", mem_info_is_on());
      mem_dump_list(stdout, 0 );
      return 1;
  }




  // issue is the accumulation relay on? accidently?

  /*
      a range is a type of mode.
      dcv10,dcv1,dcv01 are all the same except amplifier gain. so can consolidate

      -------
      use a float here.
  */

  else if( sscanf(cmd, "dcv %100s", s0) == 1) {

    /*
      It might be easier - to use the command buffer for these.
        eg. setting all the parameters with a cmd string.

    */

    // this is horrid state.
    // turn off any concurrent test.
    app->test_in_progress = 0;

    // derive new mode from initial .
    // this overrides nplc/aperture.
    *app->mode_current = *app->mode_initial;
    //  alias to ease syntax
    Mode *mode = app->mode_current;



    // set the ampliier gain.
    if( strcmp(s0, "10") == 0) {
        printf("whoot dcv10\n");
        mode->first. U506 =  U506_GAIN_1;    // amp feedback should never be turned off.
        mode->second.U506 =  U506_GAIN_1;
    }
    else if( strcmp(s0, "1") == 0) {
        printf("whoot dcv1\n");
        mode->first. U506 =  U506_GAIN_10;  // amp feedback should never be turned off.
        mode->second.U506 =  U506_GAIN_10;
    }
    else if( strcmp(s0, "01") == 0 ) {   // 100mV range
        printf("whoot dcv01\n");
        mode->first. U506 =  U506_GAIN_100;  // amp feedback should never be turned off.
        mode->second.U506 =  U506_GAIN_100;
    }
    else if( strcmp(s0, "1000") == 0) {
        printf("whoot dcv10\n");
        mode->first. U506 =  U506_GAIN_1;    // amp feedback should never be turned off.
        mode->second.U506 =  U506_GAIN_1;
    }
    else if( strcmp(s0, "100") == 0) {
        printf("whoot dcv100\n");
        mode->first. U506 =  U506_GAIN_10;  // amp feedback should never be turned off.
        mode->second.U506 =  U506_GAIN_10;
    }
    else {
        printf("bad range\n");
        return 1;
    }


    if( strcmp(s0, "10") == 0 ||  strcmp(s0, "1") == 0 ||  strcmp(s0, "01") == 0 ) {   // 100mV range

      // close/ turn on K405 relay.
      mode->first.K405_CTL  = LR_TOP;

      // follow persist_fixedz for 10Meg/high-z.
      mode->first.K402_CTL = app->persist_fixedz ?  LR_TOP :  LR_BOT ;

      // set the input muxing.
      mode->reg_direct.himux2 = S4 ;    // gnd to reduce leakage on himux
      mode->reg_direct.himux  = S7 ;    // dcv-in
    }


    // setup input relays for hv
    else if( strcmp(s0, "1000") == 0 || strcmp(s0, "100") == 0) {

      // close/ turn on K402 relay.
      mode->first.K402_CTL  = LR_TOP;

      // set the input muxing.
      mode->reg_direct.himux2 = S4 ;    // gnd to reduce leakage on himux
      mode->reg_direct.himux  = S3 ;    // dcv-div
    }
    else assert( 0);



    // TODO populate protection - and arm the fets also.

    /*
      - we want sample mode, aperture and persist_fixedz to persist.
      - when auto/man changing range.
      - but that means we would have to copy. from original.
    */

    ////////////
    // fpga/adc config

    if(mode->reg_mode != MODE_AZ
      && mode->reg_mode != MODE_NO_AZ
      /*&& mode->reg_mode != MODE_EM */) {

      // if no sample mode is set, then set one.
      // if mode is not set, then set to useful useful.
      mode->reg_mode = MODE_AZ;
    }


    if(mode->reg_mode == MODE_AZ) {

      // az controls the precharge switch.
      mode->reg_direct.azmux        = S6;    // lo
    }
    else if(mode->reg_mode == MODE_NO_AZ) {

      mode->reg_direct.sig_pc_sw_ctl  = SW_PC_SIGNAL;   // pc switch muxes signal.
      mode->reg_direct.azmux          = S1;             // azmux muxes pc-out
    }
/*
    else if(mode->reg_mode == MODE_EM) {

      mode->reg_direct.sig_pc_sw_ctl  = SW_PC_BOOT;     // pc switch muxes boot. to turn off signal to reduce leakage
      mode->reg_direct.azmux          = S2;             // azmux muxes boot directly ionpc-out
    }
*/
    else assert( 0);





    if(!mode->reg_adc_p_aperture) {

      // FIXME. this is reset on each dcv range range.
      // because mode is derived/copied from initial.
      // Ok. and easier for the moment.

      // if aperture not set, then set to useful default
      mode->reg_adc_p_aperture = nplc_to_aper_n( 1, app->lfreq ); // this is dynamic. maybe 50,60Hz. or other.
    }

    // do the state transition
    app_transition_state( app->spi, mode,  &app->system_millis );

    return 1;

  }

  ///////////////////////////////////////////////


  // flash erase
  else if(strcmp(cmd, "flash erase") == 0) {

    printf("flash erasing sector\n");
    usart1_flush();
    flash_erase_sector_();
    printf("done erase\n");
    return 1;
  }


  else if(sscanf(cmd, "flash cal save %lu", &u1 ) == 1) {

    if(!app->b) {
      printf("no cal!\n");
      return 1;
    }

    // now save to flash
    printf("flash unlock\n");
    flash_unlock();

    FILE *f = flash_open_file();
    file_blob_skip_end( f);
    // use callback to write the block.
    file_blob_write( f,  (void (*)(FILE *, Header *, void *)) my_file_cal_write, app->b );
    fclose(f);

    printf("flash lock\n");
    flash_lock();
    printf("done\n");
    return 1;
  }

  // change name load?
  else if(sscanf(cmd, "flash cal read %lu", &u1 ) == 1) {

    printf("flash unlock\n");
    flash_unlock();
    FILE *f = flash_open_file();
    file_blobs_scan( f,  (void (*)( FILE *, Header *, void *))  my_file_blobs_scan , app );
    fclose(f);

    printf("flash lock\n");
    flash_lock();
    printf("done\n");
    return 1;
  }

  // flash cal copy .  n  to n.   this would be useful. if want to replace a cal.
  // could also have a delete cal - that would invalidate.

  // flash cal list

  else {
    return 0;

  }
  // have to



  return 0;
}



