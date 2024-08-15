/*
  consider rename this file. perhaps to measurement.
  and move the more cal specific functions to cal. perhaps.

*/
#include <stdio.h>
#include <assert.h>


#include <lib2/util.h>      // ARRAY_SIZE

#include <data/data.h>
#include <data/matrix.h>     // m_from_scalar
#include <data/buffer.h>     // m_from_scalar


#include <util.h>     // aper_n_to_period

#include <ice40-reg.h>    // for seq mode

#include <lib2/format.h>  // format_float

#include <ice40-reg.h>
#include <peripheral/spi-ice40.h>

// #include <peripheral/vfd.h>


#define UNUSED(x) (void)(x)







void data_init( data_t *data)
{
  assert(data);
  assert(data->magic == DATA_MAGIC) ;

  // make a default buffer. available. on start
  // might be clearer - to just call this in main()

  data->buffer = buffer_reset( data->buffer, 10);
  assert( data->buffer);
  data_reset( data );
}



void data_reading_reset( data_t *data )
{
  // clear the reading buffers,
  // but leave the buffer alone
  memset( data->reading, 0, sizeof( data->reading));
  memset( data->reading_last, 0, sizeof( data->reading_last));
}



void data_reset( data_t * data )
{
/*
    normal buffer handling

    ---
    there's no reason this needs to make calls on the buffer, to change memory etc,
    buffer is expected to be appropriately sized.
    we just need need to truncate the rows, and reset the insert idx.
*/

  // rename
  assert(data);
  assert(data->magic == DATA_MAGIC);
  assert(data->buffer);

  m_truncate_rows( data->buffer, 0 );  // reset buffer, truncate vertical length.

  data->buffer_idx = 0;                // reset overflow index, rename?

  data_reading_reset( data);            // reset the reading buffer
}







void data_rdy_interupt( data_t *data) // runtime context
{
  /* interupt context.  don't do anything compliicated here.
    but called relatively infrequent.
  */

  assert(data);
  assert(data->magic == DATA_MAGIC) ;


  // if flag is still active, then record we missed processing some data.
  if(data->adc_interupt_valid == true) {
    data->adc_interupt_valid_missed = true;
    // ++data->adc_interupt_valid_missed;     // count better? but harder to report.
  }

  // set adc_interupt_valid flag so that update() knows to read the adc...
  data->adc_interupt_valid = true;
}





char * seq_mode_str( uint8_t sample_seq_mode, char *buf, size_t n  )
{
  char *s = 0;

  switch(sample_seq_mode) {

    case 0:                 s = "none"; break;
    case SEQ_MODE_BOOT:       s = "boot"; break;
    case SEQ_MODE_NOAZ:     s = "noaz"; break;
//    case SEQ_MODE_ELECTRO:  s = "electro"; break;
    case SEQ_MODE_AZ:       s = "az"; break;
    case SEQ_MODE_RATIO:    s =  "ratio"; break;
    case SEQ_MODE_AG:       s = "ag"; break;
    case SEQ_MODE_DIFF:     s = "diff"; break;
    case SEQ_MODE_SUM_DELTA:  s = "sum-delta"; break;
    default:
      assert(0);
  };

  strncpy(buf, s, n);
  return buf;
}






void data_update_new_reading2(data_t *data, uint32_t spi/*, bool verbose*/)
{
  assert(data);
  assert(data->magic == DATA_MAGIC);

  /*
    the question - is can we do this without interaction with the mode_t.
                  ideally we shouldn't need anything.
                  just querying the adc, and sequence acquisition.

    we have comitted to processing a new incomming reading
  */



  /*
      do all spi transfers in prioity, without any other mcu work,
  */

  uint32_t status = spi_ice40_reg_read32( spi, REG_STATUS );

  uint32_t clk_count_mux_neg = spi_ice40_reg_read32( spi, REG_ADC_CLK_COUNT_REFMUX_NEG);
  uint32_t clk_count_mux_pos = spi_ice40_reg_read32( spi, REG_ADC_CLK_COUNT_REFMUX_POS);
  uint32_t clk_count_mux_rd  = spi_ice40_reg_read32( spi, REG_ADC_CLK_COUNT_REFMUX_RD);
  uint32_t clk_count_mux_sig = spi_ice40_reg_read32( spi, REG_ADC_CLK_COUNT_MUX_SIG);

  uint32_t clk_count_mux_reset = 0;
  uint32_t stat_count_refmux_pos_up = 0;
  uint32_t stat_count_refmux_neg_up = 0;
  uint32_t stat_count_cmpr_cross_up = 0;

  if(data->show_extra) {

    clk_count_mux_reset       = spi_ice40_reg_read32( spi, REG_ADC_CLK_COUNT_REFMUX_RESET);

    stat_count_refmux_pos_up = spi_ice40_reg_read32( spi, REG_ADC_STAT_COUNT_REFMUX_POS_UP);
    stat_count_refmux_neg_up = spi_ice40_reg_read32( spi, REG_ADC_STAT_COUNT_REFMUX_NEG_UP);
    stat_count_cmpr_cross_up = spi_ice40_reg_read32( spi, REG_ADC_STAT_COUNT_CMPR_CROSS_UP);
  }



  // printf("-------------------\n");

/*
  // suppress late measure samples arriving after signal_acquisition is returned to arm
  if(!mode->trig_sa)
    return
*/


  // printf("r %u  v %lu  %s\n",  REG_STATUS, status,  str_format_bits(buf, 32, status));

  // TODO consider create a bitfield for the status register

  uint8_t hw_flags        =  ADC_STATUS_HW_FLAGS( status) ;         // 0b111 & (status >> 8 ) ;
  UNUSED(hw_flags);
  uint8_t reg_spi_mux     =  ADC_STATUS_SPI_MUX( status)  ;
  uint8_t sample_idx      =  ADC_STATUS_SAMPLE_IDX( status) ;       // 0b111 & (status >> 16) ;     // we set this to 0b111 somewhere in verilog?
  uint8_t sample_seq_n    =  ADC_STATUS_SAMPLE_SEQ_N( status) ;     // 0b111 & (status >> 20) ;
  uint8_t sample_seq_mode =  ADC_STATUS_SAMPLE_SEQ_MODE( status);   // 0b111 & (status >> 24) ;


  assert( reg_spi_mux == SPI_MUX_NONE);

  // ensure we can store the value against the sample idx.
  assert(sample_idx < ARRAY_SIZE( data->reading));

  char buf[100];


 // if(data->show_seq)
  {

    printf("%s", seq_mode_str( sample_seq_mode, buf, 8 )); // puts()
    // printf(" seq_mode %u  %u of %u ", sample_seq_mode,   sample_idx, sample_seq_n );
    printf(", %u of %u", sample_idx, sample_seq_n );
  }


  if(data->show_counts) {

    printf(", clk counts %7lu %7lu %6lu %lu",
      clk_count_mux_neg,
      clk_count_mux_pos,
      clk_count_mux_rd,
      clk_count_mux_sig
    );
  }


  // data show_count_extra?
  if(data->show_extra) {

    printf(", reset %6lu", clk_count_mux_reset);

    printf(", adc stats %lu %lu %lu",
      stat_count_refmux_pos_up,
      stat_count_refmux_neg_up,
      stat_count_cmpr_cross_up
    );

    double period = aper_n_to_period( clk_count_mux_sig);
    printf(", period %.2lf", period );

    double freq = ((double) stat_count_refmux_pos_up) / period;
    printf(", freq %.0lf kHz", freq / 1000.f );
  }





  // could factor into another func - to ease this nesting.
  if(data->model_b) {

    // TODO - have a scalar - version of this
    // would eases calculation. when only need a scalar.
    // WE NEED TO GET THIS CODED in a test. and the result recorded.
    // unsigned cols = 4;

    /*
        should keep fields stored against app_t structure. to avoid cost and complexity of always reallocating.
        and having to free.
        and make early return easier

    // app->xs = run_to_mat( ..., app->xs );   its a good simplification.
    */


    MAT *xs = run_to_matrix(
        clk_count_mux_neg,
        clk_count_mux_pos,
        clk_count_mux_rd,

        // what model here,
        // data->model_spec,
        m_rows( data->model_b ),       // why not just use data->b here, and conform to what the model requires???
        MNULL
      );

    assert( m_cols(xs) == m_rows( data->model_b) ) ;

    // we could make all these vars persist.
    MAT	*m_mux_sig = m_from_scalar( clk_count_mux_sig, MNULL );
    assert(m_mux_sig);
    assert( m_is_scalar(m_mux_sig) );

    // Mar 2024.

    // TODO rename m_predicted == ret.  use reading
    //  we should persist this.  and pass it in to m_calc_predicated.
    MAT *m_predicted =  m_calc_predicted( data->model_b, xs, m_mux_sig /*, app->m_predicted */);
    assert(m_predicted);
    assert(m_is_scalar(m_predicted) );

    // store the value against the sample idx.
    assert(sample_idx < ARRAY_SIZE( data->reading));

    // shift previous reading
    data->reading_last[ sample_idx ] = data->reading[ sample_idx ] ;

    // update for this reading
    double predicted = m_to_scalar(m_predicted );

    // we use 0 to encode no-value recorded yet..   or if(predicted == 0) predicted = EPSILON;
    assert(predicted != 0);
    data->reading[ sample_idx ] = predicted;

    if(data->show_extra) {
      printf(", %f", predicted);
    }

    // debug. show all readings
    if(0) {

        printf("\n");
        for(unsigned i = 0; i < sample_seq_n; ++i ) {
          printf("%u %lf\n", i, data->reading[ i ] ) ;
        }
    }

/*
    - there's an issue, that on the first iteration,  the last reading will corrupt the calculation
    - and then the stats buffer will be wrong - until it cycles through.
    - BUT - we may have improved this it a lot - by clearing this value - when we reset the buffer.
    ----
    - can fix - by storing same reading into the last, if it's the first time through the acquistion sequence.
*/

    M_FREE( xs );
    M_FREE( m_mux_sig );
    M_FREE( m_predicted );


    /* / EXTR.   rather than having separate variables for hi  and lo[2 ]
    // why not store in an array????  according to the sequence - then we can always update
    //  data[ sample_idx ] [ ]  = val.
    // EXTR.   Do we even need to do the shuffle, with the lo value .  Just use j
    */

    // a switch would be cleaner...

    // change name to computed readding
    data->computed_val = 0;

    switch(sample_seq_mode) {

      case SEQ_MODE_BOOT:
      case SEQ_MODE_NOAZ: {

        // AZ mode, on channel 1 or channel 2, but encoded in first two readings
        assert( sample_seq_n == 1);
        // eg. just the hi.
        if(data->reading[0] != 0)
          data->computed_val = data->reading[ 0 ] ;

        break;
      }


      case SEQ_MODE_AZ: {

        assert( sample_seq_n == 2);

        // assume 0 value means never been updated.
        if(  data->reading[0] != 0
          && data->reading[1] != 0
          && data->reading_last[1] != 0) {

          // eg. hi - average two lo
          data->computed_val = data->reading[0]  - ((data->reading[ 1 ] + data->reading_last[1] ) / 2.f);
        }


        break;
      }

      case SEQ_MODE_RATIO: {

        assert( sample_seq_n == 4);
        // ratio of two az values
        // NOTE - REVIEW - we could also used the last/lagged LO. for more reading stability

        if(  data->reading[0] != 0
          && data->reading[1] != 0
          && data->reading[2] != 0
          && data->reading[3] != 0)  {

          data->computed_val = (data->reading[0] - data->reading[1]) / (data->reading[2] - data->reading[3]);

          /*
          printf("\n");
          printf("0 %f\n", data->reading[0] );
          printf("1 %f\n", data->reading[1] );
          printf("2 %f\n", data->reading[2] );
          printf("3 %f\n", data->reading[3] );
          */
        }
        break;
      }

      case SEQ_MODE_AG: {

        assert( sample_seq_n == 4);

        if(  data->reading[0] != 0
          && data->reading[1] != 0
          && data->reading[2] != 0
          && data->reading[3] != 0)  {

          // need
          double coeff = 2.f; // two to test.
          // channel 1 reading, adjusted according to gain on the dcv-source used as reference on channel 2
          data->computed_val = (data->reading[0] - data->reading[1]) / (data->reading[2] - data->reading[3]) * coeff;
        }
        break;
      }

      case SEQ_MODE_DIFF: {

        assert( sample_seq_n == 2);
        // channel1 hi - channel 2 hi
        if(  data->reading[0] != 0
          && data->reading[2] != 0)
          data->computed_val = data->reading[0] - data->reading[2];

        break;
      }

      case SEQ_MODE_SUM_DELTA:  {

        assert( 0);
        /* think this doesn't work - we have to be able to swap the inputs around .   eg. use caps.
          so we have a sequence acquisition sampling mode - where take hi on channel 1, and lo on channel 2.
          and a lo common, but doesn't help. because (hi1 - lo ) + (hi2 - lo ) == (hi1 - hi2) as an identity.
        */
        /*
        mode->sa.reg_sa_p_seq_n = 3;
        mode->sa.reg_sa_p_seq0 = (0b01 << 4) | S3;        // dcv
        mode->sa.reg_sa_p_seq1 = (0b00 << 4) | S7;        // star-lo
        mode->sa.reg_sa_p_seq2 = (0b01 << 4) | S1;        // himux
        */
        // hang on isn't this true by definition?
        /*
        double dcv   = (data->reading[0] - data->reading[1];
        double himux = (data->reading[2] - data->reading[1];
        data->computed_val = dcv + ;
        */
        break;
      }

      default:
        assert(0);

    };


    if(data->computed_val) {

      // we want with commas (easier to read) and without commas (easier to process programatically).

      // record these
      data->adc_status = status;
      data->adc_clk_count_mux_sig = clk_count_mux_sig;



      // STTCPW

      str_format_float_with_commas(buf, 100, 7, data->computed_val);

      if(sample_seq_mode == SEQ_MODE_RATIO)
        printf(" meas %s", str_format_float_with_commas(buf, 100, 7, data->computed_val));
      else
        printf(" meas %sV", buf );

#if 0

      // write value
      vfd_write_bitmap_string2( buf, 0 , 0 );

      // write mode
      seq_mode_str( sample_seq_mode, buf, 8 );
      for(unsigned i = 0; i < strlen(buf); ++i)   // stoupper
        buf[i] = toupper(buf[i] );
      vfd_write_string2( buf, 0, 3 );

      // write nplc
      double nplc = aper_n_to_nplc( clk_count_mux_sig, data->line_freq );
      snprintf(buf, 100, "nplc %.1lf ", nplc );
      vfd_write_string2( buf, 0, 4 );

      // write a star, for the sample
      vfd_write_string2( sample_idx % 2 == 0 ? "*" : " ", 0, 5 );

      snprintf(buf, 100, "local, range DCV, 10M" );   // dummy other stuff.
      vfd_write_string2( buf, 0, 6 );
#endif


      // vfd_write_string2( "123467890", 0, 5 );
    }

    /*
      can drive this with policy arg/flag.
      if data->buffer is full either keep cycling.
      or stop. so we can retrieve/print the buffer without change
      ---
      actually we may be in a yield().  so policy is handled externally.
    */
    buffer_push( data->buffer, &data->buffer_idx, data->computed_val );

    if(data->show_stats) {
      printf(" ");
      buffer_stats_print( data->buffer );
    }
  }

  else {
    printf( "-");
    // printf( "no cal or computed val\n");
  }


  printf("\n");


}


#if 0

void data_update_new_reading(data_t *data, uint32_t spi)
{
  assert(0);
  // TODO - get rid of this function.
  // move functionality higher.
  

  assert(data);
  assert(data->magic == DATA_MAGIC);


  /*
    race condition here,,
    adc may have produced a valid measure.
    but the sample acquisition was put into arm state by mcu
    So see how much we
    --
    this is simpler. and keeps dependencies better isolated, compared with having the sa_acquisition squash the valid signal.
  */


  // TODO - fix me,   factor this condition test out.
  // to avoid the nesting.
  if(data->adc_interupt_valid) {

    data->adc_interupt_valid = false;
    data_update_new_reading2( data, spi);
  }

  // TODO - i think we forgot to bring code across for this check
  // did we miss data, for any reason
  if( data->adc_interupt_valid_missed == true) {
    printf("missed data\n");
    data->adc_interupt_valid_missed = false;
  }


}

#endif








unsigned model_spec_cols( unsigned model_spec )
{

  switch(model_spec) {

    case 2: return 2;
    case 21: return 3;
    case 3: return 3;
    case 31: return 4;
    default:
      assert(0);
  }

  return -1; // compiler
}



/*

  rename   adc_counts_to_m similar.
  and make the model the first arg.
  REVIEW - perhaps move to cal. because the model that defines this, is determined in cal
*/

// rename counts_to_m.

MAT * run_to_matrix(
    // const Run *run,
    uint32_t clk_count_mux_neg,
    uint32_t clk_count_mux_pos,
    uint32_t clk_count_mux_rd,
    unsigned model_spec,
    MAT * out
)
{
  // change name ,   adc_counts_to_matrix() ?

/*
  EXTR.
    - try adding apperture as independent variable.
    try a model that includes aperture. ie. if there are small changes between nplc=1, nplc=10
    then perhaps just including aperture.

*/


  // TODO can we move this inside each if clause?
  if(out == MNULL)
    out = m_get(1,1);


  switch(model_spec) {

    case 2: {
      /*
        more constrained.
        rundown that has both currents on - just sums
        this is nice because doesn't require anything on fpga side.
      */
      out = m_resize(out, 1, 2);
      m_set_val( out, 0, 0,  clk_count_mux_neg + clk_count_mux_rd );
      m_set_val( out, 0, 1,  clk_count_mux_pos + clk_count_mux_rd  );
      break;
      }

    case 21: {
      out = m_resize(out, 1, 3);
      m_set_val( out, 0, 0,  1.f );   // ones, offset
      m_set_val( out, 0, 1,  clk_count_mux_neg + clk_count_mux_rd );
      m_set_val( out, 0, 2,  clk_count_mux_pos + clk_count_mux_rd  );
    break;
    }


    case 3: {
      out = m_resize(out, 1, 3);
      m_set_val( out, 0, 0,  clk_count_mux_neg );
      m_set_val( out, 0, 1,  clk_count_mux_pos );
      m_set_val( out, 0, 2,  clk_count_mux_rd );
      break;
    }

    case 31: {

      out = m_resize(out, 1, 4);
      m_set_val( out, 0, 0,  1.f ); // ones, offset
      m_set_val( out, 0, 1,  clk_count_mux_neg );
      m_set_val( out, 0, 2,  clk_count_mux_pos );
      m_set_val( out, 0, 3,  clk_count_mux_rd);
      break;
    }

    default:
      assert(0);
  }

#if 0
  else if( model_spec == 5) {

    out = m_resize(out, 1, 4);
    m_set_val( out, 0, 0,  x0 );
    m_set_val( out, 0, 1,  x1  );
    m_set_val( out, 0, 2,  x2  );
    m_set_val( out, 0, 3,  x3  ); // flip_count
  }
#endif



  return out;
}



MAT * m_calc_predicted( const MAT *b, const MAT *x, const MAT *aperture)
{
  /*
    - careful - this function may crash on embedded - due to memoryeneeds of m_mlt().
      actually it shouldn't be too bad - compared with decomposition.
    do matrix multiply, and adjust by the aperture.
  */

  // don't free input arguments
  // b is 4x1, x is nx4

  assert( m_cols(x) == m_rows( b) );
  assert( m_rows(x) == m_rows( aperture) );

  // matrix multiply
  MAT *predicted = m_mlt(x, b, MNULL );

  MAT *corrected = m_element_div( predicted, aperture, MNULL );

  M_FREE(predicted );

/*
  printf("corrected\n");
  m_foutput(stdout, corrected);
  usart1_flush();
*/

  return corrected;
}








bool data_repl_statement( data_t *data,  const char *cmd )
{
  assert(data);
  assert(data->magic == DATA_MAGIC);


  // prefix with data.  same as the struct, function prefix. eg.   'data cal'  'data buffer size x' ?
  uint32_t u0;


  // reserve or resize...

  if( sscanf(cmd, "data buffer size %lu", &u0 ) == 1) {

    // if(u0 < 2 || u0 > 500 ) {
    if(u0 < 1 || u0 > 10000 ) {
      printf("set buffer size bad arg\n" );
      return 1;
    }

    // set buffer size, efault
    data->buffer = buffer_reset( data->buffer, u0);

    assert( data->buffer);
    data_reset( data );
  }

  else if( strcmp(cmd, "data buffer reset") == 0) {

    // set buffer size, efault
    data->buffer = buffer_reset( data->buffer, 10 );
    assert( data->buffer);
    data_reset( data );
  }



/*
  else if( strcmp(cmd, "data buffer print")) {
    // dump all vals.
    // print/show?
  }
*/

  else if( sscanf(cmd, "data line freq %lu", &u0 ) == 1) {

    if(  !(u0 == 50 || u0 == 60)) {
      // be safe for moment.
      printf("bad line freq arg\n" );
      return 1;
    }

    // printf("set lfreq\n" );
    data->line_freq = u0;
  }

  else if(strcmp(cmd, "data null") == 0) {
    // todo
    // eg. very easy, but useful. add an offset based on last value

  }
  else if(strcmp(cmd, "data div") == 0) {
    // div by gain.before
    // should probably be gain and offset for the calibration.
    // todo
  }

  // could be called, 'buffer show stats', 'buffer show extra' etc.

  else if(strcmp(cmd, "data show counts") == 0)
    data->show_counts = 1;

  else if(strcmp(cmd, "data show extra") == 0)
    data->show_extra = 1;

  else if(strcmp(cmd, "data show stats") == 0)
    data->show_stats = 1;


  else if(strcmp(cmd, "data cal show") == 0) {


    data_cal_show( data );

  }

  else
    return 0;

  return 1;


}












#if 0

void data_update(data_t *data, uint32_t spi )
{
  /* called from main loop.
    eg. 1M / s.
  */

  assert(data);
  assert(data->magic == DATA_MAGIC) ;


/*
  static uint32_t count = 0;
  ++count;
  if(count > 1000000) {
    printf("%lu\n", count);
    count -= 1000000;
  } */

  if(data->adc_interupt_valid ) {

    // clear flag as first thing, in order to better catch missed data, if get interupt while still processing
    data->adc_interupt_valid  = false;

    // printf("got data\n");

    uint32_t clk_count_mux_reset  = spi_ice40_reg_read32( spi, REG_ADC_CLK_COUNT_REFMUX_RESET);   // time refmux is in reset. useful check. not adc initialization time.
    uint32_t clk_count_mux_neg    = spi_ice40_reg_read32( spi, REG_ADC_CLK_COUNT_REFMUX_NEG);
    uint32_t clk_count_mux_pos    = spi_ice40_reg_read32( spi, REG_ADC_CLK_COUNT_REFMUX_POS);
    uint32_t clk_count_mux_rd     = spi_ice40_reg_read32( spi, REG_ADC_CLK_COUNT_REFMUX_RD);
    uint32_t clk_count_mux_sig    = spi_ice40_reg_read32( spi, REG_ADC_CLK_COUNT_MUX_SIG);

  /*  - OK. it doesn't matter whether aperture is for one more extra clk cycle. or one less.  eg. the clk termination condition.
      instead what matters is that the count is recorded in the same way, as for the reference currents.
      eg. so should should always refer to the returned count value, not the aperture ctrl register.

      uint32_t clk_count_mux_sig = spi_ice40_reg_read32( app->spi, REG_ADC_P_APERTURE );
  */
    printf("counts %6lu %lu %lu %6lu %lu", clk_count_mux_reset, clk_count_mux_neg, clk_count_mux_pos, clk_count_mux_rd, clk_count_mux_sig);

    printf("\n");
  }
}

#endif











/*
  - we dont want to care about azmux values here.
  - we only want to know enough to decode the stream
  - so could pass a function, setup
  ---
  - or just adhere - to a convetinon.   eg. seq0 == lo. and seq1 == hi.
      that allows us to decode.
      if seq_idx == 0 -> lo
      if seq_idx == 1->  hi

      adc_seq_idxo

    sequence acquisition can set it. when we get the adc valid signal.

    seq_idx_last <= seq_idx;

*/




#if 0
    if ( m_cols(xs) != m_rows( data->b) ) {

      // calibtration sampled data, mismatch the cols mismatch.
      // shouldn't happen if arm is working.
      printf("m_cols(xs) != m_rows( b) \n");

      printf("app->cols   %u\n", data->model_spec );
      printf("app->b cols %u\n", m_cols( data->b ) );
      printf("app->b rows %u\n", m_rows( data->b ) );
      printf("xs     cols %u\n", m_cols( xs ) );
      printf("xs     rows %u\n", m_rows( xs ) );


      M_FREE( xs );
      M_FREE( m_mux_sig );
      return;
    }
#endif


  /*  - OK. it doesn't matter whether aperture is for one more extra clk cycle. or one less.  eg. the clk termination condition.
      instead what matters is that the count is recorded in the same way, as for the reference currents.
      eg. so should should always refer to the returned count value, not the aperture ctrl register.

      uint32_t clk_count_mux_sig = spi_ice40_reg_read32( spi, REG_ADC_P_APERTURE );
  */



#if 0 // JA
  // suppress late measure samples arriving after signal_acquisition is returned to arm
  if( ! (status & STATUS_SA_ARM_TRIGGER)) {
    /*
        this is done in software. and can only be done in software - because there is a race- condition.
        that adc can generate the obs - in the time that we write the arm/trigger register for signal acquisition.
    */
    return;
  }
#endif

  /*
      -a consider adding a 8 bit. counter in place of the monitor, in the status register
      in order to check all values are read in a single transaction
      - or else a checksum etc.
      --------

      Do we expose mode here....
      Ideally NO.

      we can encode seqn in the status register.  to alleviate another call.
      and use that to determine
  */

#if 0
    Mode *mode = app->mode_current;

    if(mode->reg_mode == MODE_NO_AZ )  {

      if(app->verbose) {
        printf(" no-az (%s)", azmux_to_string( mode->reg_direct.azmux));

      }
    }
    else if(mode->reg_mode == MODE_AZ)  {

      if(app->verbose)
        printf(" az");

      // determine if az obs high or lo
      if( status & STATUS_SA_AZ_STAMP  ) {
        // treat as hival
        if(app->verbose)
          printf(" (hi %s)", himux_to_string( mode->reg_direct.himux, mode->reg_direct.himux2 ));
        app->hi = ret;
      }
      else {
        // treat as lo val
        if(app->verbose)
          printf(" (lo %s)", azmux_to_string( mode->reg_direct.azmux));

        app->lo[ 1] = app->lo[ 0];  // shift last value
        app->lo[ 0] = ret;
      }

      if(app->verbose) {
        printf(" (hi %sV)",  format_float_with_commas(buf, 100, 7, app->hi ));
        printf(" (lo %sV",   format_float_with_commas(buf, 100, 7, app->lo[0]  ));
        printf(", %sV)",  format_float_with_commas(buf, 100, 7, app->lo[1] ));
      }

      // regardless whether we got a lo or a hi. calculate and show a new value.
      ret = app->hi - ((app->lo[ 0 ] + app->lo[1] ) / 2.0);
    }
    else {
        printf(" unknown mode");
    }

#endif



#if 0
    if(m_rows(app->sa_buffer) < m_rows_reserve(app->sa_buffer)) {

      // just push onto sample buffer
      m_push_row( app->sa_buffer, & ret , 1 );
    }

    else {
      // buffer is full, so insert
      // TODO there's an issue with modulo overflow/wrap around.

      unsigned idx = app->sa_count_i++ % m_rows(app->sa_buffer);
      // printf(" insert at %u\n", idx );
      m_set_val( app->sa_buffer, idx, 0,  ret );
    }

    if(app->verbose) {
      printf(" ");
      buffer_stats_print( app->sa_buffer );
    }
#endif

