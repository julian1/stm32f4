

#include <assert.h>
#include <stdio.h>

#include <string.h>   // strcmp, memset



#include <peripheral/ice40-extra.h>

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



void spi_mode_transition_state( uint32_t spi, const _mode_t *mode, volatile uint32_t *system_millis  /*, uint32_t update_flags */ )
{
  assert(mode);

  // printf("4094 size %u\n", sizeof(_4094_state_t));
  assert( sizeof(_4094_state_t) == 7 );

  // mux spi to 4094. change mcu spi params, and set spi device to 4094
  spi_mux_4094 ( spi);

/*
  printf("-----------\n");
  printf("write first state\n");
  state_format (  (void *) &mode->first, sizeof(mode->first) );
*/

  // and write device
  spi_4094_reg_write_n(spi, (void *) &mode->first, sizeof( mode->first ) );

  // sleep 10ms
  msleep(10, system_millis);

/*
  // and format
  // printf("write second state\n");
  state_format ( (void *) &mode->second, sizeof(mode->second) );
*/

  // and write device
  spi_4094_reg_write_n(spi, (void *) &mode->second, sizeof(mode->second) );

  /////////////////////////////

  // now write dac state

   // spi_mux_dac8811(app->spi);
  spi_mux_ad5446( spi );

  // for dac881 1eg. 0=-0V out.   0xffff = -7V out. nice.
  // spi_dac8811_write16( app->spi, mode->dac_val );

  // for ad5444  14bit max is 0x3fff.
  spi_ad5446_write16( spi, mode->dac_val );




  /////////////////////////////

  // now write fpga register state
  spi_mux_ice40(spi);


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

  if(mode->trigger_source_internal)
    ice40_port_trigger_source_internal_enable();    // rename set/clear() ? better?
  else
    ice40_port_trigger_source_internal_disable();

}




void mode_set_dcv_source( _mode_t *mode, signed i0)
{
  // 10,0,-10
  printf("set dcv-source\n");

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
    // when called programmatically, should not fail.
    assert(0);
  }

  mode->second.U1006  = S1 ;          // s1.   follow  .   dcv-mux2

  // setup input relays.
  mode->first .K405 = LR_SET;     // select dcv
  mode->first .K406 = LR_SET;   // accum relay off
  mode->first .K407 = LR_RESET;   // select dcv-source

}


//    mode->second.U409 = S1;         // lo-mux,  source dcv-source-com which is ref-lo

void mode_set_ref_source( _mode_t *mode, unsigned u0 )
{

  // TODO U1012 should source a gnd, when add it
  mode->second.U1012  = SOFF;       // should probably be agnd.

  if(u0 == 7) {
    printf("with ref-hi +7V\n");
    mode->second.U1006  = S4;       // ref-hi
                                    // no . we shouldn't be setting lo-mux here
  }
  else if( u0 == 0 ) {
    // need bodge for this
    printf("with ref-lo\n");
    mode->second.U1006  = S7;       // ref-lo
  }
  else
    assert(0);

  mode->first .K405 = LR_SET;     // select dcv
  mode->first .K406 = LR_SET;   // accum relay off
  mode->first .K407 = LR_RESET;   // select dcv-source
}


void set_seq_mode( _mode_t *mode, uint32_t seq_mode , uint32_t channel )
{
  /*
    doesn't have to be exhausive wrt cases.
    can still setup manually.
  */

  mode->reg_seq_mode = seq_mode;                 // to guide decoder

  switch(seq_mode) {

    case SEQ_MODE_AZ: { // we channel 1.
    // write the seq

      mode->sa.reg_sa_p_seq_n = 2;
    // applies both chanels.
      if(channel == 1) {
        mode->sa.reg_sa_p_seq0 = (0b01 << 4) | S3;        // dcv
        mode->sa.reg_sa_p_seq1 = (0b00 << 4) | S7;        // star-lo
      }
      else if(channel == 2)  {
        mode->sa.reg_sa_p_seq0 = (0b01 << 4) | S1;        // himux
        mode->sa.reg_sa_p_seq1 = (0b00 << 4) | S8;        // lomux
      }
      else if(channel == 3)  {    // eg. for ref.
        mode->sa.reg_sa_p_seq0 = (0b01 << 4) | S3;        // dcv
        mode->sa.reg_sa_p_seq1 = (0b00 << 4) | S8;        // lomux
      }
      else assert(0);
      break;
    }


    case SEQ_MODE_NOAZ: {
      // clearer - to express as another mode, rather than as a bool.
      // azero off - just means swtich the pc for symmetry/ and keep charge-injetion the same with azero mode.

      mode->sa.reg_sa_p_seq_n = 1;
      if(channel == 1) {
        mode->sa.reg_sa_p_seq0 = (0b01 << 4) | S3;        // dcv
      }
      else if(channel == 2 ) {
        mode->sa.reg_sa_p_seq0 = (0b01 << 4) | S1;        // himux
      }
      else assert(0);
      break;
    }


    case SEQ_MODE_ELECTRO: {

      // same as no az, except don't switch the precharge
      mode->sa.reg_sa_p_seq_n = 1;
      mode->sa.reg_sa_p_seq0 = (0b00 << 4) | S3;        // dcv
      break;
    }

    case SEQ_MODE_RATIO: {
      // 4 cycle, producing single output

      mode->sa.reg_sa_p_seq_n = 4;
      mode->sa.reg_sa_p_seq0 = (0b01 << 4) | S3;        // dcv
      mode->sa.reg_sa_p_seq1 = (0b00 << 4) | S7;        // star-lo
      mode->sa.reg_sa_p_seq2 = (0b01 << 4) | S1;        // himux
      mode->sa.reg_sa_p_seq3 = (0b00 << 4) | S7;        // lomux
      break;
    }

    case SEQ_MODE_AG: {
      // auto-gain 4 cycle - same as ratio. producing a single output
      mode->sa.reg_sa_p_seq_n = 4;
      // sample
      mode->sa.reg_sa_p_seq0 = (0b01 << 4) | S3;        // dcv
      mode->sa.reg_sa_p_seq1 = (0b00 << 4) | S7;        // star-lo
      // reference
      mode->sa.reg_sa_p_seq2 = (0b01 << 4) | S1;        // himux
      mode->sa.reg_sa_p_seq3 = (0b00 << 4) | S7;        // lomux
      break;
    }

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
      mode->sa.reg_sa_p_seq2 = (0b01 << 4) | S1;        // himux
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
  int32_t i0;


  // uint32_t line_freq = app->line_freq;
      // _mode_t *mode = app->mode_current;


  /* EXTR -
      review move dcv-source function handling
      to another file. in an
      /source  ?


  */

  // +10,0,-10.    if increment. then could use the dac.
  if( sscanf(cmd, "dcv-source %ld", &i0 ) == 1) {

      // printf("set dcv-source, input relays, for current_mode\n");
      mode_set_dcv_source( mode, i0);
  }



  else if( sscanf(cmd, "dcv-source dac %100s", s0) == 1
    && str_decode_uint( s0, &u0)) {

      // our str_decode_uint function doesn't handle signedness...
      // and we want the hex value.
      // but we could

      // should
      // eg. dcv-source dac 0x3fff


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


  // ref-hi , ref-lo
  else if( strcmp(cmd, "dcv-source ref-hi") == 0)
      mode_set_ref_source( mode, 7 );

  else if( strcmp(cmd, "dcv-source ref-lo") == 0)
      mode_set_ref_source( mode, 0 );




  else if(strcmp(cmd, "azero ch1") == 0)    // dcv using star-gnd as lo
    set_seq_mode( mode, SEQ_MODE_AZ, 1 );

  else if(strcmp(cmd, "azero ch2") == 0)    // himux/lomux
    set_seq_mode( mode, SEQ_MODE_AZ, 2 );

  // this should be called 'cross'.
  // better name?
  else if(strcmp(cmd, "azero cross") == 0)      // rather than a integer argument - perhaps should pass the two az switch cases?. 
    set_seq_mode( mode, SEQ_MODE_AZ, 3 );       // actually why not expose in string api.   eg. 'azero S1 S7' etc.




  else if(strcmp(cmd, "noazero ch1") == 0)    // could take arg as to theinput.
    set_seq_mode( mode, SEQ_MODE_NOAZ, 1 );

  else if(strcmp(cmd, "noazero ch2") == 0)
    set_seq_mode( mode, SEQ_MODE_NOAZ, 2 );


  else if(strcmp(cmd, "electro") == 0)
    set_seq_mode( mode, SEQ_MODE_ELECTRO, 0 );


  else if(strcmp(cmd, "ratio") == 0)
    set_seq_mode( mode, SEQ_MODE_RATIO, 0 );

  else if(strcmp(cmd, "ag") == 0)
    set_seq_mode( mode, SEQ_MODE_AG, 0 );

  else if(strcmp(cmd, "diff") == 0)
    set_seq_mode( mode, SEQ_MODE_DIFF, 0 );

  else if(strcmp(cmd, "sum-test") == 0)
    set_seq_mode( mode, SEQ_MODE_SUM_DELTA, 0 );





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
      else if(strcmp(s0, "sig_pc_sw") == 0) {
        mode->reg_direct.sig_pc_sw_o= u0;
      }
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



      else if(strcmp(s0, "u409") == 0 || strcmp(s0, "lomux") == 0) { //  lomux / could use alternate name
        mode->second.U409 = u0 ;
      }
      else if(strcmp(s0, "u410") == 0 || strcmp(s0, "himux") == 0) {    // himux
        mode->second.U410 = u0 ;
      }




      /*
        not completely clear if trig wants to be out-of-band. eg not put in the mode structure.
      */
/*
      else if(strcmp(s0, "trig") == 0) {
        // should move/place in signal acquisition?
        mode->trigger_source_internal = u0;
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


