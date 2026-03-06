

#include <assert.h>
#include <stdio.h>
#include <math.h>   // fabs
#include <string.h>   // strcmp, memset


#include <util.h> // str_decode_uint



#include <mode.h>







static const _mode_t mode_initial =  {

  /*
    all relays need to be defined as b01 or b10.
    for default initialization
    otherwise they will not get an initial pulse/value.
  */

  .magic  = MODE_MAGIC,


  // U401
  .serial. K404  = SR_RESET,
  .serial. K403  = SR_RESET,
  .serial. K405  = SR_RESET,
  .serial. K406  = SR_RESET,

  // U402
  .serial. K407  = SR_RESET,
  .serial. K402  = SR_RESET,

  // u405
  .serial. K401  = SR_RESET,

  .serial. K701  = SR_RESET,
  .serial. K702  = SR_RESET,
  .serial. K703  = SR_RESET,

  ////////////

  // set lo-side drive of com-lc to A400-1/ star-ground
  .serial.U423   = D3,

  // set loside input boot buffer mux to A400-1/ star ground
  .serial.U426   = S4,


  ////////////

  // amplifier set fb 1x feb 2026.
  .serial .U506 = S8,


  // signal acquisition defaults
  .sa.p_clk_count_trig_delay  = CLK_FREQ * 100e-3,         // 100ms
  .sa.p_clk_count_precharge   = CLK_FREQ * 500e-6,          //  500us.

  /*
    .sa.p_seq_n = 2,
    .sa.p_seq0 = (0b01 << 4) | S3,         // dcv
    .sa.p_seq1 = (0b00 << 4) | S7,         // star-lo
    .sa.p_seq2 = 0,
    .sa.p_seq3 = 0,
  */


  .trigger_source = 1,   // set internal trigger active

  // default adc
  .adc.p_aperture     = CLK_FREQ * 0.2,             // 200ms. 10nplc 50Hz.  // Not. should use current calibration?  // should be authoritative source of state.
  .adc.p_reset        = CLK_FREQ * 500e-6,          // 500us.



  .reg_cr.mode = 0,

  // eg sigmux should be on during normal integration.
  .reg_cr.adc_p_active_sigmux  = true

};



void mode_init(_mode_t *mode)
{

  *mode = mode_initial;
}


void mode_reset(_mode_t *mode)
{
  // same as init
  mode_init( mode);
}




void _4094_state_clear_relays(_4094_state_t *state)
{


  // U401 conditioning
  state->K404  = SR_NONE;
  state->K403  = SR_NONE;
  state->K405  = SR_NONE;
  state->K406  = SR_NONE;


  // U402
  state->K407  = SR_NONE;
  state->K402  = SR_NONE;

  // u405
  state->K401 = SR_NONE;


  // u607
  state->K602  = SR_NONE;

  // u713 AMPS
  state->K701  = SR_NONE;
  state->K704  = SR_NONE;
  state->K707  = SR_NONE;

  // u705
  state->K702  = SR_NONE;
  state->K703  = SR_NONE;


  // u706
  state->K708  = SR_NONE;
  state->K705  = SR_NONE;
  state->K706  = SR_NONE;

}




void adc_aperture_set( adc_state_t *adc, uint32_t u)
// void mode_adc_aperture_set( _mode_t *mode, uint32_t u)
{
  adc->p_aperture = u;
}




void sa_trig_delay_set( sa_state_t *sa, uint32_t u)
// void sa_trig_delay_set( _mode_t *mode, uint32_t u)
{
  sa->p_clk_count_trig_delay = u;
}



void reg_cr_mode_set( reg_cr_t *reg_cr, unsigned u0)
// void mode_reg_cr_mode_set(_mode_t *mode, unsigned u0)
{

  // ease setting.
  // change name of access to mode_cr_mode_set() ... or similar...

  assert(u0 < 1<<3);

  reg_cr->mode = u0;
}











///////////////////////////







// actually may be better to have noaz. to set up. to run with p_seq_n = 1;
// and no switching.
/*
  could actually type this function in sa and  reg_direct.

*/

bool mode_az_set_relax(_mode_t *mode, const char *s)
{
  /* note the same syntax

      options here.   "ch1", "ch2", "ratio", "0".
      keep the az flag separate.
  */


  if(strcmp(s, "0") == 0 ) {

    // sample star-ground
    // use n=2 for good led activity indicator

    // signal can come in on S3, S7
    sa_state_t *sa = &mode->sa;
    sa->p_seq_n = 2;

    // zero serial
    sa->p_seq_elt[ 0].azmux  = S6;     // A400-1
    sa->p_seq_elt[ 0].pc = 0b00;

    // val
    sa->p_seq_elt[ 0].azmux  = S6;     // A400-1
    sa->p_seq_elt[ 0].pc = 0b00;

    // could set the catcher handler/closure here
  }

  else if(strcmp(s, "ch2") == 0 ) {

    // direct mode - keep
    mode->reg_direct.azmux_o = S3;
    mode->reg_direct.pc_o = 0b10;

    // az mode
    // signal can come in on S3, S7
    sa_state_t *sa = &mode->sa;
    sa->p_seq_n = 2;

    // zero
    sa->p_seq_elt[ 0].azmux  = S7;    // CH2-LO
    sa->p_seq_elt[ 0].pc = 0b00;

    // val
    sa->p_seq_elt[ 1].azmux  = S3;    // PC-CH2-OUT
    sa->p_seq_elt[ 1].pc = 0b10;      // pc2 active
  }

  else if(strcmp(s, "ch1") == 0 ) {

    // direct mode - keep
    mode->reg_direct.azmux_o = S1;
    mode->reg_direct.pc_o = 0b01;

    // az mode
    // signal can come in on S3, S7
    sa_state_t *sa = &mode->sa;
    sa->p_seq_n = 2;

    // zero
    sa->p_seq_elt[ 0].azmux  = S5;    // COM-LC
    sa->p_seq_elt[ 0].pc = 0b00;

    // val
    sa->p_seq_elt[ 1].azmux  = S1;    // PC-CH1-OUT
    sa->p_seq_elt[ 1].pc = 0b01;      // pc1 active
  }



  else if(strcmp(s, "ratio") == 0 ) {
    assert( 0);
  }
  else {
    return 0;
  }

  return 1;

}


void mode_az_set(_mode_t *mode, const char *s)
{
  bool ret = mode_az_set_relax( mode, s);
  assert( ret);
}




void mode_gain_set( _mode_t *mode, uint32_t u)
{

  printf("set amp gain\n");

  switch(u) {

    case 1:
      mode->serial .U506 = S8;
      mode->serial.U506 = S8;
      break;

    case 10:
      mode->serial. U506 = S2;
      mode->serial.U506 = S2;
      break;

    case 100:
      mode->serial. U506 = S3;
      mode->serial.U506 = S3;
      break;

    case 1000:
      mode->serial. U506 = S4;
      mode->serial.U506 = S4;
      break;

    default:
      assert(0);
  }

}








static void mode_lts_reset( _mode_t *mode )
{
  // mux agnd, instead of off. to reduce input leakage on mux followers.
  mode->serial.U1012  = S8 ;
  mode->serial.U1003  = S8 ;


}



void mode_lts_source_set( _mode_t *mode, double f0)
{

  printf("set lts\n");

  mode_lts_reset( mode);

  if(f0 >= 0) {
    printf("with +");
    mode->serial.U1003  = S1 ;       // positive source.
  } else if (f0 < 0) {

    printf("with -");
    mode->serial.U1003  = S2 ;      // negatie source
  }


  if( fabs(f0)  == 10) {
    printf("10V\n");
    mode->serial.U1012  = S1;         // 10V tap
  }
  else if(fabs(f0) == 1) {
    printf("1V\n");
    mode->serial.U1012  = S2;       // 1V.
  }
  else if(fabs(f0) == 0.1) {
    printf("0.1V\n");
    mode->serial.U1012  = S3;       // 0.1V.
  }
  else if(fabs(f0) == 0.01) {
    // not implemented/resistor not poplated
    printf("0.01V\n");
    mode->serial.U1012  = S4;       // 0.01V.
  }
/*
  note. makes no sense to source a zero here

*/
  else {
    // TODO argument validation
    // when called programmatically, should not fail.

    assert(0);
  }
}


void mode_daq_set( _mode_t *mode, unsigned u0, unsigned u1 )
{
  // mode_lts_reset( mode);

  // mode->serial.U1006  = S7;    // JA
  // mode->serial.U1007  = S7;

  // OK. we have to pass encoded arguments here.

  // set the hig/lo dac inputs.
  mode->serial.U1009  = u0;
  mode->serial.U1010  = u1;




}




void mode_invert_dac_set( _mode_t *mode, unsigned u0 )
{
  // invert dac
  printf("invert mdac\n");

  // dac7811.
  // 12 bits -  mask is FFF  == 4095
  // 0001 command to load and update.

  assert( u0 <= 0xfff);

  // move this to where dac is written. or insider the peripheral
  uint8_t cmd = 0x01;
  mode->mdac0_val = (cmd <<12) | (u0 & 0xfff);
}


void mode_sts_dac_set( _mode_t *mode, unsigned u0 )
{
  // sts dac
  printf("sts mdac\n");

  // dac7811.
  // 12 bits -  mask is FFF  == 4095
  // 0001 command to load and update.

  assert( u0 <= 0xfff);

  // move this to where dac is written. or insider the peripheral
  uint8_t cmd = 0x01;
  mode->mdac1_val = (cmd <<12) | (u0 & 0xfff);

}





static bool mode_loside_set( _mode_t *mode, const char *s)
{
  /*
    TODO add extra argument for the input.
      decode two arguments here. for boot channel U426.
      and output drive U423

  */

  // TODO need argument for the input channel to use
  // set to use channel 2.

  /*
    set boot for ch2. boot.
  */
  mode->serial.U426    = D2;      // ch 2  BOOT

  if(strcmp(s, "gnd") == 0
    || strcmp(s, "star") == 0) {

    // consider setting as default in the main.c mode initialization
    // could turn off the boot here.
    mode->serial.U423 = D3;      // drive com-lc with A400-1. star-ground

    uint16_t  val = 0x0;          // turn off
    mode_invert_dac_set( mode, val);
  }
  else if(strcmp(s, "invert") == 0
    || strcmp(s, "inverter") == 0) {

    /*
      should only really set boot.  if using the invert
    */

    mode->serial.U423 = D1;      // drive com-lc from mdac output

    uint16_t  val = 0xfff;        // full.
    mode_invert_dac_set( mode, val);
  }
  else if(strcmp(s, "divider") == 0
    || strcmp(s, "offset") == 0) {   // invert + divider.... for dither

    // invert but using the divider.
    mode->serial.U423 = D2;      // drive com-lc from mdac and divider

    uint16_t  val = 0x0;          // off
    mode_invert_dac_set( mode, val);
  }
  else if(strcmp(s, "boot") == 0) {

    mode->serial.U423 = D4;      // drive com-lc with boot direct
    uint16_t  val = 0x0;          // off
    mode_invert_dac_set( mode, val);
  }
  else {

    printf("bad lo-side argument\n");
    return 0;
  }

  return 1;
}

/*
- I think we can simplify the way we manage the serial/ serial.

  just make a tmp copy of the serial.
  write and wait 10ms.

  then clear the relay pins  of the copy
  and write

  then we do not have to keep

*/



void mode_ch1_reset(_mode_t *mode)      // change name reset() ?
{

  mode->serial.K402 = SR_RESET;    // input off
  mode->serial.K406 = SR_RESET;    // accum ch1 off
  mode->serial.K405 = SR_RESET;    // accum ch2 off


  mode->serial.K407 = SR_RESET;    // dcv-source off

  mode->serial.K401 = SR_RESET;    // ohms off
  mode->serial.K404 = SR_RESET;    // lts-source off
  mode->serial.K403 = SR_RESET;    // 10Meg impedance off
}


void mode_ch1_set_dcv(_mode_t *mode)
{
  mode_ch1_reset( mode);
  mode->serial.K402 = SR_SET;      // input on
}

void mode_ch1_set_dcv_source(_mode_t *mode)   // rename use K404. instead.
{
  mode_ch1_reset( mode);
  mode->serial.K407 = SR_SET;      // dcv-source on
}






///////

/*
// The name dcv-source name scheme. is not quite right.
// it is all the ch2 inpputs. that are possible.
// i thinkk should set the feeder mux also.
// rather than ch1. and ch2.

// set ch2 temp.
// set ch2 ref
// set ch2 lts
// set ch2 sense
// set ch2 hv-div

it works well to coordinate the three input muxes together like this.

*/

/*
  Using string switch like - could simplify all this string handling ...

*/


void mode_ch2_reset(_mode_t *mode)
{
  // mode->serial.K405 = SR_RESET;    // accum ch2 off

  // inpput muxes.
  mode->serial.U419 = SOFF;     // himux or should be ref-lo? or agnd - for leakage?
  mode->serial.U420 = SOFF;     // lomux

  mode->serial.U409 = D4;   // feedmux  hi/lo
}



bool mode_ch2_set_relax( _mode_t *mode, const char *s0)
{

    if(strcmp(s0, "off") == 0 || strcmp(s0, "reset") == 0) {      // reset

      mode_ch2_reset(mode);
    }

    else if(strcmp(s0, "ref") == 0
		|| strcmp(s0, "ref-hi") == 0) {

      mode_ch2_reset(mode);

      mode->serial.U419 = S4;   // REF-HI
      mode->serial.U420 = S7;   // REF-LO
      mode->serial.U409 = D4;   // feedmux  hi/lo
    }
    else if(strcmp(s0, "ref-lo") == 0
		/*||  strcmp(s0, "ref_lo") == 0*/) {

      mode_ch2_reset(mode);

      mode->serial.U419 = S3;   // REF-LO
      mode->serial.U420 = S7;   // REF-LO
      mode->serial.U409 = D4;   // feedmux  hi/lo
    }
    else if(strcmp(s0, "temp") == 0) {

      mode_ch2_reset(mode);

      mode->serial.U419 = S1;   // TEMP
      mode->serial.U420 = S1;   // A400-1
      mode->serial.U409 = D4;   // feedmux  hi/lo
    }
    else if(strcmp(s0, "lts") == 0) {

      mode_ch2_reset(mode);

      mode->serial.U419 = S2;   // lts-source-hi
      mode->serial.U420 = S1;   // A400-1
      mode->serial.U409 = D4;   // feedmux
    }
    else if(strcmp(s0, "daq") == 0) {

      mode_ch2_reset(mode);

      mode->serial.U419 = S5;   // daq
      mode->serial.U420 = S5;   // com-lc
      mode->serial.U409 = D4;   // feedmux  hi/lo
    }
    else if(strcmp(s0, "shunts") == 0) {

      mode_ch2_reset(mode);

      mode->serial.U419 = S6;   // shunts hi
      mode->serial.U420 = S6;   // shunts lo
      mode->serial.U409 = D4;   // feedmux  hi/lo
    }
    else if(strcmp(s0, "tia") == 0) {

      mode_ch2_reset(mode);

      mode->serial.U419 = S8;   // tia
      mode->serial.U420 = S1;   // A400-1
      mode->serial.U409 = D4;   // feedmux  hi/lo
    }
    else if(strcmp(s0, "sense") == 0) {

      mode_ch2_reset(mode);

      mode->serial.U409 = D1;         // sense hi and lo
    }

    else if(strcmp(s0, "dcv-div") == 0) {

      // TODO consider change name - hv-div. or just 'div'
        mode_ch2_reset(mode);
        mode->serial.U409 = D3;         // dcv div
    }
    else {

      return 0;
    }

  return 1;
}



void mode_ch2_set( _mode_t *mode, const char *s0)
{
  bool ret = mode_ch2_set_relax( mode, s0);
  assert( ret);
}






void mode_ch1_accum( _mode_t *mode, bool val)
{
  mode->serial.K406 = val ? SR_SET : SR_RESET;
}


void mode_ch2_accum( _mode_t *mode, bool val)
{
  mode->serial.K406 = val ? SR_SET : SR_RESET;

}



// bool mode_repl_statement( _mode_t *mode,  ranges_t *ranges, const char *cmd, const uint32_t line_freq )

bool mode_repl_statement(
  _mode_t     *mode,
  const char  *cmd,
  const uint32_t line_freq
)

{

  char s0[ 100 + 1];
  char s1[ 100 + 1];
  // char s2[100 + 1 ];
  uint32_t u0, u1;
  double f0;
  // int32_t i0;

  /*

      TODO For case handling. just convert the entire command to lower case.
      except we dont really have a buffer

  */




  if(strcmp(cmd, "reset") == 0) {

    // reset the mode.
    mode_reset( mode);
  }




  /*
      we have to disambiguate values with float args explicitly...
      because float looks like int
  */

  else if( sscanf(cmd, "aper %100s", s0) == 1
    && str_decode_float( s0, &f0))
  {

    // printf("set aperture\n");
    uint32_t aperture = period_to_aper_n( f0 );
    // assert(u1 == 1 || u1 == 10 || u1 == 100 || u1 == 1000); // not really necessary. just avoid mistakes
    aper_cc_print( aperture,  line_freq);
    mode->adc.p_aperture = aperture;
  }


  else if( sscanf(cmd, "nplc %100s", s0) == 1
    && str_decode_float( s0, &f0))
  {
    // use float here, to express sub 1nplc periods
    if( ! nplc_valid( f0 ))  {
        printf("bad nplc arg\n");
        // return 1;
    } else {

      // should be called cc_aperture or similar.
      uint32_t aperture = nplc_to_aperture( f0, line_freq );

      aper_cc_print( aperture,  line_freq);

      mode->adc.p_aperture = aperture;
    }
  }


  else if((sscanf(cmd, "precharge %100s", s0) == 1)
    && str_decode_uint( s0, &u0))  {

    // TODO consider use time, rather than raw count
    mode->sa.p_clk_count_precharge = u0;
  }

  else if((sscanf(cmd, "trig delay %100s", s0) == 1)
    && str_decode_uint( s0, &u0))  {

    // TODO consider use time, rather than raw count
    mode->sa.p_clk_count_trig_delay = u0;
  }



  // need to work out if keep the set...


  else if((sscanf(cmd, "set gain %100s", s0) == 1)
    && str_decode_uint( s0, &u0))  {


    mode_gain_set( mode, u0 );
  }




  // channel set
  else if( sscanf(cmd, "set ch1 %100s", s0) == 1)
  {
    if(strcmp(s0, "off") == 0 || strcmp(s0, "reset") == 0) {
      mode_ch1_reset(mode);
    }
    else if(strcmp(s0, "dcv") == 0) {
      mode_ch1_set_dcv(mode);
    }
    else if(strcmp(s0, "dcv-source") == 0) {
      mode_ch1_set_dcv_source(mode);
    }
    else assert(0);
  }



  else if( sscanf(cmd, "set lts %lf", &f0) == 1) {

    mode_lts_source_set( mode, f0);
  }




  else if( sscanf(cmd, "set daq %100s %100s", s0, s1 ) == 2
    && str_decode_uint( s0, &u0)
    && str_decode_uint( s1, &u1)
  )  {

    // eg. 'dcv-source daq s1 s2'
    // mode_ch2_set_daq( mode, u0, u1);
    mode_daq_set( mode, u0, u1);
  }



  // TODO consider better name than iso. eg. just sts,  or floating sts. flt-sts-dcv-source
  // consider remove prefix m. from the dac.

  // inverter dac
  else if(( sscanf(cmd, "set invert %100s", s0) == 1
    || sscanf(cmd, "set mdac0 %100s", s0) == 1)    // doesn't work?

    && str_decode_uint( s0, &u0)
  )  {

    if( u0 <= 0xfff)
      mode_invert_dac_set( mode, u0);
    else
      printf("arg out of range\n");

  }

  // iso dac
  else if(( sscanf(cmd, "set sts %100s", s0) == 1
    || sscanf(cmd, "set mdac1 %100s", s0) == 1 )
    && str_decode_uint( s0, &u0)
  )  {

    if( u0 <= 0xfff)
      mode_sts_dac_set( mode, u0);
    else
      printf("arg out of range\n");
  }




#if 0
  else if( sscanf(cmd, "dcv-source sts %100s", s0) == 1
    && str_decode_int( s0, &i0)) {

    // hex values are not working.
    printf("value %ld hex %lx\n", i0, i0 );

      // this isnt quite working.
    // note. can take a negative value.
    // eg. 0x3fff or -0x3fff
    mode_ch2_set_sts( mode, i0);
  }

#endif




  else if( sscanf(cmd, "set loside %100s", s0) == 1
  )  {

    bool ret = mode_loside_set( mode, s0);
    if(!ret) {
      printf("bad argument\n");
    }
  }

  else if( sscanf(cmd, "set az %100s", s0) == 1
    /* || sscanf(cmd, "set sa %100s", s0) == 1 */)  {

    bool ret = mode_az_set_relax( mode, s0 );
    if(!ret) {
      printf("bad argument\n");
    }

  }






  else if( sscanf(cmd, "set ch2 %100s", s0) == 1)
  {
    // why not manage this argument passing
    // in the handler ???
    // because issue with repl validation/error handling.


    bool ret = mode_ch2_set_relax( mode, s0);

    if(!ret) {
      printf("unrecognized\n");
      assert(0);
      return 0;
    }

  }


#if 0
    if(strcmp(s0, "off") == 0 || strcmp(s0, "reset") == 0) {      // reset
      mode_ch2_reset(mode);
    }

    else if(strcmp(s0, "ref") == 0
		|| strcmp(s0, "ref-hi") == 0) {

      mode_ch2_set_ref( mode);
    }
    else if(strcmp(s0, "ref-lo") == 0
		||  strcmp(s0, "ref_lo") == 0) {
      mode_ch2_set_ref_lo( mode);
    }
    else if(strcmp(s0, "temp") == 0) {
      mode_ch2_set_temp( mode);
    }
    else if(strcmp(s0, "lts") == 0) {
      mode_ch2_set_lts( mode);
    }
    else if(strcmp(s0, "daq") == 0) {
      mode_ch2_set_daq( mode);
    }
    else if(strcmp(s0, "shunts") == 0) {
      mode_ch2_set_shunts(mode);
    }
    else if(strcmp(s0, "tia") == 0) {
      mode_ch2_set_tia( mode );
    }
    else if(strcmp(s0, "sense") == 0) {
      mode_ch2_set_sense(mode);
    }
    else if(strcmp(s0, "dcv-div") == 0) {
      mode_ch2_set_dcv_div(mode);
    }
    else {
      printf("unrecognized\n");
      return 0;
      // assert(0);
    }

#endif



  // two val set cmd
  else if( sscanf(cmd, "set %100s %100s", s0, s1) == 2
    && str_decode_uint( s1, &u0)
  ) {

      // printf("set %s %lu\n", s0, u0);

      // cannot manage pointer to bitfield. so have to hardcode.

/*
      if(strcmp(s0, "mdac1") == 0) {
        mode->mdac1_val = u0;
      }
*/



     if(strcmp(s0, "pc") == 0) {
        mode->reg_direct.pc_o = u0;
      }


      else if(strcmp(s0, "azmux") == 0) {
        mode->reg_direct.azmux_o = u0;

      }

    //////////////


      // better name. _count.
      else if(strcmp(s0, "seqn") == 0) {
        mode->sa.p_seq_n = u0;
      }


      // fpga0 mode.
      else if(strcmp(s0, "mode") == 0) {

        reg_cr_mode_set( &mode->reg_cr, u0);
      }


      else if(strcmp(s0, "noaz") == 0) {

        // this will be over-written by a range. reset...
        mode->reg_cr.sa_p_noaz = u0;
      }




      else if(strcmp(s0, "direct") == 0) {
        assert(sizeof(mode->reg_direct) == 4);
        assert(sizeof(u0) == 4);
        memcpy( &mode->reg_direct, &u0, sizeof(mode->reg_direct));
      }
      // set red_direct via bitfield arguments, nice.
      else if(strcmp(s0, "leds") == 0) {
        mode->reg_direct.leds_o = u0;
      }
      // by field
      else if(strcmp(s0, "monitor") == 0) {
        mode->reg_direct.monitor_o = u0;
      }






      else if(strcmp(s0, "adc_refmux") == 0) {
        mode->reg_direct.adc_refmux_o = u0;
      }
      else if(strcmp(s0, "adc_cmpr_latch") == 0) {
        mode->reg_direct.adc_cmpr_latch_o = u0;
      }
      else if(strcmp(s0, "spi_interrupt_ctl") == 0) {
        mode->reg_direct.spi_interrupt_ctl_o = u0;
      }
      else if(strcmp(s0, "meas_complete") == 0) {
        mode->reg_direct.meas_complete_o = u0;
      }

      ////////////////////////////////////////////
      // 4094 components.
      // perhaps rename serial. _4094_serial etc.

/*
      else if(strcmp(s0, "u504") == 0) {
        mode->serial.U504 = u0;
      }

*/
      else if(strcmp(s0, "u1003") == 0) {
        mode->serial.U1003 = u0;
      }
/*      else if(strcmp(s0, "u1006") == 0) {
        mode->serial.U1006 = u0;
      }
*/
      else if(strcmp(s0, "u1012") == 0) {
        mode->serial.U1012 = u0;
      }

      else if(strcmp(s0, "u1009") == 0) {
        mode->serial.U1009 = u0;
      }
      else if(strcmp(s0, "u1010") == 0) {
        mode->serial.U1010 = u0;
      }


#if 0
      else if( strcmp(s0, "dac") == 0 || strcmp(s0, "u1016") == 0 || strcmp(s0, "u1014") == 0) {
        // let the mode update - determine setting up spi params.
        mode->dac_val = u0;
      }
#endif
      /*
          handle latch relay pulse encoding here, rather than at str_decode_uint() time.
          valid values are 1 (0b01)  and 2 (0b10). not 1/0.
          reset is default schem contact position.
      */

      else if(strcmp(s0, "u409") == 0 || strcmp(s0, "feedmux") == 0) {

        // should use set channel functionality instead.
        mode->serial.U409 = u0 ;
      }

      else if(strcmp(s0, "u423") == 0) {
        mode->serial.U423 = u0 ;
      }
      else if(strcmp(s0, "u426") == 0) {
        mode->serial.U426 = u0 ;
      }




      else if(strcmp(s0, "k401") == 0) {
        mode->serial.K401 = u0 ? SR_SET: SR_RESET;
      }
      else if(strcmp(s0, "k402") == 0) {
        mode->serial.K402 = u0 ? SR_SET: SR_RESET;
      }
      else if(strcmp(s0, "k403") == 0) {
        mode->serial.K403 = u0 ? SR_SET: SR_RESET;
      }
      else if(strcmp(s0, "k404") == 0) {
        mode->serial.K404 = u0 ? SR_SET: SR_RESET;
      }
      else if(strcmp(s0, "k405") == 0) {
        mode->serial.K405 = u0 ? SR_SET: SR_RESET;
      }
      else if(strcmp(s0, "k406") == 0) {
        mode->serial.K406 = u0 ? SR_SET: SR_RESET;
      }
      else if(strcmp(s0, "k407") == 0) {
        mode->serial.K407 = u0 ? SR_SET: SR_RESET ;      // 0 == reset
      }


      else if(strcmp(s0, "k701") == 0) {
        mode->serial.K701 = u0 ? SR_SET: SR_RESET;
      }
      else if(strcmp(s0, "k702") == 0) {
        mode->serial.K702 = u0 ? SR_SET: SR_RESET;
      }
      else if(strcmp(s0, "k703") == 0) {
        mode->serial.K703 = u0 ? SR_SET: SR_RESET;
      }
      else if(strcmp(s0, "k704") == 0) {
        mode->serial.K704 = u0 ? SR_SET: SR_RESET;
      }
      else if(strcmp(s0, "k705") == 0) {
        mode->serial.K705 = u0 ? SR_SET: SR_RESET;
      }







#if 0
  /*
    perhaps keep the 'set' prefix to clearly disambiguate these actions under common syntactic form.
  */

  // three val
  else if( sscanf(cmd, "set %100s %100s %100s", s0, s1, s2) == 3
    && str_decode_uint( s1, &u0)
    && str_decode_uint( s2, &u1)
  ) {
      /*
        setting/encoding sequence values directly.
        eg.
        > set seq0 0b01 s3
        > set seq0 0b00 soff
      */

      // maybe be handy to have in a function. or else return
      uint32_t val =  ((u0 & 0b11) << 4) | ( u1 & 0b1111);

      if(strcmp(s0, "seq0") == 0) {
        mode->sa.p_seq0 = val;
      }
      else if(strcmp(s0, "seq1") == 0) {
        mode->sa.p_seq1 = val;
      }
       else if(strcmp(s0, "seq2") == 0) {
        mode->sa.p_seq2 = val;
      }
      else if(strcmp(s0, "seq3") == 0) {
        mode->sa.p_seq3 = val;
      }
      else {
        printf("unknown target %s for 3 var set\n", s0);
        return 0;
      }

  }

#endif


      /*
        not completely clear if trig should be out-of-band. eg not put in the mode structure.
      */
/*
      else if(strcmp(s0, "trig") == 0) {
        // should move/place in signal acquisition?
        mode->trig_sa = u0;
      }
*/

      else {

        printf("unknown target %s for 2 var set\n", s0);
        return 0;

      }
  }



  else {

    return 0;
  }



  return 1;
}












#if 0



static void mode_ch2_set_iso( _mode_t *mode, signed u0 )
{
  /* this function is possible. but it is a bit confusing parallel state.
    because output does not appear
    and it is read from from the daq.
    so should probably be handled exceptionally

    and would need a call to the daq.
  - void mode_ch2_set_daq( _mode_t *mode, unsigned u0, unsigned u1 )
  */

  UNUSED(mode);
  UNUSED(u0);
  assert(0);
}



// remove argument handling is messy here.
void mode_ch2_set_ref( _mode_t *mode )
{
  // rename mode_dcv_ref_source

  mode_lts_reset( mode);

  if(u0 == 7) {
    printf("with ref-hi +7V\n");
    mode->serial.U1006  = S4;       // ref-hi
    // mode->serial.U1007  = S4;       // ref-lo
  }
  else if( u0 == 0 ) {
    // need bodge for this
    printf("with ref-lo\n");
    mode->serial.U1006  = S8;       // ref-lo
    // mode->serial.U1007  = S4;       // ref-lo - looks funny. gives bad measurement. on DMM.
  }
  else
    assert(0);
}




// this is a poor abstraction.
void mode_ch2_set_channel( _mode_t *mode, unsigned u0 )
{

  // neither channel
  mode->serial.K407 = SR_RESET;
  mode->serial.U409 = DOFF;       // hi/lo mux.

  if(u0 == 1) {

    mode->serial.K407 = SR_SET;
  } else if(u0 == 2) {

    mode->serial.U409 = D4;
  }

}

#endif





/*
  TODO.
  this function is awful.

  instead just set up a closure/handler - at the time we set the AZ mux sequencing.

  instead code as,

     set_seq( idx, pc, azmux );
     set_seq_n();

  also the catcher/interpreter of the result.
  remember it's main use used can also

  just encode by hand. as we setup modes at top level.

*/

  // anything on channel two.

/*
  // zero serial
  sa->p_seq_elt[ 0].azmux  = S6;     // A400-1
  sa->p_seq_elt[ 0].pc = 0b00;

  // val
  sa->p_seq_elt[ 1].azmux  = S3;     // CH2-IN
  sa->p_seq_elt[ 1].pc = 0b10;


  // could set the LS drive. serial input switch here - eg. BOOT1/BOOT2/ agnd. if wanted.
  // so it is set up for the double range.
  // set the data catcher closurec.
*/

#if 0

void mode_seq_set( _mode_t *mode, uint32_t seq_mode , uint8_t arg0, uint8_t arg1 )
{
  /*
    doesn't have to be exhausive wrt cases.
    can still setup manually.
  */
/*
    we could define these in define int. also.
    so that the strings would be correctly decoded.

    S3 - dcv
    S7 - star-lo
    S1 - himux
    S8 - lomux
*/

  mode->reg_seq_mode = seq_mode;                 // to guide decoder

  switch(seq_mode) {


    // boot mode - might be particularly useful when sampling.

    case SEQ_MODE_BOOT: {
      // sample a hi, but don't switch the pc switch, generally only used for electrometer, very high input impedance.

      mode->sa.p_seq_n = 1;
      if(arg0 == S3 ) {
        mode->sa.p_seq0 = (0b00 << 4) | S3;        // dcv
      }
      else if(arg0 == S1 ) {
        mode->sa.p_seq0 = (0b00 << 4) | S1;        // himux
      }
      else assert(0);
      break;
    }


    /*/ for noaz.
    // if we were to slect a lo here...
    // if it's a hi - then switch the PC - for symmetry. if lo. then don't bother.
    */

    case SEQ_MODE_NOAZ: {
      // clearer - to express as another mode, rather than as a bool.
      // azero off - just means swtich the pc for symmetry/ and keep charge-injetion the same with azero mode.

      mode->sa.p_seq_n = 1;
      if(arg0 == S3 ) {
        mode->sa.p_seq0 = (0b01 << 4) | S3;        // dcv
      }
      else if(arg0 == S1 ) {
        mode->sa.p_seq0 = (0b10 << 4) | S1;        // himux
      }
      else if(arg0 == S7 ) {
        mode->sa.p_seq0 = (0b00 << 4) | S7;        // star-lo
      }
      else if(arg0 == S8 ) {
        mode->sa.p_seq0 = (0b00 << 4) | S8;        // lomux
      }

      else assert(0);
      break;
    }



    case SEQ_MODE_AZ: {
    // write the seq

      mode->sa.p_seq_n = 2;

      // hi goes serial

      if(arg0 == S3 )
        mode->sa.p_seq0 = (0b01 << 4) | S3;      // dcv
      else if(arg0 == S1 )
        mode->sa.p_seq0 = (0b10 << 4) | S1;        // himux
      else
        assert(0);

      if(arg1 == S7)
        mode->sa.p_seq1 = (0b00 << 4) | S7;        // star-lo
      else if(arg1 == S8)
        mode->sa.p_seq1 = (0b00 << 4) | S8;        // lomux
      else
        assert(0);
/*
    // applies both chanels.
      if(arg0 == S3 && arg1 == S7) {
        mode->sa.p_seq0 = (0b01 << 4) | S3;        // dcv
        mode->sa.p_seq1 = (0b00 << 4) | S7;        // star-lo
      }
      else if(arg0 == S1 && arg1 == S8)  {
        mode->sa.p_seq0 = (0b01 << 4) | S1;        // himux   WRONG. FIXME.   not switching the PC.
        mode->sa.p_seq1 = (0b00 << 4) | S8;        // lomux
      }
      else if(arg0 == S3 && arg1 == S8 )  {                // eg. for ref.
        mode->sa.p_seq0 = (0b01 << 4) | S3;        // dcv
        mode->sa.p_seq1 = (0b00 << 4) | S8;        // lomux
      }
      else assert(0);
*/
      break;
    }

/*
    case SEQ_MODE_ELECTRO: {

      // same as no az, except don't switch the precharge
      mode->sa.p_seq_n = 1;
      mode->sa.p_seq0 = (0b00 << 4) | S3;        // dcv
      break;
    }
*/

    case SEQ_MODE_AG:
    case SEQ_MODE_RATIO: {
      // 4 cycle, producing single output
      // Issue - is for internal - we need to set the common lo. eg. ref-lo. or start

      mode->sa.p_seq_n = 4;
      mode->sa.p_seq0 = (0b01 << 4) | S3;        // dcv
      mode->sa.p_seq1 = (0b00 << 4) | S8;        // ref-lo // star-lo
      mode->sa.p_seq2 = (0b10 << 4) | S1;        // himux
      mode->sa.p_seq3 = (0b00 << 4) | S8;        // ref-lo /// lomux
      break;
    }

/*
    case SEQ_MODE_AG: {
      // auto-gain 4 cycle - same as ratio. producing a single output


      mode->sa.p_seq_n = 4;
      // sample
      mode->sa.p_seq0 = (0b01 << 4) | S3;        // dcv
      mode->sa.p_seq1 = (0b00 << 4) | S7;        // star-lo
      // reference
      mode->sa.p_seq2 = (0b10 << 4) | S1;        // himux
      mode->sa.p_seq3 = (0b00 << 4) | S7;        // lomux
      break;
    }
*/
    case SEQ_MODE_DIFF: {
      // 2 cycle, hi- hi2, with both precharge switches switches. single output.
      mode->sa.p_seq_n = 2;
      mode->sa.p_seq0 = (0b01 << 4) | S3;        // dcv
      mode->sa.p_seq2 = (0b01 << 4) | S1;        // himux
      break;
    }

    case SEQ_MODE_SUM_DELTA: {    // change name.  SUM_DELTA. 0w

      // similar. take hi/lo, hi2/lo, .  but where lo is shared. so can calculate hi-lo, hi2-lo, hi-hi2.
      // advantage of a single sequence - is that flicker noise should cancel some.
      // noting that input can be external terminals - or the dcv-source and its inverted output.
      // to encodekkkkkkkkk
      // can do as 3 values or 4 values.   3 is more logical.

      mode->sa.p_seq_n = 3;
      mode->sa.p_seq0 = (0b01 << 4) | S3;        // dcv
      mode->sa.p_seq1 = (0b00 << 4) | S7;        // star-lo
      mode->sa.p_seq2 = (0b10 << 4) | S1;        // himux
      break;
    }

    default:
      assert( 0);
  }


}

#endif




#if 0

  //  maybe make explicit all values  U408_SW_CTL. at least for the initial mode, from which others derive.

  .serial .U408_SW_CTL = 0,      // b2b fets/ input protection off/open
  .serial.U408_SW_CTL = 0,

  // AMP FEEDBACK SHOULD NEVER BE TURNED OFF.
  // else draws current, and has risk damaging parts. mux pin 1. of adg. to put main amplifier in buffer/G=1 configuration.
  .serial. U506 =  D1,     // should always be on
  .serial.U506 =  D1,           // amplifier should always be on.

  .serial. K603_CTL  = SR_RESET,     // ohms relay off.


  /////////////////////////
  // 700
  // has inverting cmos buffer
  .serial. K702_CTL  = SR_RESET,
  .serial.K702_CTL  = 0b11,

  // 0.1R shunt off. has inverting cmos buffer
  .serial. K703_CTL  = SR_RESET,
  .serial.K703_CTL  = 0b11,

  // shunts / TIA - default to shunts
  .serial. K709_CTL  = SR_SET,

  // agn200 shunts are off.
  .serial. K707_CTL  = SR_SET,
  .serial. K706_CTL  = SR_SET,
  .serial. K704_CTL  = SR_SET,
  .serial. K705_CTL  = SR_SET,


#endif




#if 0
  // mode_lts_reset( mode);

  // mode->serial.U1006  = S3;       // JA mux dac

  if(u0 >= 0) {
    printf("positive");
    mode->serial.U1003  = S2;       // positive source.
  } else if (u0 < 0) {

    printf("neg");
    mode->serial.U1003  = S1;      // negatie source
  }

#endif









#if 0
  else if( sscanf(cmd, "dcv-source chan %lu", &u0) == 1)  {

    //
    //  set channel.  1 on.  2. on 1 off.
    // kind of need off and on.?
    // eg. 'dcv-source daq s1 s2'

    mode_ch2_set_channel( mode, u0);
  }

#endif



#if 0

  else if( sscanf(cmd, "boot%100s", s0) == 1
    && str_decode_uint( s0, &u0))  {

    mode_seq_set( mode, SEQ_MODE_BOOT, u0, 0 );
  }
  else if( sscanf(cmd, "noazero %100s", s0) == 1
    && str_decode_uint( s0, &u0))  {

    mode_seq_set( mode, SEQ_MODE_NOAZ, u0, 0 );
  }
  else if( sscanf(cmd, "azero %100s %100s", s0, s1) == 2
    && str_decode_uint( s0, &u0)
    && str_decode_uint( s1, &u1)) {

    mode_seq_set( mode, SEQ_MODE_AZ, u0, u1 );
  }

    // ratio is hardcoded to use lomux at the moment. and not star-lo.
  else if(strcmp(cmd, "ratio") == 0) {
    mode_seq_set( mode, SEQ_MODE_RATIO, 0,0 );
  }

  else if(strcmp(cmd, "ag") == 0)
    mode_seq_set( mode, SEQ_MODE_AG, 0, 0 );

  else if(strcmp(cmd, "diff") == 0)
    mode_seq_set( mode, SEQ_MODE_DIFF, 0 , 0);

  else if(strcmp(cmd, "sum-test") == 0)
    mode_seq_set( mode, SEQ_MODE_SUM_DELTA, 0, 0 );

#endif










#if 0
  spi_port_configure( spi_4094);
  spi_4094_write_n( spi_4094, (void *) &x , 4  );
/*
  uint32_t x = 0xffffffff;
  uint32_t x = 0b10101010101010101010101010101010;
  uint32_t y = 0b01010101010101010101010101010101;

  // sleep 10ms, for relays
  msleep(10, system_millis);

  // and write device
  spi_4094_write_n( spi_4094, (void *) &y , 2  );

  // sleep 10ms, for relays
  msleep(10, system_millis);

  // and write device
  spi_4094_write_n( spi_4094, (void *) &x , 2  );
*/
#endif




  /*
    setup the sequence numbers for the different modes.
    we could inject this field - as a string - into data as well.
      or write it using a status bit, of adc for good synchronization. from mode -> adc -> to stamped values, read by data.

    this is a read_mode.  or sequence_mode.
  */



// Might be cleaner to have functions() for these.
// or just pass. note the CH can be represented as an argument.


