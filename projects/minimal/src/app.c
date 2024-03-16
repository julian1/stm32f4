


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

#include <lib2/streams.h>

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
#include <util.h> // str_decode_uint


#include <ice40-reg.h>


#include <data/data.h>     // for data_update()


// fix me
int flash_lzo_test(void);


/*
  keep general repl stuff (related to flashing, reset etc) here,
  put app specific/ tests in a separatefile.

*/







/////////////////////////
/*
  TODO.
  could put raw buffers directly in app structure? but poointer keeps clearer
  Why not put on the stack? in app_t ?
*/

static char buf_console_in[1000];
static char buf_console_out[1000];    // changing this and it freezes. indicates. bug


static char buf_command[1000];


void app_init_buffers( app_t *app )
{
  assert(app);
  assert(app->magic == APP_MAGIC);


  /* note no printf yet.
  */

  // uart/console
  cbuf_init(&app->console_in,  buf_console_in, sizeof(buf_console_in));
  cbuf_init(&app->console_out, buf_console_out, sizeof(buf_console_out));

  cbuf_init_stdout_streams(  &app->console_out );
  cbuf_init_stdin_streams( &app->console_in );


  cstring_init(&app->command, buf_command, buf_command + sizeof( buf_command));


}




void app_systick_interupt(app_t *app)
{
  assert(app);
  assert(app->magic == APP_MAGIC);


  // interupt context. don't do anything compliicated here.

  ++ app->system_millis;
}









static void app_update_soft_500ms(app_t *app)
{
  assert(app);
  assert(app->magic == APP_MAGIC);

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
  if( !ice40_port_extra_cdone_get()) {

    spi_ice40_bitstream_send(app->spi, & app->system_millis );

    // TODO . could improve error handling here,  although subsequent spi code is harmless

    // check/verify 4094 OE is not asserted
    assert( ! spi_ice40_reg_read32( app->spi, REG_4094 ));


    // reset the mode.
    *app->mode_current = *app->mode_initial;


    /* OK. this is tricky.
        OE must be enabled to pulse the relays. to align them to initial/current state.
        but we probably want to configure as much other state first, before asserting 4094 OE.
    */
    // write the default 4094 state for muxes etc.
    printf("spi_mode_transition_state() for muxes\n");
    spi_mode_transition_state( app->spi, app->mode_current, &app->system_millis);

    // now assert 4094 OE
    // should check supply rails etc. first.
    printf("asserting 4094 OE\n");
    spi_ice40_reg_write32( app->spi, REG_4094, 1 );
    // ensure 4094 OE asserted
    assert( spi_ice40_reg_read32( app->spi, REG_4094 ));

    // now call transition state again. which will do relays
    printf("spi_mode_transition_state() for relays\n");
    spi_mode_transition_state( app->spi, app->mode_current, &app->system_millis);


    /* enable the ice40 interupt
    // need to delay until after fpga is configured, else get spurious
    */
    spi1_port_interupt_handler_set( (void (*) (void *)) data_rdy_interupt, app->data );
  }


  if(ice40_port_extra_cdone_get()) {

    // app_update_soft_500ms_configured( app);
  }


}






static void app_update_console(app_t *app)
{
  assert(app);
  assert(app->magic == APP_MAGIC);



  while( !cbuf_is_empty(&app->console_in)) {

    // got a character
    int32_t ch = cbuf_pop(&app->console_in);
    assert(ch >= 0);


    if (ch == ';' || ch == '\r' )
    {
      // a separator, then apply what we have so far.
      char *cmd = cstring_ptr(&app->command);
      cmd = str_trim_whitespace_inplace( cmd );
      // could transform lower case
      printf("\n");
      app_repl_statement(app, cmd);

      // clear the current command buffer,
      // note, still more data to process in console_in
      cstring_clear( &app->command);
    }
    else if( cstring_count(&app->command) < cstring_reserve(&app->command) ) {

      // normal character
      // must accept whitespace here, since used to demarcate args
      cstring_push_back(&app->command, ch);
      // echo to output. required for minicom.
      putchar( ch);
    } else {

      // ignore overflow chars,
      printf("too many chars!!\n");
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


/*
    - for the yield we don't want to accept commands
    - and probably don't want to test or update the fpga
*/


void app_loop(app_t *app)
{
  /*
    main outer app loop, eg. bottom of control stack
  */

  assert(app);
  assert(app->magic == APP_MAGIC);



  // consider change name to app_process(),

  while(true) {

    // process potential new incomming data in priority
    data_update(app->data, app->spi);


    // handle console
    app_update_console(app);

    // 500ms soft timer
    if( (app->system_millis - app->soft_500ms) > 500) {
      app->soft_500ms += 500;

      // system_millis is shared, for msleep() and soft_timer.
      // but to avoid integer overflow/wraparound - could make dedicated and then subtract 500.
      // eg. have a deciated signed int 500ms counter,   if(app->soft_500ms >= 500) app->soft_500ms -= 500;
      // for msleep() use another dedicated counter.  since msleep() is not used recursively. simple, just reset count to zero, on entering msleep(), and count up.
      // actually msleep_with_yield() could be called recursively.
      // probably want to check, with a count/mutex.
      app_update_soft_500ms(app);
    }

  }
}



void app_update(app_t *app)
{
  /* non looping. for use in yield function.
      call, to keep pumping data input processing.
      useful for yield functions
    */
  assert(app);
  assert(app->magic == APP_MAGIC);


  // process new incomming data in priority
  data_update(app->data, app->spi);


  // no console process. - or else just process quit() , or some kind of interupt.

  // 500ms soft timer
  if( (app->system_millis - app->soft_500ms) > 500) {
    app->soft_500ms += 500;

    // probably want to check, with a count/mutex.
    app_update_soft_500ms(app);
  }

}










static void spi_print_register( uint32_t spi, uint32_t reg )
{
  // basic generic print
  // query any register
  spi_mux_ice40( spi);
  uint32_t ret = spi_ice40_reg_read32( spi, reg );
  char buf[ 100];
  printf("r %lu  v %lu  %s\n",  reg, ret,  str_format_bits(buf, 32, ret ));
}



static void spi_print_seq_register( uint32_t spi, uint32_t reg )
{
  // basic generic print
  // query any register
  spi_mux_ice40( spi);
  uint32_t ret = spi_ice40_reg_read32( spi, reg );
  char buf[ 100];
  char buf2[ 100];
  // printf("r %lu  v %lu  %s\n",  reg, ret,  str_format_bits(buf, 32, ret ));

  printf("r %lu   pc:%s   azmux:%s\n",  reg,
      str_format_bits(buf, 2, ret >> 4  ),          // pc switch value
      mux_to_string( ret & 0b1111,  buf2, 100  )    // azmux value
    );
}








/*
  EXTR.
  most of this is just setting the mode.  and should be expressed, 

    mode_repl_statement( mode,  cmd ). 


  anything that needs the app - can be a separate function
*/


void app_repl_statement(app_t *app,  const char *cmd)
{

  assert(app);
  assert(app->magic == APP_MAGIC);

  /*
    write the app->mode_current.
    and handle some out-of-mode  functions.

    eg. statement without semi-colon.

  */

  // to debug
  // printf("cmd '%s'  %u\n", cmd, strlen(cmd) );



  char s0[100 + 1 ];
  // char s1[100 + 1 ];
  // char s2[100 + 1 ];
  uint32_t u0;//, u1;
  double f0;
  // int32_t i0;



  ////////////////////


  if(strcmp(cmd, "") == 0) {
    // ignore
    printf("empty\n" );
  }


  else if(strcmp(cmd, "help") == 0) {

    printf("help <command>\n" );
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

  // change name fpga bitstrea load/test
  else if(strcmp(cmd, "bitstream test") == 0) {

    spi_ice40_bitstream_send(app->spi, & app->system_millis );
  }

  // don't we have some code - to handle sscan as binary/octal/hex ?


  else if(strcmp(cmd, "cal") == 0) {

    // void data_cal( data_t *data , uint32_t spi, _mode_t *mode /* void (*yield)( void * ) */ )
    _mode_t mode = *app->mode_initial;
    data_cal( app->data,  app->spi, &mode /* void (*yield)( void * ) */ );

  }




  else if( strcmp( cmd, "spi-mux?") == 0) {

    spi_print_register( app->spi, REG_SPI_MUX);
  }
  else if( strcmp( cmd, "4094?") == 0) {

    spi_print_register( app->spi, REG_4094);
  }
   else if( strcmp(cmd, "mode?") == 0) {

    spi_print_register( app->spi, REG_MODE);
  }
  else if( strcmp( cmd, "direct?") == 0) {

    spi_print_register( app->spi, REG_DIRECT);
  }
  else if( strcmp( cmd, "status?") == 0) {

    spi_print_register( app->spi, REG_STATUS);
  }
  else if( sscanf(cmd, "reg? %lu", &u0 ) == 1) {

    spi_print_register( app->spi, u0 );
  }

  // querying fpga direct. bypassing mode.
  else if( strcmp( cmd, "seq0?") == 0) {
    spi_print_seq_register( app->spi, REG_SA_P_SEQ0);
  }
  else if( strcmp( cmd, "seq1?") == 0) {
    spi_print_seq_register( app->spi, REG_SA_P_SEQ1);
  }
  else if( strcmp( cmd, "seq2?") == 0) {
    spi_print_seq_register( app->spi, REG_SA_P_SEQ2);
  }
  else if( strcmp( cmd, "seq3?") == 0) {
    spi_print_seq_register( app->spi, REG_SA_P_SEQ3);
  }

  else if( strcmp( cmd, "seqn?") == 0) {

    spi_print_register( app->spi, REG_SA_P_SEQ_N);
  }







  else if( strcmp(cmd, "nplc?") == 0
    || strcmp(cmd, "aper?") == 0) {
    // query fpga directly. not mode
    spi_mux_ice40(app->spi);
    uint32_t aperture = spi_ice40_reg_read32(app->spi, REG_ADC_P_CLK_COUNT_APERTURE );
    aper_cc_print( aperture,  app->line_freq);
  }


  ///////////////////////
  // We want clear separation - for setting mode versus setting anything directly on fpga.
  // actually just remoe any thing that bypasses the mode.


  else if(strcmp(cmd, "reset") == 0) {
    // reset mode. distinct from  trigger control
    // reset the mode.
    *app->mode_current = *app->mode_initial;
  }


  /*
    these can apply the mode state, that has previously been setup.
    this can simplify, the code in these functions.
  */
  else if( app_test01( app, cmd  )) { }
  else if( app_test02( app, cmd  )) { }
  else if( app_test03( app, cmd  )) { }
  else if( app_test05( app, cmd  )) { }
  else if( app_test14( app, cmd  )) { }
  else if( app_test15( app, cmd  )) { }



  else if( mode_repl_statement( app->mode_current,  cmd, app->line_freq )) { } 


  else {

    printf("unknown cmd, or bad argument '%s'\n", cmd );

  }
}




static void app_repl_statement_direct(app_t *app,  const char *cmd)
{
  assert(app);
  assert(app->magic == APP_MAGIC);

  /* this doesn't need app structure.
    except for the spi.

  */

  /* test code - writes direct to fpga.
      noting values will get immiedately overwritten by the mode transition function.
      So useful, but only for tests, for new functions, or when not using the mode transition.
      eg. testing the dac.
  */

  char s0[100 + 1 ];
  uint32_t u0, u1;


  if( sscanf(cmd, "reg %lu %100s", &u0, s0) == 2
    && str_decode_uint( s0, &u1)
  ) {
    spi_mux_ice40(app->spi);
    spi_ice40_reg_write32(app->spi, u0 , u1 );
    spi_print_register( app->spi, u0);
  }
  else if( sscanf(cmd, "mode %lu", &u0 ) == 1) {

    spi_mux_ice40(app->spi);
    spi_ice40_reg_write32(app->spi, REG_MODE, u0 );
    spi_print_register( app->spi, REG_MODE);
  }
  else if( sscanf(cmd, "direct %100s", s0) == 1
    && str_decode_uint( s0, &u0)
  ) {

    spi_mux_ice40(app->spi);
    spi_ice40_reg_write32(app->spi, REG_DIRECT, u0 );
    spi_print_register( app->spi, REG_DIRECT);
  }


  else if( sscanf(cmd, "dac %s", s0 ) == 1
    && str_decode_uint( s0, &u0)
  ) {
       // spi_mux_dac8811(app->spi);
      spi_mux_ad5446(app->spi );

      // eg. 0=-0V out.   0xffff = -7V out. nice.
      spi_dac8811_write16( app->spi, u0 );

      spi_mux_ice40(app->spi);
    }


}




void app_repl_statements(app_t *app,  const char *s)
{
  assert(app);
  assert(app->magic == APP_MAGIC);
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





