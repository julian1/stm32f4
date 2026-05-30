
/*
  decode counts, use cal to calculate reading, and assign range

*/




#include <stdio.h>
#include <assert.h>
#include <string.h>     // memcpy




#include <peripheral/spi-ice40.h>
#include <device/spi-fpga0-reg.h>   // for seq mode


#include <lib3/util.h>      // UNUSED
#include <lib3/format.h>    // format_float


#include <data/cal.h>
#include <data/data.h>
#include <data/range.h>
#include <ranging.h>
#include <environment.h>


#include <data/decode.h>

#include <mode.h>   // sa_state_t

#include <support.h>  // char * str_from_mux( char *buf, size_t n, unsigned val);



/*
  EXTR.
  can pass data_t to the update() function.
  rather than static injecting on construction.
  --------------
*/






static void decode_update_data_conversion( decode_t *decode,  data_t *data  )
{

  // consider change name data_conversion()...

  assert( decode && decode->magic == DECODE_MAGIC);


  spi_t *spi = decode->spi;
  assert( spi);


  reg_sr_t  status = data->status;

  assert( status.isr.adc) ;

  const cal_t *cal = data->cal;
  assert( cal && cal->magic == CAL_MAGIC);


  // record for current part of reading
  // cal w. needs this data
  data->adc_refmux_pos = spi_ice40_reg_read32( spi, REG_ADC_CLK_COUNT_REFMUX_POS);
  data->adc_refmux_neg = spi_ice40_reg_read32( spi, REG_ADC_CLK_COUNT_REFMUX_NEG);
  data->adc_sigmux     = spi_ice40_reg_read32( spi, REG_ADC_CLK_COUNT_SIGMUX);


  // useful for bounds - and to correct asymetry
  data->ratio_refmux =
      (data->adc_refmux_pos >= data->adc_refmux_neg)
      ?  (double) data->adc_refmux_pos / data->adc_refmux_neg
      :  - (double) data->adc_refmux_neg / data->adc_refmux_pos;




  if( true || decode->show_counts) {

    printf( "{counts pos %7lu neg %7lu sig %7lu}, ",
      data->adc_refmux_pos,
      data->adc_refmux_neg,
      data->adc_sigmux
    );
    // printf( "ratio %.2f, ", ratio);
  }

  // printf(" sigmux %lu ", data->adc_sigmux);


  /*
    delegate decode

    assumes that the current mode has valid sample acquisition handler.
    potential synchronization issue repl update of mode?
    but we always write state to the board. after setting  the mode. so should be ok.

  */
  const sa_state_t *sa = &decode->mode->sa;
  assert( sa);
  assert( sa->decode_ctx);
  assert( sa->decode_strategy);
  assert( sa->decode_ctx);


  // convert counts to count_sum
  sa->decode_strategy( sa->decode_ctx, data );



  if( data->reading_valid) {

    // can get an exact zero, due to quantization
    // assert( data->count_sum);

    if(decode->show_counts)
      printf("sum %.2f, ", data->count_sum);


    // normalize count
    data->count_sum_norm = data->count_sum  / data->adc_sigmux;


    // nominal reading
    // data->reading_nominal = cal->b * data->count_sum_norm;
    // double reading_nominal = cal->b * data->count_sum_norm;



    // range should already be set.
    const range_t *range = data->range;
    assert(range && range->magic == RANGE_MAGIC);

    // with range
    data->reading = range->range_reading_convert( range, cal, data->count_sum_norm);


    char buf[100 + 1];

    if(decode->show_reading) {

      // printf( "norm %s, ", str_format_float_with_commas(buf, 100, 8, data->reading_nominal));
      // printf( "%s-%s, ", range->name, range->arg );

      printf( "read %s ", str_format_float_with_commas(buf, 100, 8, data->reading));

      printf( "%s,", data->reading > range->fs ? "OL" : "  ");


      // printf( "%s, ", range->unit );
      // printf( "%s, ", range ? range->unit : ""  );
    }

  } // data->valid


}



/*
  - consider add a short (8 bit) transaction id field to the status register.
    - to check consistency of register reads
*/




#define BIT_TO_CHAR(a) ((a) ? '1' : '0')


static void print_term( const term_t *term)
{
  assert( term);

  char buf[ 100];

  printf( "{");
  // printf( "azmux %2u(%s), ",  term->azmux, str_from_mux( buf, 100, term->azmux));
  // printf( "azmux %s (%2u), ", str_from_mux( buf, 100, term->azmux), term->azmux);
  printf( "azmux %s, ",       str_from_mux( buf, 100, term->azmux));
  printf( "pc_protect %s, ",  str_format_bits( buf, 2, term->pc_protect));
  printf( "pc_sample %s, ",   str_format_bits( buf, 2, term->pc_sample));
  printf( "next-idx %u, ",    term->next_idx );
  printf( "oob %c, ",         BIT_TO_CHAR( term->oob_aperture));
  printf( "zglc %c, ",        BIT_TO_CHAR( term->zgjc));
  printf( "dither %c ",       BIT_TO_CHAR( term->cm_dac_dither));
  printf( "}, ");

  // printf( "hi %c ",           BIT_TO_CHAR( term->hi));
  // printf( "convert %c ",      BIT_TO_CHAR( term->convert));
}



static void print_term_brief( const term_t *term)
{
  assert( term);

  char buf[ 100];

  printf( "{");
  // printf( "azmux %2u(%s), ",  term->azmux, str_from_mux( buf, 100, term->azmux));
  printf( "azmux %s, ",       str_from_mux( buf, 100, term->azmux));
  printf( "oob %c, ",         BIT_TO_CHAR( term->oob_aperture));
  printf( "zglc %c ",         BIT_TO_CHAR( term->zgjc));
  printf( "}, ");
}



static void print_status_cmpr( const reg_sr_t status)
{

  char buf[100];

  snprintf( buf, 100, "{%c%c %c%c %c%c %c%c %c%c}, ",

    BIT_TO_CHAR( status.cmpr.amp_zero_lt),
    BIT_TO_CHAR( status.cmpr.amp_zero_gt),

    BIT_TO_CHAR( status.cmpr.amp_ovld_lt),
    BIT_TO_CHAR( status.cmpr.amp_ovld_gt),

    BIT_TO_CHAR( status.cmpr.amp_unld_lt),
    BIT_TO_CHAR( status.cmpr.amp_unld_gt),

    BIT_TO_CHAR( status.cmpr.boot_ch1_lt),
    BIT_TO_CHAR( status.cmpr.boot_ch1_gt),

    BIT_TO_CHAR( status.cmpr.boot_ch2_lt),
    BIT_TO_CHAR( status.cmpr.boot_ch2_gt)

  );


  // we no longer have a concept of whether a conversion is hi/lo. here.
  // so don't suppress printing the lo comparator vals
  printf( buf );

/*

  if( term.hi) {

    printf( buf );
  } else {

    // ignore for LO
    // just use pad spaces
    printf("%*s", strlen( buf), "");
  }
*/
}






void decode_update_data( decode_t *decode,  data_t *data  /* range_t *range */ )
{

  assert( decode && decode->magic == DECODE_MAGIC);


  assert( data && data->magic == DATA_MAGIC);

  spi_t *spi = decode->spi;
  assert( spi);

  /////////////////
  // copy environment fields first

  const cal_t *cal = decode->cal;
  assert( cal && cal->magic == CAL_MAGIC);

  const range_t *range = ranging_range_active_get( decode->ranging);
  assert(range && range->magic == RANGE_MAGIC);

  const environment_t *environment = decode->environment;
  assert( environment && environment->magic == ENVIRONMENT_MAGIC);

  /*
    only place/juncture where ranging active range . is
    used to stamp the data_t.

  */


  data->range     = range;
  data->cal       = cal;

  // could just set a copy of the environment...
  data->line_freq = environment->line_freq;


  /////////////////
  // now sa/adc


  // read the status, read into data->status
  _Static_assert ( sizeof( data->status) == 4);
  spi_ice40_reg_read_n( spi, REG_SR, &data->status, sizeof( data->status));


  // ease syntax
  const reg_sr_t  status = data->status;
  assert( status.isr.magic  == 0b1010 );
  assert( status.isr.adc );  // adc now the only interrupt source
  assert( !status.isr.cmpr );


  // read the seq-elt
  _Static_assert ( sizeof( data->term) == 4);
  spi_ice40_reg_read_n( spi, REG_SA_TERM_ELT, &data->term, sizeof( data->term));

  // east syntax
  // const term_t       term = data->term;



  printf( "%s-%s, ", range->name, range->arg );

  ////////////////



  /*
    printf( "{isr %c%c}, ",
      BIT_TO_CHAR( status.isr.cmpr),
      BIT_TO_CHAR( status.isr.adc)
    );
  */

  printf( "{idx=%u, first=%c}, ",
    status.sample.idx,
    BIT_TO_CHAR( status.sample.first)
  );

  // print_status_cmpr( status );

  // print_term( &data->term );

  print_term_brief( &data->term );



  // adc conversion
  decode_update_data_conversion( decode,  data);


}







bool decode_repl_statement( decode_t *decode,  const char *cmd)
{
  assert( decode);
  assert( decode->magic == DECODE_MAGIC);


  if(strcmp(cmd, "decode show") == 0
    || strcmp(cmd, "decode show") == 0)
    decode->show_counts = true;

  else if(strcmp(cmd, "decode unshow") == 0
    || strcmp(cmd, "decode unshow") == 0)
    decode->show_counts = false;

  else
    return 0;

  return 1;
}





void decode_init(

  decode_t        *decode,
  spi_t           *spi,
  const cal_t     *cal,
  const _mode_t   *mode,

  const ranging_t	*ranging,
  const environment_t *environment
  // uint32_t        *line_freq
)
{
  // called once at initialization

  assert( decode);
  assert( ranging);
  assert(cal && cal->magic == CAL_MAGIC);

  *decode = (const decode_t) {

    .magic        = DECODE_MAGIC,
    .spi          = spi,
    .cal          = cal,    // correct. like a singleton.
    .mode         = mode,

    .ranging      = ranging,
    .environment = environment,

    // .line_freq    = line_freq,

    .show_counts  = true,
    .show_reading = true,
  };

}





#if 0

nplc 1/ aggregate 10  - actually worse. interesting
STAR-LO-10, {idx=2, first=0}, {azmux s6, oob 0, zglc 1 }, {counts pos  198434 neg  203381 sig  400001}, (hi first) hi
STAR-LO-10, {idx=3, first=0}, {azmux s6, oob 0, zglc 0 }, {counts pos  198433 neg  203380 sig  400001}, (hi first) lo read -0.000,000,87 mean -0.000,000,03 (n=100/100), stddev +190.9n



may 28.

buffer n == 100

nplc 10
DCV-10, {idx=2, first=0}, {azmux s1, oob 0, zglc 1 }, {counts pos 1976339 neg 2025610 sig 4000001}, hi
DCV-10, {idx=3, first=0}, {azmux s5, oob 0, zglc 0 }, {counts pos 1976344 neg 2025615 sig 4000001}, lo read 0.000,000,39 mean 0.000,000,15 (n=100/100), stddev +157.8n

nplc 1
DCV-10, {idx=2, first=0}, {azmux s1, oob 0, zglc 1 }, {counts pos  198445 neg  203392 sig  400001}, hi
DCV-10, {idx=3, first=0}, {azmux s5, oob 0, zglc 0 }, {counts pos  198446 neg  203393 sig  400001}, lo read 0.000,000,87 mean 0.000,000,40 (n=100/100), stddev +620.4n
missed data interrupt


star-lo
nplc 10

STARLO-10, {idx=3, first=0}, {azmux s6, oob 0, zglc 0 }, {counts pos 1976332 neg 2025600 sig 4000001}, lo read 0.000,000,04 mean 0.000,000,00 (n=100/100), stddev +174.3n
STARLO-10, {idx=2, first=0}, {azmux s6, oob 0, zglc 1 }, {counts pos 1976330 neg 2025598 sig 4000001}, hi

STARLO-10, {idx=2, first=0}, {azmux s6, oob 0, zglc 1 }, {counts pos 1976343 neg 2025611 sig 4000001}, hi
STARLO-10, {idx=3, first=0}, {azmux s6, oob 0, zglc 0 }, {counts pos 1976342 neg 2025610 sig 4000001}, lo read 0.000,000,17 mean -0.000,000,01 (n=100/100), stddev +157.5n

STAR-LO-10, {idx=2, first=0}, {azmux s6, oob 0, zglc 1 }, {counts pos 1976334 neg 2025604 sig 4000001}, hi
STAR-LO-10, {idx=3, first=0}, {azmux s6, oob 0, zglc 0 }, {counts pos 1976334 neg 2025604 sig 4000001}, lo read -0.000,000,00 mean -0.000,000,01 (n=100/100), stddev +154.4n

STAR-LO-10, {idx=2, first=0}, {azmux s6, oob 0, zglc 1 }, {counts pos 1976329 neg 2025602 sig 4000001}, (hi first)hi
STAR-LO-10, {idx=3, first=0}, {azmux s6, oob 0, zglc 0 }, {counts pos 1976329 neg 2025602 sig 4000001}, (hi first)lo read 0.000,000,04 mean -0.000,000,03 (n=100/100), stddev +157.8n



nplc 1
STARLO-10, {idx=2, first=0}, {azmux s6, oob 0, zglc 1 }, {counts pos  198410 neg  203356 sig  400001}, hi
STARLO-10, {idx=3, first=0}, {azmux s6, oob 0, zglc 0 }, {counts pos  198409 neg  203355 sig  400001}, lo read -0.000,000,87 mean -0.000,000,06 (n=100/100), stddev +522.4n

STARLO-10, {idx=2, first=0}, {azmux s6, oob 0, zglc 1 }, {counts pos  198410 neg  203356 sig  400001}, hi
STARLO-10, {idx=3, first=0}, {azmux s6, oob 0, zglc 0 }, {counts pos  198411 neg  203357 sig  400001}, lo read 0.000,000,44 mean -0.000,000,04 (n=100/100), stddev +619.7n

STARLO-10, {idx=2, first=0}, {azmux s6, oob 0, zglc 1 }, {counts pos  198411 neg  203357 sig  400001}, hi
STARLO-10, {idx=3, first=0}, {azmux s6, oob 0, zglc 0 }, {counts pos  198411 neg  203357 sig  400001}, lo read -0.000,000,00 mean 0.000,000,04 (n=100/100), stddev +527.6n

STAR-LO-10, {idx=2, first=0}, {azmux s6, oob 0, zglc 1 }, {counts pos  198432 neg  203379 sig  400001}, (hi first)hi
STAR-LO-10, {idx=3, first=0}, {azmux s6, oob 0, zglc 0 }, {counts pos  198432 neg  203379 sig  400001}, (hi first)lo read -0.000,000,00 mean -0.000,000,02 (n=100/100), stddev +487.8n


ref
nplc 10
REF-, {idx=2, first=0}, {azmux s3, oob 0, zglc 1 }, {counts pos  988714 neg 3013349 sig 4000001}, hi
REF-, {idx=3, first=0}, {azmux s7, oob 0, zglc 0 }, {counts pos 1975951 neg 2025225 sig 4000001}, lo read 6.999,994,09 mean 6.999,993,89 (n=100/100), stddev +178.2n






may 27, 2026.

noise is really good.  with the LO. averaging.

    10nplc. 160nV. RMS.
    - with lt1021/7V. reference.
    - no LP ref filter.
    - minimal shielding - just one cover over power supply.

    - previos experiments substituting - low-jitter xtals.  current-liimit resistors (zfoil, orn, lt5400) and values (40k,50k).
         FF. 175,  273,  574. and logic, and shieilding, power supplies, low-pass filterinig the ref.

    - currently  -
        - voltagtes kept close. eg. 3v3 for clk, gpio. and 3v5 for lvc DFF, lv 4053 switch.
        - removed some combinatorial logic on the fgpa register outputs for integrator reset, signal (doubtful made a difference).
        - data handling, only AZ difference. no nominal conversions (doubtful made a difference).
        - reinstate average LO code, for AZ. HI-first conversions (default). really helps.


dcv-10 may be 160nV. RMS.

n = 10
DCV-10, {idx=2, first=0}, {azmux  1(s1), oob 0 zglc 1 }, {counts pos 1976344 neg 2025613 sig 4000001}, hi
DCV-10, {idx=3, first=0}, {azmux  9(s5), oob 0 zglc 0 }, {counts pos 1976350 neg 2025619 sig 4000001}, lo read 0.000,000,75(5, 10), mean   0.000,000,65, stddev +126.2n
DCV-10, {idx=2, first=0}, {azmux  1(s1), oob 0 zglc 1 }, {counts pos 1976347 neg 2025616 sig 4000001}, hi
DCV-10, {idx=3, first=0}, {azmux  9(s5), oob 0 zglc 0 }, {counts pos 1976353 neg 2025622 sig 4000001}, lo read 0.000,000,39(6, 10), mean   0.000,000,63, stddev +148.0n
DCV-10, {idx=2, first=0}, {azmux  1(s1), oob 0 zglc 1 }, {counts pos 1976348 neg 2025617 sig 4000001}, hi
DCV-10, {idx=3, first=0}, {azmux  9(s5), oob 0 zglc 0 }, {counts pos 1976349 neg 2025618 sig 4000001}, lo read 0.000,000,26(7, 10), mean   0.000,000,56, stddev +158.7n
DCV-10, {idx=2, first=0}, {azmux  1(s1), oob 0 zglc 1 }, {counts pos 1976346 neg 2025615 sig 4000001}, hi
DCV-10, {idx=3, first=0}, {azmux  9(s5), oob 0 zglc 0 }, {counts pos 1976351 neg 2025620 sig 4000001}, lo read 0.000,000,35(8, 10), mean   0.000,000,55, stddev +170.1n
DCV-10, {idx=2, first=0}, {azmux  1(s1), oob 0 zglc 1 }, {counts pos 1976343 neg 2025612 sig 4000001}, hi
DCV-10, {idx=3, first=0}, {azmux  9(s5), oob 0 zglc 0 }, {counts pos 1976351 neg 2025620 sig 4000001}, lo read 0.000,000,70(9, 10), mean   0.000,000,58, stddev +170.6n
DCV-10, {idx=2, first=0}, {azmux  1(s1), oob 0 zglc 1 }, {counts pos 1976345 neg 2025614 sig 4000001}, hi
DCV-10, {idx=3, first=0}, {azmux  9(s5), oob 0 zglc 0 }, {counts pos 1976352 neg 2025621 sig 4000001}, lo read 0.000,000,57(0, 10), mean   0.000,000,58, stddev +170.6n
DCV-10, {idx=2, first=0}, {azmux  1(s1), oob 0 zglc 1 }, {counts pos 1976345 neg 2025614 sig 4000001}, hi
DCV-10, {idx=3, first=0}, {azmux  9(s5), oob 0 zglc 0 }, {counts pos 1976315 neg 2025583 sig 4000001}, lo read 0.000,000,75(1, 10), mean   0.000,000,59, stddev +176.8n
DCV-10, {idx=2, first=0}, {azmux  1(s1), oob 0 zglc 1 }, {counts pos 1976346 neg 2025615 sig 4000001}, hi
DCV-10, {idx=3, first=0}, {azmux  9(s5), oob 0 zglc 0 }, {counts pos 1976351 neg 2025620 sig 4000001}, lo read 0.000,000,61(2, 10), mean   0.000,000,57, stddev +169.5n
DCV-10, {idx=2, first=0}, {azmux  1(s1), oob 0 zglc 1 }, {counts pos 1976346 neg 2025615 sig 4000001}, hi
DCV-10, {idx=3, first=0}, {azmux  9(s5), oob 0 zglc 0 }, {counts pos 1976353 neg 2025622 sig 4000001}, lo read 0.000,000,52(3, 10), mean   0.000,000,56, stddev +169.6n
DCV-10, {idx=2, first=0}, {azmux  1(s1), oob 0 zglc 1 }, {counts pos 1976345 neg 2025614 sig 4000001}, hi
DCV-10, {idx=3, first=0}, {azmux  9(s5), oob 0 zglc 0 }, {counts pos 1976352 neg 2025621 sig 4000001}, lo read 0.000,000,66(4, 10), mean   0.000,000,56, stddev +161.9n
DCV-10, {idx=2, first=0}, {azmux  1(s1), oob 0 zglc 1 }, {counts pos 1976347 neg 2025616 sig 4000001}, hi
DCV-10, {idx=3, first=0}, {azmux  9(s5), oob 0 zglc 0 }, {counts pos 1976315 neg 2025583 sig 4000001}, lo read 0.000,000,57(5, 10), mean   0.000,000,54, stddev +149.5n
DCV-10, {idx=2, first=0}, {azmux  1(s1), oob 0 zglc 1 }, {counts pos 1976346 neg 2025615 sig 4000001}, hi
DCV-10, {idx=3, first=0}, {azmux  9(s5), oob 0 zglc 0 }, {counts pos 1976352 neg 2025621 sig 4000001}, lo read 0.000,000,66(6, 10), mean   0.000,000,56, stddev +144.8n
DCV-10, {idx=2, first=0}, {azmux  1(s1), oob 0 zglc 1 }, {counts pos 1976346 neg 2025615 sig 4000001}, hi
DCV-10, {idx=3, first=0}, {azmux  9(s5), oob 0 zglc 0 }, {counts pos 1976353 neg 2025622 sig 4000001}, lo read 0.000,000,57(7, 10), mean   0.000,000,60, stddev +104.3n
DCV-10, {idx=2, first=0}, {azmux  1(s1), oob 0 zglc 1 }, {counts pos 1976345 neg 2025614 sig 4000001}, hi
DCV-10, {idx=3, first=0}, {azmux  9(s5), oob 0 zglc 0 }, {counts pos 1976352 neg 2025621 sig 4000001}, lo read 0.000,000,66(8, 10), mean   0.000,000,63, stddev +65.31n
DCV-10, {idx=2, first=0}, {azmux  1(s1), oob 0 zglc 1 }, {counts pos 1976348 neg 2025617 sig 4000001}, hi
DCV-10, {idx=3, first=0}, {azmux  9(s5), oob 0 zglc 0 }, {counts pos 1976317 neg 2025585 sig 4000001}, lo read 0.000,000,57(9, 10), mean   0.000,000,61, stddev +62.21n
DCV-10, {idx=2, first=0}, {azmux  1(s1), oob 0 zglc 1 }, {counts pos 1976351 neg 2025620 sig 4000001}, hi
DCV-10, {idx=3, first=0}, {azmux  9(s5), oob 0 zglc 0 }, {counts pos 1976315 neg 2025583 sig 4000001}, lo read 0.000,000,44(0, 10), mean   0.000,000,60, stddev +80.02n
DCV-10, {idx=2, first=0}, {azmux  1(s1), oob 0 zglc 1 }, {counts pos 1976345 neg 2025614 sig 4000001}, hi
DCV-10, {idx=3, first=0}, {azmux  9(s5), oob 0 zglc 0 }, {counts pos 1976317 neg 2025585 sig 4000001}, lo read 0.000,000,97(1, 10), mean   0.000,000,62, stddev +131.3n





1nplc.  around 500nV RMS.

DCV-10, {idx=2, first=0}, {azmux  1(s1), oob 0 zglc 1 }, {counts pos  198447 neg  203394 sig  400001}, hi
DCV-10, {idx=3, first=0}, {azmux  9(s5), oob 0 zglc 0 }, {counts pos  198447 neg  203394 sig  400001}, lo read 0.000,000,47(3, 10), mean   0.000,000,58, stddev +502.1n
missed data interrupt
DCV-10, {idx=2, first=0}, {azmux  1(s1), oob 0 zglc 1 }, {counts pos  198447 neg  203394 sig  400001}, hi
DCV-10, {idx=3, first=0}, {azmux  9(s5), oob 0 zglc 0 }, {counts pos  198407 neg  203353 sig  400001}, lo read 0.000,000,03(4, 10), mean   0.000,000,63, stddev +420.0n
missed data interrupth
DCV-10, {idx=2, first=0}, {azmux  1(s1), oob 0 zglc 1 }, {counts pos  198447 neg  203394 sig  400001}, hi
DCV-10, {idx=3, first=0}, {azmux  9(s5), oob 0 zglc 0 }, {counts pos  198408 neg  203354 sig  400001}, lo read 0.000,000,50(5, 10), mean   0.000,000,59, stddev +413.3n
missed data interrupt
DCV-10, {idx=2, first=0}, {azmux  1(s1), oob 0 zglc 1 }, {counts pos  198446 neg  203393 sig  400001}, hi
DCV-10, {idx=3, first=0}, {azmux  9(s5), oob 0 zglc 0 }, {counts pos  198448 neg  203395 sig  400001}, lo read 0.000,001,78(6, 10), mean   0.000,000,73, stddev +539.3n
missed data interrupt
DCV-10, {idx=2, first=0}, {azmux  1(s1), oob 0 zglc 1 }, {counts pos  198446 neg  203393 sig  400001}, hi
DCV-10, {idx=3, first=0}, {azmux  9(s5), oob 0 zglc 0 }, {counts pos  198447 neg  203394 sig  400001}, lo read 0.000,001,31(7, 10), mean   0.000,000,72, stddev +532.3n
missed data interrupth
DCV-10, {idx=2, first=0}, {azmux  1(s1), oob 0 zglc 1 }, {counts pos  198447 neg  203394 sig  400001}, hi
DCV-10, {idx=3, first=0}, {azmux  9(s5), oob 0 zglc 0 }, {counts pos  198447 neg  203394 sig  400001}, lo read -0.000,000,00(8, 10), mean   0.000,000,63, stddev +570.3n
missed data interrupt
DCV-10, {idx=2, first=0}, {azmux  1(s1), oob 0 zglc 1 }, {counts pos  198447 neg  203394 sig  400001}, hi
DCV-10, {idx=3, first=0}, {azmux  9(s5), oob 0 zglc 0 }, {counts pos  198408 neg  203354 sig  400001}, lo read 0.000,000,47(9, 10), mean   0.000,000,68, stddev +530.5n
missed data interrupt
DCV-10, {idx=2, first=0}, {azmux  1(s1), oob 0 zglc 1 }, {counts pos  198447 neg  203394 sig  400001}, hi
DCV-10, {idx=3, first=0}, {azmux  9(s5), oob 0 zglc 0 }, {counts pos  198408 neg  203354 sig  400001}, lo read 0.000,000,93(0, 10), mean   0.000,000,74, stddev +526.6n
missed data interrupt
DCV-10, {idx=2, first=0}, {azmux  1(s1), oob 0 zglc 1 }, {counts pos  198447 neg  203394 sig  400001}, hi
DCV-10, {idx=3, first=0}, {azmux  9(s5), oob 0 zglc 0 }, {counts pos  198408 neg  203354 sig  400001}, lo read 0.000,000,93(1, 10), mean   0.000,000,74, stddev +526.6n
missed data interrupt
DCV-10, {idx=2, first=0}, {azmux  1(s1), oob 0 zglc 1 }, {counts pos  198447 neg  203394 sig  400001}, hi
DCV-10, {idx=3, first=0}, {azmux  9(s5), oob 0 zglc 0 }, {counts pos  198408 neg  203354 sig  400001}, lo read 0.000,000,93(2, 10), mean   0.000,000,74, stddev +526.6n
missed data interrupt
DCV-10, {idx=2, first=0}, {azmux  1(s1), oob 0 zglc 1 }, {counts pos  198408 neg  203354 sig  400001}, hi
DCV-10, {idx=3, first=0}, {azmux  9(s5), oob 0 zglc 0 }, {counts pos  198409 neg  203355 sig  400001}, lo read 0.000,000,44(3, 10), mean   0.000,000,73, stddev +528.3nh
missed data interrupt
DCV-10, {idx=2, first=0}, {azmux  1(s1), oob 0 zglc 1 }, {counts pos  198408 neg  203354 sig  400001}, hi
DCV-10, {idx=3, first=0}, {azmux  9(s5), oob 0 zglc 0 }, {counts pos  198408 neg  203354 sig  400001}, lo read 0.000,000,44(4, 10), mean   0.000,000,77, stddev +486.7n
missed data interrupt



dcv 0.1

DCV-0.1, {idx=3, first=0}, {azmux  9(s5), oob 0 zglc 0 }, {counts pos 1977868 neg 2022984 sig 4000001}, lo read 0.000,000,82(6, 10), mean   0.000,000,67, stddev +152.2n
DCV-0.1, {idx=2, first=0}, {azmux  1(s1), oob 0 zglc 1 }, {counts pos 1977842 neg 2022982 sig 4000001}, hi
DCV-0.1, {idx=3, first=0}, {azmux  9(s5), oob 0 zglc 0 }, {counts pos 1977842 neg 2022965 sig 4000001}, lo read 0.000,000,73(7, 10), mean   0.000,000,67, stddev +153.1n
DCV-0.1, {idx=2, first=0}, {azmux  1(s1), oob 0 zglc 1 }, {counts pos 1977837 neg 2022983 sig 4000001}, hi
DCV-0.1, {idx=3, first=0}, {azmux  9(s5), oob 0 zglc 0 }, {counts pos 1977839 neg 2022966 sig 4000001}, lo read 0.000,000,74(8, 10), mean   0.000,000,68, stddev +154.2n
DCV-0.1, {idx=2, first=0}, {azmux  1(s1), oob 0 zglc 1 }, {counts pos 1977823 neg 2022969 sig 4000001}, hi
DCV-0.1, {idx=3, first=0}, {azmux  9(s5), oob 0 zglc 0 }, {counts pos 1977840 neg 2022968 sig 4000001}, lo read 0.000,000,66(9, 10), mean   0.000,000,72, stddev +65.36n
DCV-0.1, {idx=2, first=0}, {azmux  1(s1), oob 0 zglc 1 }, {counts pos 1977828 neg 2022975 sig 4000001}, hi
DCV-0.1, {idx=3, first=0}, {azmux  9(s5), oob 0 zglc 0 }, {counts pos 1977842 neg 2022974 sig 4000001}, lo read 0.000,000,61(0, 10), mean   0.000,000,71, stddev +73.73n
DCV-0.1, {idx=2, first=0}, {azmux  1(s1), oob 0 zglc 1 }, {counts pos 1977850 neg 2022993 sig 4000001}, hi
DCV-0.1, {idx=3, first=0}, {azmux  9(s5), oob 0 zglc 0 }, {counts pos 1977850 neg 2022976 sig 4000001}, lo read 0.000,000,49(1, 10), mean   0.000,000,68, stddev +93.95n
DCV-0.1, {idx=2, first=0}, {azmux  1(s1), oob 0 zglc 1 }, {counts pos 1977839 neg 2022986 sig 4000001}, hi
DCV-0.1, {idx=3, first=0}, {azmux  9(s5), oob 0 zglc 0 }, {counts pos 1977830 neg 2022960 sig 4000001}, lo read 0.000,000,67(2, 10), mean   0.000,000,67, stddev +90.68n
DCV-0.1, {idx=2, first=0}, {azmux  1(s1), oob 0 zglc 1 }, {counts pos 1977815 neg 2022962 sig 4000001}, hi
DCV-0.1, {idx=3, first=0}, {azmux  9(s5), oob 0 zglc 0 }, {counts pos 1977853 neg 2022980 sig 4000001}, lo read 0.000,000,67(3, 10), mean   0.000,000,67, stddev +90.48n
DCV-0.1, {idx=2, first=0}, {azmux  1(s1), oob 0 zglc 1 }, {counts pos 1977848 neg 2022988 sig 4000001}, hi
DCV-0.1, {idx=3, first=0}, {azmux  9(s5), oob 0 zglc 0 }, {counts pos 1977861 neg 2022981 sig 4000001}, lo read 0.000,000,59(4, 10), mean   0.000,000,67, stddev +90.73n
DCV-0.1, {idx=2, first=0}, {azmux  1(s1), oob 0 zglc 1 }, {counts pos 1977847 neg 2022984 sig 4000001}, hi
DCV-0.1, {idx=3, first=0}, {azmux  9(s5), oob 0 zglc 0 }, {counts pos 1977853 neg 2022972 sig 4000001}, lo read 0.000,000,62(5, 10), mean   0.000,000,66, stddev +87.34n
DCV-0.1, {idx=2, first=0}, {azmux  1(s1), oob 0 zglc 1 }, {counts pos 1977856 neg 2022992 sig 4000001}, hi
DCV-0.1, {idx=3, first=0}, {azmux  9(s5), oob 0 zglc 0 }, {counts pos 1977862 neg 2022983 sig 4000001}, lo read 0.000,000,56(6, 10), mean   0.000,000,63, stddev +72.99n
DCV-0.1, {idx=2, first=0}, {azmux  1(s1), oob 0 zglc 1 }, {counts pos 1977824 neg 2022966 sig 4000001}, hi
DCV-0.1, {idx=3, first=0}, {azmux  9(s5), oob 0 zglc 0 }, {counts pos 1977862 neg 2022981 sig 4000001}, lo read 0.000,000,80(7, 10), mean   0.000,000,64, stddev +85.18n
DCV-0.1, {idx=2, first=0}, {azmux  1(s1), oob 0 zglc 1 }, {counts pos 1977836 neg 2022981 sig 4000001}, hi
DCV-0.1, {idx=3, first=0}, {azmux  9(s5), oob 0 zglc 0 }, {counts pos 1977847 neg 2022976 sig 4000001}, lo read 0.000,000,75(8, 10), mean   0.000,000,64, stddev +86.76n
DCV-0.1, {idx=2, first=0}, {azmux  1(s1), oob 0 zglc 1 }, {counts pos 1977817 neg 2022954 sig 4000001}, hi
DCV-0.1, {idx=3, first=0}, {azmux  9(s5), oob 0 zglc 0 }, {counts pos 1977859 neg 2022988 sig 4000001}, lo read 0.000,000,31(9, 10), mean   0.000,000,61, stddev +130.9n
DCV-0.1, {idx=2, first=0}, {azmux  1(s1), oob 0 zglc 1 }, {counts pos 1977826 neg 2022972 sig 4000001}, hi
DCV-0.1, {idx=3, first=0}, {azmux  9(s5), oob 0 zglc 0 }, {counts pos 1977842 neg 2022971 sig 4000001}, lo read 0.000,000,62(0, 10), mean   0.000,000,61, stddev +131.0n



#endif


#if 0

  mar 12. 2026. move this into a test. sampling lo 10.

  nose seems lower in the morning. 6.50am.
  nplc 10.

  > first=1 idx=0 seq_n=2, counts pos 1975984 neg 2025255 sig 4000001,
  first=0 idx=1 seq_n=2, counts pos 1976354 neg 2025634 sig 4000001, sum 0.23, LO-10, read -0.000,000,82V, (0, 0), mean   -0.000,000,82V, stddev 0.000,000,00V,
  first=0 idx=0 seq_n=2, counts pos 1975946 neg 2025216 sig 4000001,
  first=0 idx=1 seq_n=2, counts pos 1976352 neg 2025632 sig 4000001, sum 0.13, LO-10, read -0.000,000,46V, (1, 1), mean   -0.000,000,64V, stddev 0.000,000,18V,
  first=0 idx=0 seq_n=2, counts pos 1976348 neg 2025628 sig 4000001,
  first=0 idx=1 seq_n=2, counts pos 1976354 neg 2025634 sig 4000001, sum 0.15, LO-10, read -0.000,000,52V, (2, 2), mean   -0.000,000,60V, stddev 0.000,000,16V,
  first=0 idx=0 seq_n=2, counts pos 1976348 neg 2025628 sig 4000001,
  first=0 idx=1 seq_n=2, counts pos 1976316 neg 2025595 sig 4000001, sum 0.20, LO-10, read -0.000,000,70V, (3, 3), mean   -0.000,000,63V, stddev 0.000,000,14V,
  first=0 idx=0 seq_n=2, counts pos 1976347 neg 2025627 sig 4000001,
  first=0 idx=1 seq_n=2, counts pos 1976316 neg 2025595 sig 4000001, sum 0.22, LO-10, read -0.000,000,79V, (4, 4), mean   -0.000,000,66V, stddev 0.000,000,14V,
  first=0 idx=0 seq_n=2, counts pos 1976349 neg 2025629 sig 4000001,
  first=0 idx=1 seq_n=2, counts pos 1976318 neg 2025597 sig 4000001, sum 0.22, LO-10, read -0.000,000,79V, (5, 5), mean   -0.000,000,68V, stddev 0.000,000,14V,
  first=0 idx=0 seq_n=2, counts pos 1976351 neg 2025631 sig 4000001,
  first=0 idx=1 seq_n=2, counts pos 1976354 neg 2025634 sig 4000001, sum 0.07, LO-10, read -0.000,000,26V, (6, 6), mean   -0.000,000,62V, stddev 0.000,000,20V,
  first=0 idx=0 seq_n=2, counts pos 1975945 neg 2025215 sig 4000001,
  first=0 idx=1 seq_n=2, counts pos 1976315 neg 2025594 sig 4000001, sum 0.23, LO-10, read -0.000,000,82V, (7, 7), mean   -0.000,000,65V, stddev 0.000,000,19V,
  first=0 idx=0 seq_n=2, counts pos 1976346 neg 2025626 sig 4000001,
  first=0 idx=1 seq_n=2, counts pos 1976353 neg 2025633 sig 4000001, sum 0.17, LO-10, read -0.000,000,61V, (8, 8), mean   -0.000,000,64V, stddev 0.000,000,18V,
  first=0 idx=0 seq_n=2, counts pos 1976348 neg 2025628 sig 4000001,
  first=0 idx=1 seq_n=2, counts pos 1976354 neg 2025634 sig 4000001, sum 0.15, LO-10, read -0.000,000,52V, (9, 9), mean   -0.000,000,63V, stddev 0.000,000,18V,
  first=0 idx=0 seq_n=2, counts pos 1975945 neg 2025215 sig 4000001,
  first=0 idx=1 seq_n=2, counts pos 1976353 neg 2025633 sig 4000001, sum 0.18, LO-10, read -0.000,000,64V, (0, 10), mean   -0.000,000,61V, stddev 0.000,000,17V,
  first=0 idx=0 seq_n=2, counts pos 1976348 neg 2025628 sig 4000001,
  first=0 idx=1 seq_n=2, counts pos 1976317 neg 2025596 sig 4000001, sum 0.22, LO-10, read -0.000,000,79V, (1, 10), mean   -0.000,000,65V, stddev 0.000,000,17V,
#endif


