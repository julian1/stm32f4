

#include <assert.h>
#include <stdio.h>

#include <string.h>   // strcmp, memset
#include <math.h>   // fabs



#include <peripheral/trigger.h>

#include <peripheral/spi-ice40.h>
#include <peripheral/spi-4094.h>
#include <peripheral/spi-ad5446.h>
#include <peripheral/spi-dac8811.h>



#include <lib2/util.h>      // msleep, UNUSED
#include <lib2/format.h>   // str_format_bits

#include <ice40-reg.h>
#include <mode.h>

#include <util.h> // str_decode_uint



static void state_format ( uint8_t *state, size_t n)
{
  assert(state);

  char buf[100];
  for(unsigned i = 0; i < n; ++i ) {
    printf("v %s\n",  str_format_bits(buf, 8, state[ i ]  ));
  }
}


/*
  eg. we will probably need to pass app
    for all the different spi devices spi devneed

*/


void spi_mode_transition_state( 
      spi_ice40_t   *spi, 
      spi_4094_t    *spi_4094, 
      // spi_ad5446_t  *spi_ad5446,
      spi_t  *spi_ad5446,

      const _mode_t *mode, volatile uint32_t *system_millis  /*, uint32_t update_flags */ )
{
  assert(mode);

  printf("spi_mode_transition_state()\n");
  // printf("4094 size %u\n", sizeof(_4094_state_t));
  assert( sizeof(_4094_state_t) == 12 );

  // mux spi to 4094. change mcu spi params, and set spi device to 4094
  assert(0);
  // spi_mux_4094 ( spi);

/*
  printf("-----------\n");
  printf("write first state\n");
  state_format (  (void *) &mode->first, sizeof(mode->first) );
*/

  // and write device
  spi_4094_reg_write_n( spi_4094, (void *) &mode->first, sizeof( mode->first ) );

  // sleep 10ms, for relays
  msleep(10, system_millis);

/*
  // and format
  // printf("write second state\n");
  state_format ( (void *) &mode->second, sizeof(mode->second) );
*/

  // and write device
  spi_4094_reg_write_n( spi_4094, (void *) &mode->second, sizeof(mode->second) );

  /////////////////////////////


#if 1

  // now write dac state
  assert(0);
  // spi_mux_ice40( spi);
  spi_ice40_reg_write32( spi, REG_SPI_MUX,  SPI_MUX_DAC );

  assert( 0);
  // spi_port_configure_ad5446( spi);
  spi_ad5446_write16( spi_ad5446, mode->dac_val );

#endif



  /////////////////////////////

  // now write fpga register state
  assert(0);
  // spi_mux_ice40(spi);


  spi_ice40_reg_write32(spi, REG_MODE, mode->reg_mode );

  // reg_direct for outputs under fpga control
  assert( sizeof(reg_direct_t) == 4);
  spi_ice40_reg_write_n(spi, REG_DIRECT,  &mode->reg_direct,  sizeof( mode->reg_direct) );


  // sequence mode,
  spi_ice40_reg_write32(spi, REG_SEQ_MODE, mode->reg_seq_mode );


  // sa
  // printf("writing sig acquisition params" );
  spi_ice40_reg_write32(spi, REG_SA_P_CLK_COUNT_PRECHARGE, mode->sa.reg_sa_p_clk_count_precharge );

#if 1
  /*
    had a spi_reg_write8. it would ease this,.
    or use a bitfield having an array.  eg.   seq[ 0].pc   and seq[0].azmux etc.
    but separate regs, eases destructuring on fpga side.
  */
  spi_ice40_reg_write32(spi, REG_SA_P_SEQ_N,        mode->sa.reg_sa_p_seq_n );
  spi_ice40_reg_write32(spi, REG_SA_P_SEQ0,        mode->sa.reg_sa_p_seq0 );
  spi_ice40_reg_write32(spi, REG_SA_P_SEQ1,        mode->sa.reg_sa_p_seq1 );
  spi_ice40_reg_write32(spi, REG_SA_P_SEQ2,       mode->sa.reg_sa_p_seq2 );
  spi_ice40_reg_write32(spi, REG_SA_P_SEQ3,       mode->sa.reg_sa_p_seq3 );
#endif


  // adc
  // printf("writing adc params - aperture %lu\n" ,   mode->adc.reg_adc_p_aperture  );
  spi_ice40_reg_write32(spi, REG_ADC_P_CLK_COUNT_APERTURE,  mode->adc.reg_adc_p_aperture );
  spi_ice40_reg_write32(spi, REG_ADC_P_CLK_COUNT_RESET,     mode->adc.reg_adc_p_reset );


  /*
    AND. ensure that at end of this function - the spi_mux is so only communicating with fpga.
      to limit emi. on spi peripheral lines (4094, dac etc).
  */

  /*
    IMPORTANT - can put the mcu source - trigger state in mode.
    and then set it last. after all the 4094 and fpga register state has been updated.
    --
    this preserves the sequencing.  and minimizes spi xfer emi.
    since it's a single gpio toggle, rather than spi transaction.
    ---
    also ensure the spi_mux == 0.
  */

  // ensure no spurious emi on 4094 lines, when we read fpga state readings
  // can probably just assert and reaad.
  assert( spi_ice40_reg_read32(spi, REG_SPI_MUX) == 0 );
  // spi_ice40_reg_write32(spi, REG_SPI_MUX,  0 );

  // we may want delay here. or make the trigger  an external control state to the mode.


  if(mode->trig_sa)
    ice40_port_trig_sa_enable();    // rename set/clear() ? better?
  else
    ice40_port_trig_sa_disable();

}



/*
    U1003==S5. is A1000-1 gnd. should have used a ref-lo.
      But A1000-1 has current on it - because used with dac.

*/

/*
    - need a reset of all dcv-source muxes.  that we call first.
    - eg. to reset U1003.
*/

void mode_set_dcv_source( _mode_t *mode, double f0 /*signed i0*/)     // needs to be a float... for 0.1 0.01 etc.
{
  // better name?

  printf("set dcv-source\n");


  mode->second.U1006  = S2;   // dcv-source
  mode->second.U1007  = S2;   // dcv-source


  if(f0 >= 0) {
    printf("with +");
    mode->second.U1003  = S1 ;       // positive source.
  } else if (f0 < 0) {

    printf("with -");
    mode->second.U1003  = S2 ;      // negatie source
  }


  if( fabs(f0)  == 10) {
    printf("10V\n");
    mode->second.U1012  = S1;         // 10V tap
  }
  else if(fabs(f0) == 1) {
    printf("1V\n");
    mode->second.U1012  = S2;       // 1V.
  }
  else if(fabs(f0) == 0.1) {
    printf("0.1V\n");
    mode->second.U1012  = S3;       // 0.1V.
  }
  else if(fabs(f0) == 0.01) {
    // not implemented/resistor not poplated
    printf("0.01V\n");
    mode->second.U1012  = S4;       // 0.01V.
  }
  else if(fabs(f0) == 0) {
    printf("0V\n");
    mode->second.U1012  = S5;       // 0V tap.
  }

  else {
    // need better argument validation here,
    // when called programmatically, should not fail.

    assert(0);
  }



}





//    mode->second.U409 = S1;         // lo-mux,  source dcv-source-com which is ref-lo

void mode_set_dcv_source_ref( _mode_t *mode, unsigned u0 )
{

    mode->second.U1012  = S8 ;       // agnd. reset/not used..
    mode->second.U1003  = S8 ;       // agnd. reset/not used..


  if(u0 == 7) {
    printf("with ref-hi +7V\n");
    mode->second.U1006  = S4;       // ref-hi
    mode->second.U1007  = S4;       // ref-lo
  }
  else if( u0 == 0 ) {
    // need bodge for this
    printf("with ref-lo\n");
    mode->second.U1006  = S8;       // ref-lo
    mode->second.U1007  = S4;       // ref-lo - looks funny. gives bad measurement. on DMM.
  }
  else
    assert(0);
}



void mode_set_dcv_source_dac( _mode_t *mode, signed u0 )
{
    printf("dac\n");

    mode->second.U1012  = S8 ;       // agnd. reset/not used..
    mode->second.U1003  = S8 ;       // agnd. reset/not used..

    mode->second.U1006  = S3;       // dac
    mode->second.U1007  = S3;       // dac

  if(u0 >= 0) {
    printf("with +");
    mode->second.U1003  = S1 ;       // positive source.
  } else if (u0 < 0) {

    printf("with -");
    mode->second.U1003  = S2 ;      // negatie source
  }

  // should do better range check.
  assert(u0 <= 0x3fff);

  mode->dac_val = u0;// abs( u0 );
}



void mode_set_dcv_source_header( _mode_t *mode )
{

  mode->second.U1006  = S8;          // cap.


  mode->second.U1003  = S3;          // agnd.
}









void set_seq_mode( _mode_t *mode, uint32_t seq_mode , uint8_t arg0, uint8_t arg1 )
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

      mode->sa.reg_sa_p_seq_n = 1;
      if(arg0 == S3 ) {
        mode->sa.reg_sa_p_seq0 = (0b00 << 4) | S3;        // dcv
      }
      else if(arg0 == S1 ) {
        mode->sa.reg_sa_p_seq0 = (0b00 << 4) | S1;        // himux
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

      mode->sa.reg_sa_p_seq_n = 1;
      if(arg0 == S3 ) {
        mode->sa.reg_sa_p_seq0 = (0b01 << 4) | S3;        // dcv
      }
      else if(arg0 == S1 ) {
        mode->sa.reg_sa_p_seq0 = (0b10 << 4) | S1;        // himux
      }
      else if(arg0 == S7 ) {
        mode->sa.reg_sa_p_seq0 = (0b00 << 4) | S7;        // star-lo
      }
      else if(arg0 == S8 ) {
        mode->sa.reg_sa_p_seq0 = (0b00 << 4) | S8;        // lomux
      }

      else assert(0);
      break;
    }



    case SEQ_MODE_AZ: {
    // write the seq

      mode->sa.reg_sa_p_seq_n = 2;

      // hi goes first

      if(arg0 == S3 )
        mode->sa.reg_sa_p_seq0 = (0b01 << 4) | S3;      // dcv
      else if(arg0 == S1 )
        mode->sa.reg_sa_p_seq0 = (0b10 << 4) | S1;        // himux
      else
        assert(0);

      if(arg1 == S7)
        mode->sa.reg_sa_p_seq1 = (0b00 << 4) | S7;        // star-lo
      else if(arg1 == S8)
        mode->sa.reg_sa_p_seq1 = (0b00 << 4) | S8;        // lomux
      else
        assert(0);
/*
    // applies both chanels.
      if(arg0 == S3 && arg1 == S7) {
        mode->sa.reg_sa_p_seq0 = (0b01 << 4) | S3;        // dcv
        mode->sa.reg_sa_p_seq1 = (0b00 << 4) | S7;        // star-lo
      }
      else if(arg0 == S1 && arg1 == S8)  {
        mode->sa.reg_sa_p_seq0 = (0b01 << 4) | S1;        // himux   WRONG. FIXME.   not switching the PC.
        mode->sa.reg_sa_p_seq1 = (0b00 << 4) | S8;        // lomux
      }
      else if(arg0 == S3 && arg1 == S8 )  {                // eg. for ref.
        mode->sa.reg_sa_p_seq0 = (0b01 << 4) | S3;        // dcv
        mode->sa.reg_sa_p_seq1 = (0b00 << 4) | S8;        // lomux
      }
      else assert(0);
*/
      break;
    }

/*
    case SEQ_MODE_ELECTRO: {

      // same as no az, except don't switch the precharge
      mode->sa.reg_sa_p_seq_n = 1;
      mode->sa.reg_sa_p_seq0 = (0b00 << 4) | S3;        // dcv
      break;
    }
*/

    case SEQ_MODE_AG:
    case SEQ_MODE_RATIO: {
      // 4 cycle, producing single output
      // Issue - is for internal - we need to set the common lo. eg. ref-lo. or start

      mode->sa.reg_sa_p_seq_n = 4;
      mode->sa.reg_sa_p_seq0 = (0b01 << 4) | S3;        // dcv
      mode->sa.reg_sa_p_seq1 = (0b00 << 4) | S8;        // ref-lo // star-lo
      mode->sa.reg_sa_p_seq2 = (0b10 << 4) | S1;        // himux
      mode->sa.reg_sa_p_seq3 = (0b00 << 4) | S8;        // ref-lo /// lomux
      break;
    }

/*
    case SEQ_MODE_AG: {
      // auto-gain 4 cycle - same as ratio. producing a single output


      mode->sa.reg_sa_p_seq_n = 4;
      // sample
      mode->sa.reg_sa_p_seq0 = (0b01 << 4) | S3;        // dcv
      mode->sa.reg_sa_p_seq1 = (0b00 << 4) | S7;        // star-lo
      // reference
      mode->sa.reg_sa_p_seq2 = (0b10 << 4) | S1;        // himux
      mode->sa.reg_sa_p_seq3 = (0b00 << 4) | S7;        // lomux
      break;
    }
*/
    case SEQ_MODE_DIFF: {
      // 2 cycle, hi- hi2, with both precharge switches switches. single output.
      mode->sa.reg_sa_p_seq_n = 2;
      mode->sa.reg_sa_p_seq0 = (0b01 << 4) | S3;        // dcv
      mode->sa.reg_sa_p_seq2 = (0b01 << 4) | S1;        // himux
      break;
    }

    case SEQ_MODE_SUM_DELTA: {    // change name.  SUM_DELTA. 0w

      // similar. take hi/lo, hi2/lo, .  but where lo is shared. so can calculate hi-lo, hi2-lo, hi-hi2.
      // advantage of a single sequence - is that flicker noise should cancel some.
      // noting that input can be external terminals - or the dcv-source and its inverted output.
      // to encodekkkkkkkkk
      // can do as 3 values or 4 values.   3 is more logical.

      mode->sa.reg_sa_p_seq_n = 3;
      mode->sa.reg_sa_p_seq0 = (0b01 << 4) | S3;        // dcv
      mode->sa.reg_sa_p_seq1 = (0b00 << 4) | S7;        // star-lo
      mode->sa.reg_sa_p_seq2 = (0b10 << 4) | S1;        // himux
      break;
    }

    default:
      assert( 0);
  }


}





bool mode_repl_statement( _mode_t *mode,  const char *cmd, uint32_t line_freq )
{

  // move this to mode.

  char s0[100 + 1 ];
  char s1[100 + 1 ];
  char s2[100 + 1 ];
  uint32_t u0, u1;
  double f0;
  // int32_t i0;


  // uint32_t line_freq = app->line_freq;
      // _mode_t *mode = app->mode_current;


  /*
      could move the dcv-source function handling
  */

  // we want

  // +10,0,-10.    fixed divider.
  // if increment. then could use the dac.
  // perhaps change name dcv-source fixed
  if( sscanf(cmd, "dcv-source %lf", &f0) == 1) {

      // printf("set dcv-source, input relays, for current_mode\n");
      mode_set_dcv_source( mode, f0);
  }

  // ref-hi , ref-lo
  else if( strcmp(cmd, "dcv-source ref-hi") == 0)
      mode_set_dcv_source_ref( mode, 7 );

  else if( strcmp(cmd, "dcv-source ref-lo") == 0)
      mode_set_dcv_source_ref( mode, 0 );


  else if( sscanf(cmd, "dcv-source dac %100s", s0) == 1
    && str_decode_uint( s0, &u0)) {

      // TODO perhaps should be able to handle +-
      // eg. dcv-source dac 0x3fff
      mode_set_dcv_source_dac( mode, u0);
  }

  // cap
  else if( strcmp(cmd, "dcv-source header") == 0)
    mode_set_dcv_source_header( mode);



  else if( sscanf(cmd, "boot%100s", s0) == 1
    && str_decode_uint( s0, &u0))  {

    set_seq_mode( mode, SEQ_MODE_BOOT, u0, 0 );
  }
  else if( sscanf(cmd, "noazero %100s", s0) == 1
    && str_decode_uint( s0, &u0))  {

    set_seq_mode( mode, SEQ_MODE_NOAZ, u0, 0 );
  }
  else if( sscanf(cmd, "azero %100s %100s", s0, s1) == 2
    && str_decode_uint( s0, &u0)
    && str_decode_uint( s1, &u1)) {

    set_seq_mode( mode, SEQ_MODE_AZ, u0, u1 );
  }

    // ratio is hardcoded to use lomux at the moment. and not star-lo.
  else if(strcmp(cmd, "ratio") == 0) {
    set_seq_mode( mode, SEQ_MODE_RATIO, 0,0 );
  }

  else if(strcmp(cmd, "ag") == 0)
    set_seq_mode( mode, SEQ_MODE_AG, 0, 0 );

  else if(strcmp(cmd, "diff") == 0)
    set_seq_mode( mode, SEQ_MODE_DIFF, 0 , 0);

  else if(strcmp(cmd, "sum-test") == 0)
    set_seq_mode( mode, SEQ_MODE_SUM_DELTA, 0, 0 );


  // "h" for halt
  else if(strcmp(cmd, "halt") == 0 || strcmp(cmd, "h") == 0) {

    // _mode_t *mode = app->mode_current;
    mode->trig_sa = 0;
  }
  // "t" to trigger
  else if(strcmp(cmd, "trig") == 0 || strcmp(cmd, "t") == 0) {

    /* / trigger - has a dependency on both data and the mode
    // because we want to clear the data buffer
    // No. I think the trigger. should not change or clear the data buff. it just starts the adc.
    */

    printf("trigger set\n");

   // _mode_t *mode = app->mode_current;

    mode->trig_sa = 1;
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
    mode->adc.reg_adc_p_aperture = aperture;
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

      mode->adc.reg_adc_p_aperture = aperture;
    }
  }

#if 0
    else if(strcmp(s0, "precharge") == 0) {
      mode->sa.reg_sa_p_clk_count_precharge = u0;
    }
#endif


  else if( sscanf(cmd, "seq mode %100s", s0) == 1
    && str_decode_uint( s0, &u0)) {

    mode->reg_seq_mode = u0;

  }



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
        mode->sa.reg_sa_p_seq0 = val;
      }
      else if(strcmp(s0, "seq1") == 0) {
        mode->sa.reg_sa_p_seq1 = val;
      }
       else if(strcmp(s0, "seq2") == 0) {
        mode->sa.reg_sa_p_seq2 = val;
      }
      else if(strcmp(s0, "seq3") == 0) {
        mode->sa.reg_sa_p_seq3 = val;
      }
      else {
        printf("unknown target %s for 3 var set\n", s0);
        return 0;
      }

  }

  // two value set.
  else if( sscanf(cmd, "set %100s %100s", s0, s1) == 2
    && str_decode_uint( s1, &u0)
  ) {

      // printf("set %s %lu\n", s0, u0);

      // cannot manage pointer to bitfield. so have to hardcode.


      // better name. _count.
      if(strcmp(s0, "seqn") == 0) {
        mode->sa.reg_sa_p_seq_n = u0;
      }


      // ice40 mode.
      else if(strcmp(s0, "mode") == 0) {
        mode->reg_mode = u0;
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
      else if(strcmp(s0, "sig_pc_sw") == 0 || strcmp(s0, "pc") == 0   ) {
        mode->reg_direct.sig_pc_sw_o= u0;
      }
/*
      else if(strcmp(s0, "sig_pc_sw0") == 0) {

        uint8_t val = mode->reg_direct.sig_pc_sw_o;

        val |= u0  ;
        val &= ~ u0 ;
        mode->reg_direct.sig_pc_sw_o |= u0 ;
      }
*/





      else if(strcmp(s0, "azmux") == 0) {
        mode->reg_direct.azmux_o = u0;
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
      // perhaps rename second. _4094_second etc.


      else if(strcmp(s0, "u504") == 0) {
        mode->second.U504 = u0;
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
/*
      else if(strcmp(s0, "u1010") == 0) {
        mode->second.U1010 = u0;
      }
      else if(strcmp(s0, "u1009") == 0) {
        mode->second.U1009 = u0;
      }
*/


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

      else if(strcmp(s0, "k703") == 0) {
        mode->first.K703 = u0 ? LR_SET: LR_RESET;
      }


      else if(strcmp(s0, "u408") == 0 || strcmp(s0, "himux") == 0) {
        mode->second.U408 = u0 ;
      }
      else if(strcmp(s0, "u409") == 0 || strcmp(s0, "lomux") == 0) {
        mode->second.U409 = u0 ;
      }

/*
      set via fpga direct reg. not 4094.
      else if(strcmp(s0, "u410") == 0 || strcmp(s0, "azmux") == 0) {
        mode->second.U410 = u0 ;
      }
*/



      /*
        not completely clear if trig wants to be out-of-band. eg not put in the mode structure.
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
  } else {

    return 0;
  }

  return 1;
}






  /*
    setup the sequence numbers for the different modes.
    we could inject this field - as a string - into data as well.
      or write it using a status bit, of adc for good synchronization. from mode -> adc -> to stamped values, read by data.

    this is a read_mode.  or sequence_mode.
  */



// Might be cleaner to have functions() for these.
// or just pass. note the CH can be represented as an argument.


