


#include <stdio.h>    // printf, scanf
#include <string.h>   // strcmp, memset
#include <assert.h>
#include <malloc.h> // malloc_stats()
#include <stdlib.h>   // abs()



// #include <libopencm3/stm32/rcc.h>   // for clock initialization
#include <libopencm3/cm3/scb.h>  // reset()
// #include <libopencm3/stm32/spi.h>   // SPI1

#include <lib2/util.h>   // msleep(), UNUSED, print_stack_pointer()
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


// vfd
#include <peripheral/fsmc.h>
#include <peripheral/vfd.h>





#include <mode.h>
#include <app.h>
#include <util.h> // str_decode_uint


#include <ice40-reg.h>


#include <data/data.h>     // for main loop, data_update()


// fix me
int flash_lzo_test(void);


/*
  keep general repl stuff (related to flashing, reset etc) here,
  put app specific/ tests in a separatefile.

*/


/*
  3 events

    - systick/soft-timer interupt.
    - fpga interupt
    - external ui interupt
    - (also uart but never blocks).

    - yield can be used, on any long-running process to continue to service updates
        and out-of-band function

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


void app_init_console_buffers( app_t *app )
{
  assert(app);
  assert(app->magic == APP_MAGIC);


  /* note no printf yet.
      avoid assert()
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
    TODO. EXTR. much check the magic of the bitstream before we try to send it.
          it is easy to forget to actually put bitstream in the stm32 flash

  */


  if( /*false &&*/ !ice40_port_extra_cdone_get() ) {

    spi_ice40_bitstream_send(app->spi, & app->system_millis );

    // TODO . could improve error handling here,  although subsequent spi code is harmless

    /*
    for(unsigned i = 0; i < 50; ++i )  {
      static uint32_t counter = 0;
      ++counter;
      uint32_t magic = counter  ^ (counter >> 1);
      spi_ice40_reg_write32( app->spi, REG_DIRECT, magic );
      msleep( 50,  &app->system_millis);
    }
    */

#if 1
    /* we expect fpga is now configured. failure could be due to isolators not populated.
      do we want more graceful handling here?
      better to fail fast for the moment. */
    // assert(ice40_port_extra_cdone_get()) ;

    if( ! ice40_port_extra_cdone_get()) {

      printf("ice40 fpga config failed\n");
    }
    else {

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
#endif
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
      // a separator, update state with changes according to string.

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

    // apply state change
    if(ch == '\r')
    {
      // correct. it is ok/desirable. to update analog board state by calling transition_state(),
      // even if state hasn't been modified. eg. ensures that state is consistent/aligned.

      spi_mode_transition_state( app->spi, app->mode_current, &app->system_millis);
      // issue new command prompt
      printf("\n> ");

    }
  }   // while
}



/*
  In order to use the simple functions in yield statements, we want to split out the update() from the loop() t want the

  Actually we don't even want a app_loop() just loop at the bottom of the main statement.
*/


void app_update_main(app_t *app)
{
  assert(app);
  assert(app->magic == APP_MAGIC);


  // process potential new incomming data in priority
  data_update_new_reading( app->data, app->spi/*, app->verbose*/);


  // handle console
  // note this calls app_update_repl() that starts actions.
  // we could pass a flag indicicating if it whoudl be processed.
  app_update_console(app);

  // 500ms soft timer
  if( (app->system_millis - app->soft_500ms) > 500) {
    app->soft_500ms += 500;

    /*
      TODO review
      system_millis is shared, for msleep() and soft_timer.
      but to avoid integer overflow/wraparound - could make dedicated and then subtract 500.
      eg. have a deciated signed int 500ms counter,   if(app->soft_500ms >= 500) app->soft_500ms -= 500;
      for msleep() use another dedicated counter.  since msleep() is not used recursively. simple, just reset count to zero, on entering msleep(), and count up.
      actually msleep_with_yield() could be called recursively.
      probably want to check, with a count/mutex.
    */
    app_update_soft_500ms(app);
  }
}





void app_update_simple_with_data(app_t *app)
{
  assert(app);
  assert(app->magic == APP_MAGIC);


  // process potential new incomming data in priority
  data_update_new_reading( app->data, app->spi/*, app->verbose*/);


  // 500ms soft timer
  if( (app->system_millis - app->soft_500ms) > 500) {
    app->soft_500ms += 500;


    // blink mcu led
    app->led_state = ! app->led_state;
    if(app->led_state)
      led_on();
    else
      led_off();
  }
}








void app_update_simple_led_blink(app_t *app)
{
  // or just app_update_led_blink()
  // change name app_update_no_data() or app_update_restricted()

  assert(app);
  assert(app->magic == APP_MAGIC);

  // 500ms soft timer
  if( (app->system_millis - app->soft_500ms) > 500) {
    app->soft_500ms += 500;

    // blink mcu led
    app->led_state = ! app->led_state;
    if(app->led_state)
      led_on();
    else
      led_off();
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




// static bool app_repl_statement_direct(app_t *app,  const char *cmd);



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



/*
  EXTR.  jul 2024.
    sleep time can be treated as a var , and moved into mode.
    And it will then be applied at the time of transition state.
    eg. on a '\r'

    that way we can use it embedded.
    --------
    alternatively if we see a sleep. we should apply the current mode.

*/

  else if( sscanf(cmd, "sleep %100s", s0) == 1
    && str_decode_float( s0, &f0))
  {
#if 1
    // update state based on current mode
    spi_mode_transition_state( app->spi, app->mode_current, &app->system_millis);
#endif
    // sleep
    msleep( (uint32_t ) (f0 * 1000), &app->system_millis);

  }


  else if(strcmp(cmd, "help") == 0) {

    printf("help <command>\n" );
  }
  else if( sscanf(cmd, "print %100s", s0) == 1
    || sscanf(cmd, "echo %100s", s0) == 1) {
    // review . from old code
    printf( s0  );
    printf("\n");
  }


  // verbosity?
  else if( sscanf(cmd, "verbose %lu", &u0 ) == 1) {

    printf("setting verbose %lu\n", u0  );
    app->verbose = u0;
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





  // TODO remove. and move to normal 4094. transition-state() july 2024.
  // we dont

  else if( sscanf(cmd, "iso dac %s", s0 ) == 1
    && str_decode_uint( s0, &u0)
  ) {
      // working july
      spi_mux_ice40( app->spi);
      spi_ice40_reg_write32(app->spi, REG_SPI_MUX,  SPI_MUX_ISO_DAC );
      spi_port_configure_ad5446( app->spi);

      spi_ad5446_write16(app->spi, u0 );
      spi_mux_ice40(app->spi);
    }



  else if( strcmp( cmd, "vfd") == 0) {
    // vfd


    fsmc_gpio_setup();


    // fsmc_setup( 12 );   // slow.
    // with divider == 1. is is easier to see the address is already well asserted on WR rising edge. before CS.
    fsmc_setup( 1 );   // fase.
    vfd_init_gpio();

    msleep( 10, &app->system_millis );


    ///////////
    vfd_init(  &app->system_millis);

    vfd_do_something();
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


  /*
    - the cal function requires mode. which is only available in app.
     - should take an argument. for model_id.
    - no. only set the id, when it is saved to flash.
  */


  else if( sscanf(cmd, "cal %lu", &u0 ) == 1) {

    _mode_t mode = *app->mode_initial;
    unsigned model_spec = u0;

    data_cal( app->data,  app->spi, &mode, model_spec, &app->system_millis, (void (*)(void *))app_update_simple_led_blink, app  );
  }

  else if(strcmp(cmd, "cal") == 0) {
    // cal with default model

    _mode_t mode = *app->mode_initial;
    unsigned model_spec = 3;

    data_cal( app->data,  app->spi, &mode, model_spec, &app->system_millis, (void (*)(void *))app_update_simple_led_blink, app  );
  }


  /*
    direct fpga register query/access.  for debugging
    could write as array loop.
    consider remove
  */
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
  else if( strcmp( cmd, "seq mode?") == 0) {

    spi_print_register( app->spi, REG_SEQ_MODE);
  }

  else if( strcmp( cmd, "status?") == 0) {

    spi_print_register( app->spi, REG_STATUS);

    // don't bother decode contents.  because  sequence update is too fast.


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

    assert( app->data);
    aper_cc_print( aperture,  app->data->line_freq);
  }


  ///////////////////////
  // We want clear separation - for setting mode versus setting anything directly on fpga.
  // actually just remoe any thing that bypasses the mode.


  else if(strcmp(cmd, "reset") == 0) {
    // reset mode. distinct from  trigger control
    // reset the mode.
    *app->mode_current = *app->mode_initial;
  }




  // else if(  app_repl_statement_direct( app,  cmd )) { }


  else if(  mode_repl_statement( app->mode_current,  cmd, app->data->line_freq )) { }

  else if( data_repl_statement( app->data, cmd )) { }

  else if( data_flash_repl_statement(app->data, cmd)) { }


  /*
    these can apply the mode state, that has previously been setup.
    this can simplify, the code in these functions.
  */
  else if( app_test01( app, cmd  )) { }
  else if( app_test02( app, cmd  )) { }
  else if( app_test03( app, cmd  )) { }

  else if( app_test10( app, cmd  )) { }
  else if( app_test11( app, cmd  )) { }

  else if( app_test12( app, cmd  )) { }
  else if( app_test14( app, cmd  )) { }
  else if( app_test15( app, cmd  )) { }

  else if( app_test20( app, cmd, (void (*)(void *))app_update_simple_with_data, app )) { }
  else if( app_test40( app, cmd, (void (*)(void *))app_update_simple_with_data, app )) { }
  else if( app_test41( app, cmd, (void (*)(void *))app_update_simple_with_data, app )) { }
  else if( app_test42( app, cmd, (void (*)(void *))app_update_simple_with_data, app )) { }


  else {

    printf("unknown cmd, or bad argument '%s'\n", cmd );

  }
}


#if 0

static bool app_repl_statement_direct(app_t *app,  const char *cmd)
{
  assert(app);
  assert(app->magic == APP_MAGIC);

  /* this doesn't need app structure.
    except for the spi.

  */


  char s0[100 + 1 ];
  uint32_t u0;// , u1;

#if 0

  /*  test code - writes direct to fpga.
      THESE Dont WORK - since values will get immiedately updated/overwritten by the mode transition function.
      use set reg, set mode, set direct instead.
  */

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


/*
  we don't really have good test points.  for this. without controlling dcv-source output. u1006,u1007.

  else if( sscanf(cmd, "dac %s", s0 ) == 1
    && str_decode_uint( s0, &u0)
  ) {
      spi_mux_ice40( app->spi);
      spi_ice40_reg_write32(app->spi, REG_SPI_MUX,  SPI_MUX_DAC );
      spi_port_configure_ad5446( app->spi);

      spi_ad5446_write16(app->spi, u0 );
      spi_mux_ice40(app->spi);
    }
*/

#endif



  else
    return 0;


  return 1;
}

#endif


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





