/*
    it is good to code as repl, because it tests the repl. also.


  - it's not completely clear who should determine the yield function.
    eg. the caller passing it as a dependency of callee?
    but the callee knows what it needs . so could set up itself.
    in any case it doesn't really matter.

*/


#include <stdio.h>
#include <assert.h>
#include <string.h>   // strcmp


#include <app.h>
#include <data/data.h>
#include <data/matrix.h>  // m_rows()
// #include <data/buffer.h>

#include <lib2/util.h>    // yield_with_msleep

#include <mode.h>       // transition state
#include <peripheral/trigger.h>      // trigger


#define UNUSED(x) (void)(x)


static void fill_buffer( app_t *app, MAT *buffer, size_t sz, void (*yield)( void *), void *yield_ctx)
{
  assert(buffer);
  /*
    we probably don't need to pass app here.
  */

  data_t *data = app->data;

  // start acquisition, generating interupts, which sets data ready flags, which we ignore for the moemnt. - with trig
  printf("change state and buffer\n");

  // swap the buffer to use
  data->buffer  = buffer;

  // we need to toggle the trigger/ reset of sa controller. to get clean values.
  // we should do  this via the register.
  app->mode_current->trig_sa = 1;
  spi_mode_transition_state( app->spi, app->spi_4094, app->spi_ad5446, app->mode_current, &app->system_millis);

  // reset the reading values buffer, no value will be recorded before this is filled.
  data_reading_reset( data );

  // sleep?
  // yield_with_msleep( 1 * 100, &app->system_millis, yield, yield_ctx);


  printf("waiting for data\n");

  size_t sz_now = m_rows( data->buffer);
  // wait until have extra vals
  while( m_rows(data->buffer ) < sz_now + sz ) {
    yield( yield_ctx);
  }

  // potential race condition here, we don't want to collect spurious data.

  ice40_port_trig_sa_disable();

  // print output
  // m_foutput( stdout, data->buffer );
}




/* A is +10V. B is tap wrt gnd,   C is Gnd.
// C is gnd referenced.
*/

// remember to read right to left, for the bitwise encoding for the mux
// could also name this as a 3-way select.  like S1-8, or W1-4.  eg.
#define A 0b11
#define B 0b10
#define C 0b00



static void negative_side( 
  app_t *app ,
  void (*yield)( void *),
  void *yield_ctx
) 
{

  UNUSED(app);
  UNUSED(yield);
  UNUSED(yield_ctx);

    unsigned rows = 3;

    MAT *ba =  m_get(  rows * 3, 1 );   // rows x cols
    MAT *cb =  m_get(  rows * 3, 1 );   // rows x cols
    MAT *ca =  m_get(  rows * 3, 1 );   // rows x cols
    MAT *bb =  m_get(  rows * 3, 1 );   // rows x cols

    m_truncate_rows( ba, 0);
    m_truncate_rows( cb, 0);
    m_truncate_rows( ca, 0);
    m_truncate_rows( bb, 0);

    for( unsigned i = 0; i < rows; ++i ) {

      // JA disabled july 2024.
      assert(0);
/*
      printf("------\nb-a\n");
      app->mode_current->second.U1010 = (A << 2) | B ;      // B-A,  TAP-10V
      fill_buffer( app, ba, 3, yield, yield_ctx) ;

      printf("------\nc-b\n");
      app->mode_current->second.U1010 = (B << 2) | C ;      // C-B,  GND-TAP
      fill_buffer( app, cb, 3, yield, yield_ctx) ;

      printf("------\nc-a\n");
      app->mode_current->second.U1010 = (A << 2) | C ;      // C-A, GND-10V
      fill_buffer( app, ca, 3, yield, yield_ctx) ;

      printf("------\b-b\n");
      app->mode_current->second.U1010 = (B << 2) | B ;      // B-B, TAP-TAP. on dcv-source-1 and dcv-source-com, from different inputs
      fill_buffer( app, bb, 3, yield, yield_ctx) ;
*/
    }

    assert(m_rows(ba) == m_rows(cb));
    assert(m_rows(ba) == m_rows(ca));
    assert(m_rows(ba) == m_rows(bb));

    printf("------\nb-a\n"); m_foutput(stdout, ba );
    printf("------\nc-b\n"); m_foutput(stdout, cb );
    printf("------\nc-a\n"); m_foutput(stdout, ca );

    // actually rather than generate a new matrix. just calculate in output loop
    // double diff = ba + cb - ca;   // store
    MAT *sum =  m_element_add(ba, cb, MNULL);
    MAT *diff = m_element_sub( sum, ca, MNULL);

    printf("diff\n");
    m_foutput(stdout, diff);

    printf("-------\n");

    // idea is to dealay preprocessing  the data.
    // loop rows and print data.
    for(unsigned i = 0; i < m_rows(ba); ++i ) {

      printf("%u", 1  );                              // group id . needs
      printf(", %.7f",  m_get_val( cb, i, 0)  );      // eg. tap voltage. for plotting.  we will need to round this to be the same across voltages.. no . no need to round
      printf(", %.7f", m_get_val( diff, i, 0) );      // diff
      printf(", %.7f", m_get_val( bb, i, 0)   );       // bb for ref
      printf(",\n");
    }

    // also restore the buffer
    // free ab , ba  etc.
    // MFREE(sum);.
    // MFREE(diff);.
}







static void positve_side( 
  app_t *app ,
  void (*yield)( void *),
  void *yield_ctx
) 
{

  UNUSED(app);
  UNUSED(yield);
  UNUSED(yield_ctx);

    unsigned rows = 3;

    MAT *ab =  m_get(  rows * 3, 1 );   // rows x cols
    MAT *bc =  m_get(  rows * 3, 1 );   // rows x cols
    MAT *ac =  m_get(  rows * 3, 1 );   // rows x cols
    MAT *bb =  m_get(  rows * 3, 1 );   // rows x cols

    m_truncate_rows( ab, 0);
    m_truncate_rows( bc, 0);
    m_truncate_rows( ac, 0);
    m_truncate_rows( bb, 0);

    for( unsigned i = 0; i < rows; ++i ) {

    // JA disable july 2024.
    assert(0);

/*
      printf("------\na-b\n");
      app->mode_current->second.U1010 = (B << 2) | A ;      // A-B,  10V -TAP
      fill_buffer( app, ab, 3, yield, yield_ctx) ;

      printf("------\nb-c\n");
      app->mode_current->second.U1010 = (C << 2) | B ;      // B-C,  TAP-GND
      fill_buffer( app, bc, 3, yield, yield_ctx) ;

      printf("------\na-c\n");
      app->mode_current->second.U1010 = (C << 2) | A ;      // A-C, 10V-GND
      fill_buffer( app, ac, 3, yield, yield_ctx) ;

      printf("------\b-b\n");
      app->mode_current->second.U1010 = (B << 2) | B ;      // B-B, TAP-TAP. on dcv-source-1 and dcv-source-com, from different inputs
      fill_buffer( app, bb, 3, yield, yield_ctx) ;
*/
    }

    assert(m_rows(ab) == m_rows(bc));
    assert(m_rows(ab) == m_rows(ac));
    assert(m_rows(ab) == m_rows(bb));

    printf("------\nb-a\n");
    m_foutput(stdout, ab );

    printf("------\nc-b\n");
    m_foutput(stdout, bc );

    printf("------\nc-a\n");
    m_foutput(stdout, ac );

    // actually rather than generate a new matrix. just aclculate in output loop
    // double diff = ab + bc - ac;   // store
    MAT *sum =  m_element_add(ab, bc, MNULL);
    MAT *diff = m_element_sub( sum, ac, MNULL);

    printf("diff\n");
    m_foutput(stdout, diff);

    printf("-------\n");

    // idea is to dealay preprocessing  the data.
    // loop rows and print data.
    for(unsigned i = 0; i < m_rows(ab); ++i ) {

      printf("%u", 1  );                              // group id . needs
      printf(", %.7f",  m_get_val( bc, i, 0)  );      // eg. tap voltage. for plotting.  we will need to round this to be the same across voltages.. no . no need to round
      printf(", %.7f", m_get_val( diff, i, 0) );      // diff
      printf(", %.7f", m_get_val( bb, i, 0)   );       // bb for ref
      printf(",\n");
    }

    // also restore the buffer
    // free ab , ab  etc.
    // MFREE(sum);.
    // MFREE(diff);.
}










bool app_test42(
  app_t *app,
  const char *cmd,
  void (*yield)( void *),
  void *yield_ctx
) {
  assert(app);
  assert(app->magic == APP_MAGIC);
  assert(cmd);
  assert(yield);
  assert(yield_ctx);

  data_t *data = app->data;
  assert(data);
  assert(data->magic == DATA_MAGIC);

  ////////////////////

  /*
    It is easier to think about as A-B,   etc.
    it is just the bytecode ordering when passed to the mux that needs to be swapped
  */

  int32_t d0;

  if( sscanf(cmd, "test42 %ld", &d0 ) == 1) {
  // if( strcmp(cmd, "test42") == 0) {


    if( !data->model_b) {
      // avoid loading deafult
      printf("no cal model - loading one\n");
      return 1;
    }

    // setup
    app_repl_statements(app, "                \
        reset;                                \
        dcv-source header;     \
        set k407 0;   set k405 1;             \
        set lomux s1;                         \
        nplc 10; set mode 7 ; azero s3 s8;    \
        data buffer size 5                    \
        data show stats;  trig;               \
      " );

    if(d0 == 1) 
      positve_side( app, yield, yield_ctx); 
    else if(d0 == -1) 
      negative_side( app, yield, yield_ctx); 
    else {
      printf("bad argument\n");
      return 1;
    }


    app->mode_current->trig_sa = 0;

    return 1;
  }




  return 0;
}


/*

    // actually rather than generate a new matrix. just calculate in output loop
    // double diff = ba + cb - ca;   // store
    MAT *tmp = 0;
    MAT *tmp2 = 0;
    MAT *diff = m_element_sub( m_element_add(ba, cb, tmp), ca, tmp2 );    // this leaks. because tmp will still be NULL after the call.
    assert(tmp == NULL);  // issue. so we cannot free it.





    // ok. so we have everything and should be able to output the data ok.

    // ok. for simpler formatting.
    // we ought to be able to take the B measurement - and just emit a digit.
    // actually just store b. then we can do in any order
#if 0
    for( unsigned i = 0; i < 5; ++i ) {

      printf(", %u",    1  );                   // group id . needs
      printf(", %.1f",  ar_tap[i ] );            // eg. tap voltage. for plotting.  ar_ba or
      printf(", %.2lf", ar_diff[i ] * 1e6);     // diff
      printf(", %.2lf", ar_bb[i ] * 1e6);       // bb for ref

      // printf("diff %.2lfuV", ar_diff[i ] * 1e6);
      // printf(", bb %.2lfuV", ar_bb[i ] * 1e6);
      printf("\n");
    }
#endif


    // check_data( == 7.000 )  etc.




mar 30.

  get a baseline.  in both directions.  and plot
  - then fix gnd current comp.
  - and remove extra mux. and jumper
  - and try two var cal.

> data cal show
Matrix: 3 by 1
row 0:     17.4986934
row 1:    -17.9358312
row 2:   -0.458200302
model_id    0
model_cols  3
stderr(V)   0.86uV  (nplc10)
res         0.115uV  digits      7.94 (nplc 10)calling spi_mode_transition_state()

positive
double diff = ab + bc - ac;   // store

4.8 tap.

diff -3.85uV, bb -0.87uV
diff -2.69uV, bb -0.10uV
diff -1.94uV, bb -1.07uV
diff -4.47uV, bb -1.05uV
diff -4.15uV, bb -1.44uV

2.4
diff -4.83uV, bb -1.23uV
diff -4.02uV, bb -1.44uV
diff -4.65uV, bb -0.38uV
diff -3.56uV, bb -1.56uV
diff -5.22uV, bb -1.81uV

7.2
diff -3.43uV, bb -1.91uV
diff -4.30uV, bb -0.62uV
diff -4.08uV, bb -1.23uV
diff -3.28uV, bb -0.55uV
diff -3.97uV, bb -1.15uV

3.6
diff -7.96uV, bb -1.17uV
diff -4.89uV, bb -0.93uV
diff -4.98uV, bb -0.92uV
diff -6.19uV, bb -0.55uV
diff -6.13uV, bb -0.87uV

6.0
diff -5.18uV, bb -0.64uV
diff -6.04uV, bb -1.40uV
diff -5.46uV, bb -0.85uV
diff -5.01uV, bb -1.10uV
diff -5.35uV, bb -1.39uV

1.2
diff -6.36uV, bb -0.92uV
diff -4.58uV, bb -1.28uV
diff -6.08uV, bb -1.19uV
diff -6.07uV, bb -1.43uV
diff -5.10uV, bb -1.00uV

8.4
diff -4.53uV, bb -1.71uV
diff -6.24uV, bb -1.84uV
diff -4.71uV, bb -1.20uV
diff -5.75uV, bb -1.18uV
diff -5.16uV, bb -0.79uV


negative
double diff = ba + cb - ca;   // store

4.8 tap.
diff 3.78uV, bb -1.21uV
diff 2.69uV, bb -0.68uV
diff 2.47uV, bb -1.16uV
diff 1.85uV, bb -1.10uV
diff 1.84uV, bb -0.95uV

2.4
diff 0.55uV, bb -0.52uV
diff 0.23uV, bb -1.07uV
diff 1.44uV, bb -0.81uV
diff -0.18uV, bb -0.60uV
diff -0.30uV, bb -1.66uV

7.2
diff 0.75uV, bb -1.79uV
diff 1.09uV, bb -0.68uV
diff 1.62uV, bb -1.83uV
diff 0.68uV, bb -1.46uV
diff -0.13uV, bb -1.18uV

3.6
diff 0.98uV, bb -1.52uV
diff -1.14uV, bb -0.69uV
diff -1.20uV, bb -1.49uV
diff -1.87uV, bb -1.11uV
diff -2.22uV, bb -1.00uV

6.0
diff 0.35uV, bb -1.07uV
diff -2.53uV, bb -1.71uV
diff -2.50uV, bb -1.29uV
diff -1.04uV, bb -1.65uV
diff -2.38uV, bb -1.60uV

1.2
diff -2.67uV, bb -1.92uV
diff -2.61uV, bb -1.08uV
diff -1.54uV, bb -1.24uV
diff -1.71uV, bb -1.71uV
diff -1.95uV, bb -0.48uV

8.4
diff -1.57uV, bb -0.60uV
diff -3.30uV, bb -0.62uV
diff -2.36uV, bb -1.16uV
diff -3.39uV, bb -1.81uV
diff -3.81uV, bb -1.18uV





---------------------
For sum-tests,
I spent quote some time trying to get two series 10u film caps to work.
This included a lot of over-engineered muxing - for cap selection, and polarity, and to be able to charge to different spot voltages.
But I couldn't avoid a constant leakage of -2uV/s probably to the negative rail (probably due to 0.65" ssop dpdt mux package).
In the past I used relays, but that would be too cumbersome for a single board.

Trying the battery approach,
8x 1.2V enneloup batteries in a battery-holder with taps, switched manually
Method - is sample AB for 10 readings, 10nplc, then BC (bottom half) , then AC (series ), take the means, and calculate the diff/delta.
repeat 5 times.
eg. diff = 4.8V + 4.8V - 9.6V


After reducing resolution, change series rundown bias-resistor from 220R to 1k. and new cal.

> data cal show
Matrix: 3 by 1
row 0:     17.4986934
row 1:    -17.9358312
row 2:   -0.458200302
model_id    0
model_cols  3
stderr(V)   0.86uV  (nplc10)
res         0.115uV  digits      7.94 (nplc 10)

4.8
diff -3.50uV
diff -5.12uV
diff -4.66uV
diff -4.04uV
diff -1.76uV

2.4
diff -5.39uV
diff -4.11uV
diff -3.88uV
diff -0.94uV
diff -3.44uV

7.2
diff -5.95uV
diff -2.77uV
diff -3.09uV
diff -4.99uV
diff -2.00uV

3.6
diff -4.51uV
diff -6.62uV
diff -5.07uV
diff -4.34uV
diff -1.98uV


6.0
diff -8.43uV
diff -7.10uV
diff -3.93uV
diff -7.28uV
diff -2.49uV

6.0 repeat.
diff -8.09uV
diff -4.13uV
diff -3.95uV
diff -3.43uV
diff -4.59uV


1.2
diff -1.94uV
diff -2.22uV
diff 0.08uV
diff -2.07uV
diff -4.10uV

8.4
diff -3.94uV
diff -1.96uV
diff -2.35uV
diff -2.66uV
diff -2.43uV

I've only just got this working, and am not quite sure what to make of the offset.
Probably it would be good to try the negative polarity, and I would like to experiment more with a two-variable weighting model for the adc reference currents.

The board includes footprints for 8, and 10pin mdacs, for creating +- spot voltage and are working,
these might be used to test inl in a sum-type ratio mode, through a polarity flip.
But I forgot to add a resistor divider, which would need to be bodged.
And I don't like the idea of lower-impedance source, as one cannot buffer the divider since the buffer Vos will not invert through the polarity refernce voltage flip.
The mdacs look to be reasonbly low noise as far as I can tell.








  -------------
  after doing another cal.

  10nplc.  sample n=10, for each of AB (4.8V), BC (4.8V), AC (9.6V).
  diff -4.06uV
  diff -3.74uV
  diff -4.70uV
  diff -4.84uV
  diff -4.97uV
  diff -3.40uV

    at least it's stable.
    waitin

----

  mar 27.
    after upgrading ltz1000.
  1nplc.
    diff -1.67uV
    diff -5.79uV
    diff -9.30uVo
    diff -5.10uV
    diff -5.46uV

    doesn't look very good.

  10 nplc
    diff -2.06uV
    diff -3.51uV
    diff -3.00uV
    diff -2.43uV
    diff -3.52uV

      Ok. so noise is down a lot kkkkkk

  mar 26.

  Second attempt - is 8x 1.2V enneloup batteries in a battery-holder with centre tap, for the middle voltage.

  The cal done a few days ago, so there may be some temp drift on the cal coefficients.
  There seems to be some noise, which I think is from the adc ref - currently lt1021.
  So several runs are kind of needed to get any sense of it.

  Method - is to sample AB (top half battery pack) for 10 readings, then BC (bottom half) , then AC (series ), take the means, and calculate the diff/delta.
  eg. diff = 4.8V + 4.8V - 9.6V

  10nplc, sample n=10, azmode
  diff -3.97uV
  diff -2.53uV
  diff -1.90uVo
  diff -2.50uV
  diff -0.09uV
  diff -4.44uV

  1nplc, sample n=10, azmode
  diff 3.09uV
  diff -1.50uV
  diff 0.79uV
  diff 4.45uV
  diff 1.70uV
  diff -0.63uV
  diff 0.88uV

  So there's quite a bit of noise, and upgrading the adc ref may be the next step.
  It may make sense to do more interleaving for the different voltage more, althouth it needs all the serial peripheral state on the board to update.


*/





#if 0
    // az sample ref-hi on ch1, via the low mux, and ref-lo should be 7.000,000V.

    /* note, there's real confusion - with in order, and out of order repl statements.
      eg. flash cal read, and data show stats etc will be done in sequence, while mode update is out of bound
      we want to do the data_reset after the adc is running.
    */


    app_repl_statements(app, "        \
        flash cal read 123;           \
        reset;                        \
        dcv-source ref-lo;            \
        set k407 0;  set k405 1;      \
        set lomux s1;                 \
        nplc 10; set mode 7 ; azero s3 s8;  halt; \
        data show stats;              \
        data buffer size 30;          \
      " );

    // cal
    spi_mode_transition_state( app->spi, app->mode_current, &app->system_millis);

    // data->buffer = buffer_reset( data->buffer, 30);     // resise buffer
    // data_reset( data );                                 // reset

    // note - we could set the buffer, etc. and then do the trigger later.

    ice40_port_trig_sa_enable();    // rename set/clear() ? better?
#endif


