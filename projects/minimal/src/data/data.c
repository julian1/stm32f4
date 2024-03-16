
#include <stdio.h>
#include <assert.h>


#include <data/data.h>


#include <ice40-reg.h>
#include <peripheral/spi-ice40.h>






void data_init ( data_t *data )
{
  assert(data);
  assert(data->magic == DATA_MAGIC) ;


  // not sure if really even need this. 
  printf("whoot data init!\n");
}


void data_rdy_interupt( data_t *data) // runtime context
{
  /* interupt context.  don't do anything compliicated here.
    but called relatively infrequent.
  */

  assert(data);
  assert(data->magic == DATA_MAGIC) ;


  // if flag is still active, then record we missed processing some data.
  if(data->adc_measure_valid == true) {
    data->adc_measure_valid_missed = true;
    // ++data->adc_measure_valid_missed;     // count better? but harder to report.
  }

  // set adc_measure_valid flag so that update() knows to read the adc...
  data->adc_measure_valid = true;
}




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


  if(data->adc_measure_valid ) {


    // clear flag as first thing, in order to better catch missed data, if get interupt while still processing
    data->adc_measure_valid  = false;

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



/*
  also
 change name row_to_matrix() to   adc_counts_to_row() or similar.
  model argument should probably go first.
  and this isn't a function that takes data_ as arg. so more a helper, and perhaps belong in different func

*/


MAT * run_to_matrix(
    // const Run *run,
    uint32_t clk_count_mux_neg,
    uint32_t clk_count_mux_pos,
    uint32_t clk_count_mux_rd,
    unsigned model,
    MAT * out
)
{
  // change name ,   adc_counts_to_matrix() ?


  /*
    we have aperture stored in the Param.
    - it's easy to pull off device.  so perhaps it should be moved to Run. or else store in both Run and Param.
    - OR. it should always be passed here. - because it is a fundamental data on device, and for calculating predicted..
    ------
    - not sure. we want to test a model with aperture as independent var.

  */

  // TODO can we move this inside each if clause?
  if(out == MNULL)
    out = m_get(1,1);


  if(model == 2) {
    /*
      more constrained.
      rundown that has both currents on - just sums
      this is nice because doesn't require anything on fpga side.
    */
    double x0_ = clk_count_mux_neg + clk_count_mux_rd;
    double x1_ = clk_count_mux_pos + clk_count_mux_rd;

    out = m_resize(out, 1, 2);
    m_set_val( out, 0, 0,  x0_ );
    m_set_val( out, 0, 1,  x1_  );
  }

  else if( model == 3) {

    out = m_resize(out, 1, 3);
    m_set_val( out, 0, 0,  clk_count_mux_neg );
    m_set_val( out, 0, 1,  clk_count_mux_pos );
    m_set_val( out, 0, 2,  clk_count_mux_rd );
  }

/*
  EXTR.
    - try adding apperture as independent variable.
    try a model that includes aperture. ie. if there are small changes between nplc=1, nplc=10
    then perhaps just including aperture.

*/
  else if ( model == 4) {

    out = m_resize(out, 1, 4);
    m_set_val( out, 0, 0,  1.f ); // ones, offset
    m_set_val( out, 0, 1,  clk_count_mux_neg );
    m_set_val( out, 0, 2,  clk_count_mux_pos );
    m_set_val( out, 0, 3,  clk_count_mux_rd);
  }

#if 0
  else if( model == 5) {

    out = m_resize(out, 1, 4);
    m_set_val( out, 0, 0,  x0 );
    m_set_val( out, 0, 1,  x1  );
    m_set_val( out, 0, 2,  x2  );
    m_set_val( out, 0, 3,  x3  ); // flip_count
  }
#endif


  else assert( 0);

  return out;
}







