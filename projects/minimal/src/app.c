


#include <stdio.h>    // printf, scanf
#include <string.h>   // strcmp, memset
#include <assert.h>
#include <malloc.h> // malloc_stats()
#include <stdlib.h>   // abs()




#include <lib2/util.h>   // msleep(), UNUSED, print_stack_pointer()
#include <lib2/format.h>   // trim_whitespace()  format_bits()
#include <lib2/streams.h>


#include <peripheral/spi-ice40.h>
#include <peripheral/spi-4094.h>
#include <peripheral/spi-ice40-bitstream.h>
#include <peripheral/spi-dac8811.h>
#include <peripheral/spi-ad5446.h>
#include <peripheral/led.h>
#include <peripheral/interrupt.h>
#include <peripheral/vfd.h>   // this is ok.

// app/app.c  should  not deal with devices

#include <device/fsmc.h>      // should removee. ?  setup should be in main()


#include <vfd.h>


#include <support.h>    // mcu_reset

#include <mode.h>
#include <app.h>
#include <util.h> // str_decode_uint


#include <device/fpga0_reg.h>

#include <lib2/stream-flash.h>

// last 128 . on 512k.
// this be declared in /periphal  perhaps. it's an arch/build dependency

#define FLASH_U202_ADDR   0x08060000
#define FLASH_UP5K_SIZE  104090           // fits in one sect.

// analog board.
#define FLASH_U102_ADDR   0x08080000
#define FLASH_HX8K_SIZE   135100          // needs two sect


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





static void spi_print_register( spi_t *spi, uint32_t reg )
{
  // basic generic print
  // query any register

  //assert(0);
  // spi_mux_ice40( spi);
  uint32_t ret = spi_ice40_reg_read32( spi, reg );
  char buf[ 100];
  printf("r %lu  v %lu  %s\n",  reg, ret,  str_format_bits(buf, 32, ret ));
}



static void spi_print_seq_register( spi_t *spi, uint32_t reg )
{
  // basic generic print
  // query any register

  assert(0);
  // spi_mux_ice40( spi);
  uint32_t ret = spi_ice40_reg_read32( spi, reg );
  char buf[ 100];
  char buf2[ 100];
  // printf("r %lu  v %lu  %s\n",  reg, ret,  str_format_bits(buf, 32, ret ));

  printf("r %lu   pc:%s   azmux:%s\n",  reg,
      str_format_bits(buf, 2, ret >> 4  ),          // pc switch value
      mux_to_string( ret & 0b1111,  buf2, 100  )    // azmux value
    );
}



void app_configure( app_t *app )
{

#if 0

  assert( 0);
  FILE *f = flash_open_file( FLASH_U202_ADDR );
  spi_ice40_bitstream_send( app->spi_u202, f, FLASH_UP5K_SIZE, & app->system_millis );
  fclose(f);

  // if( ! spi_port_cdone_get()) {
  if( ! app->spi_u202->cdone( app->spi_u202)) {

    printf("fpga config failed\n");

  }


#endif




    printf("app_configure()\n");

    assert( app->cdone_fpga0 );

    // check/verify 4094 OE is not asserted
    assert( ! spi_ice40_reg_read32( app->spi_fpga0, REG_4094_OE ));

    // reset the mode.
    *app->mode_current = *app->mode_initial;

    /* OK. this is tricky.
        OE must be enabled to pulse the relays. to align them to initial/current state.
        but want to configure as much 4094 state as possible (eg. muxes), before asserting 4094 OE.
    */
    // write the default 4094 state for muxes etc.
    printf("spi_mode_transition_state() for muxes\n");
    spi_mode_transition_state( app->spi_fpga0, app->spi_4094, app->spi_mdac0, app->mode_current, &app->system_millis);

    /*
        nov 2024. 4094-oe could just include in mode. to simplify this
    */

    // now assert 4094 OE
    // should check supply rails etc. first.
    printf("asserting 4094 OE\n");
    spi_port_configure( app->spi_fpga0);
    spi_ice40_reg_write32( app->spi_fpga0, REG_4094_OE, 1 );

    // check/ensure 4094 OE asserted
    assert( spi_ice40_reg_read32( app->spi_fpga0, REG_4094_OE ));

    // now call transition state again. which will do relays
    printf("spi_mode_transition_state() for relays\n");
    spi_mode_transition_state( app->spi_fpga0, app->spi_4094, app->spi_mdac0, app->mode_current, &app->system_millis);


    // setup the fpga0 interrupt handler
    interrupt_set_handler( app->fpga0_interrupt, app->data, (interupt_handler_t ) data_rdy_interupt);

#if 0

    /* enable the ice40 interupt
    // to delay until after fpga is configured, else get spurious
    */
    // spi1_port_interupt_handler_set( (void (*) (void *)) data_rdy_interupt, app->data );


    interrupt_t *x =  app->interrupt_u202;
    assert(x);
    x->handler = ( interupt_handler_t ) data_rdy_interupt;
    x->ctx = app->data ;

    // not needed
    // msleep( 10, &app->system_millis);

    // default, start in dcv mode.
    app_repl_statements(app, "        \
        flash cal read 123;           \
        reset;                        \
        data show stats;              \
        dcv;   \n                     \
      " );

  }
#endif
}





void app_beep( app_t * app, uint32_t n)
{
    // double beep ok.
    uint32_t t = 70;

  printf("configuring port\n");

  spi_port_configure( app->spi_u202 );

  for(unsigned i = 0; i < n; ++i)  {
    printf("on\n");
    spi_ice40_reg_write32( app->spi_u202, REG_DIRECT, 1 /*1<<10 */);
    spi_print_register( app->spi_u202, REG_DIRECT );
    msleep( t , &app->system_millis);

    printf("o\n");
    spi_ice40_reg_write32( app->spi_u202, REG_DIRECT, 0 );
    spi_print_register( app->spi_u202, REG_DIRECT );
    msleep( t , &app->system_millis);
  }
}



void app_led_dance( app_t * app )
{
  // should pass the spi/ and millis?. perhaps.
  // config.

  // needs to be in mode direct.

  spi_port_configure( app->spi_fpga0);

  // led dance
  for(unsigned i = 0; i < 50; ++i )  {
    static uint32_t counter = 0;
    ++counter;
    uint32_t magic = counter  ^ (counter >> 1);
    spi_ice40_reg_write32( app->spi_fpga0, REG_DIRECT, magic );

    // check the magic numger
    uint32_t ret = spi_ice40_reg_read32( app->spi_fpga0, REG_DIRECT);
    if(ret != magic ) {
      // comms no good
      char buf[ 100] ;
      printf("comms failed, returned reg value %s\n",  str_format_bits(buf, 32, ret ));
    } else {
      // printf("comms ok\n");
    }



    msleep( 50,  &app->system_millis);
  }

  spi_ice40_reg_write32( app->spi_fpga0, REG_DIRECT, 0 );
}



static void app_update_soft_500ms(app_t *app)
{
  assert(app);
  assert(app->magic == APP_MAGIC);

  /*
    function should reconstruct to localize scope of app. and then dispatch to other functions.
  */


  // blink mcu led
  app->led_state = ! app->led_state;
  led_set( app->led_status, app->led_state);


  //////////////
    assert( sizeof(seq_elt_t) == 4);

    seq_elt_t x;
    memset(&x, 0, sizeof(x));

    x.azmux = S1;
    x.pc = 0b01;

    void *px = &x;

    assert( *(uint32_t *)  px  == ((0b01 << 4) | S1));

  //////////////


#if 0
  // only try to read registers if configured.
  if( spi_ice40_cdone( app->spi_u202)) {


    // spi_print_register( app->spi_u202, REG_STATUS );    // show fan speed.

    spi_port_configure( app->spi_u202 );

    // perhaps should put on a separate register.
    uint8_t reg = REG_STATUS;
    uint32_t ret = spi_ice40_reg_read32( app->spi_u202, reg );

    uint32_t speed = ret & 0xffff;
    uint32_t rpm = speed * 60;

    printf("r %u  v %lu %lu\n",  reg, speed, rpm);
  }
#endif



  // fpga0 on analog board
  if( /*false &&*/ !app->cdone_fpga0) {


    FILE *f = flash_open_file( FLASH_U102_ADDR);
    int ret = spi_ice40_bitstream_send( app->spi_fpga0, f, FLASH_HX8K_SIZE, & app->system_millis );
    fclose(f);


    /* don't rely on cdone here.
      if no analog board is connected, this pin floats.
      instead use the return value of the spi_ice40_bitstream_send().
    */
    if( ret < 0) {

      printf("fpga config failed\n");
    } else {

      printf("fpga ok!\n");

      app->cdone_fpga0 = true;

      // app_beep( app, 2 );
      app_led_dance( app );

      // TODO better name
      app_configure( app);
    }

  }


  // u202 local ice40
  if( false && !spi_ice40_cdone( app->spi_u202)) {

    FILE *f = flash_open_file( FLASH_U202_ADDR );
    spi_ice40_bitstream_send( app->spi_u202, f, FLASH_UP5K_SIZE, & app->system_millis );
    fclose(f);

    if( !spi_ice40_cdone( app->spi_u202)) {

      printf("fpga config failed\n");
    } else {

      printf("fpga ok!\n");
      app_beep( app, 2 );
    }
  }


  /*
    would be nice if could detect if we lose configuration eg. for power supply

    we may be able to detect with a cdone==lo.
    disable the 4094-oe. check the cdone flag. re-enable 4094-oe.
  */
}


/*
  // u102 analog board - old
  // if( false && !app->cdone && !spi_port_cdone_get() ) {
  if( false && !app->cdone && !  app->spi_u202->cdone( app->spi_u202) ) {

    // we should separate these.

    app_configure( app );
  }
*/






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

      if(app->cdone_fpga0) {
        spi_mode_transition_state( app->spi_fpga0, app->spi_4094, app->spi_mdac0, app->mode_current, &app->system_millis);


        /*
          whether to restore interupt handler - could be predicated on trigger.

        */


        // restore fpga0 interrupt handler
        //if(app->mode_current->sa.p_trig)

        interrupt_set_handler( app->fpga0_interrupt, app->data, (interupt_handler_t ) data_rdy_interupt);

      }



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

  data_t *data = app->data;
  assert(data);


  // process new adc data in priority
  if(data->adc_interupt_valid) {

    data->adc_interupt_valid = false;
    data_update_new_reading2( data, app->spi_fpga0);

    // we don't need a continuation, so long as we have the interupt condition.
    // update vfd/gui
    vfd_update_new_reading( app->data );  // use the data previously computed.
  }


  // TODO - check - we brought the right code across in the interupt handler also.
  if( data->adc_interupt_valid_missed == true) {
    // just report for now
    printf("missed adc interupt\n");
    data->adc_interupt_valid_missed = false;
  }



  // handle console
  // note this calls app_update_repl() that starts actions.
  // we could pass a flag indicicating if it whoudl be processed.
  app_update_console(app);

  /* to side-step overflow/wrap around issues
      just a dedicated signed upward counter
      and subtract 500 eachtime.
    */

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

  data_t *data = app->data;
  assert(data);



  // process potential new incomming data in priority
  // data_update_new_reading( app->data, app->spi/*, app->verbose*/);

  // process new incoming data.
  if(data->adc_interupt_valid) {

    data->adc_interupt_valid = false;
    data_update_new_reading2( data, app->spi_fpga0);
  }




  // 500ms soft timer
  if( (app->system_millis - app->soft_500ms) > 500) {
    app->soft_500ms += 500;

    // blink mcu led
    app->led_state = ! app->led_state;
    led_set( app->led_status, app->led_state);
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
    led_set( app->led_status, app->led_state);

  }

}








static bool spi_repl_reg_query( spi_t *spi,  const char *cmd, uint32_t line_freq)
{
  /*
    typed on spi arg, should move?

    --
    direct fpga register query/access.  useful for debugging
    could also write as array loop.
    consider remove

    - consider prefix with 'reg' to indicate register query of fpga rather than mode structure.
    eg.

    reg spi-mux
    reg mode
    reg direct
    reg seq-mode
    reg status

    etc
  */


  uint32_t u0;

  // spi_port_configure( spi_fpga);


  if( strcmp( cmd, "spi-mux?") == 0) {

    spi_print_register( spi, REG_SPI_MUX);
  }
  else if( strcmp( cmd, "4094?") == 0) {

    spi_print_register( spi, REG_4094_OE);     // needs a better name. use a general control register. "CR" con
  }
   else if( strcmp(cmd, "mode?") == 0) {

    spi_print_register( spi, REG_MODE);
  }
  else if( strcmp( cmd, "direct?") == 0) {

    spi_print_register( spi, REG_DIRECT);
  }
/*
  else if( strcmp( cmd, "seq mode?") == 0) {

    spi_print_register( spi, REG_SEQ_MODE);
  }
*/

  else if( strcmp( cmd, "status?") == 0) {

    spi_print_register( spi, REG_STATUS);

    // don't bother decode contents.  because  sequence update is too fast.
  }

  else if( sscanf(cmd, "reg? %lu", &u0 ) == 1) {

    spi_print_register( spi, u0 );
  }

  // querying fpga direct. bypassing mode.
  else if( strcmp( cmd, "seq0?") == 0) {
    spi_print_seq_register( spi, REG_SA_P_SEQ0);
  }
  else if( strcmp( cmd, "seq1?") == 0) {
    spi_print_seq_register( spi, REG_SA_P_SEQ1);
  }
  else if( strcmp( cmd, "seq2?") == 0) {
    spi_print_seq_register( spi, REG_SA_P_SEQ2);
  }
  else if( strcmp( cmd, "seq3?") == 0) {
    spi_print_seq_register( spi, REG_SA_P_SEQ3);
  }

  else if( strcmp( cmd, "seqn?") == 0) {

    spi_print_register( spi, REG_SA_P_SEQ_N);
  }


  else if( strcmp(cmd, "nplc?") == 0
    || strcmp(cmd, "aper?") == 0
  ) {

    // spi_port_configure( spi_fpga);
    uint32_t aperture = spi_ice40_reg_read32( spi, REG_ADC_P_CLK_COUNT_APERTURE );

    aper_cc_print( aperture,  line_freq);
  }

  else return 0;


  return 1;
}



#if 0

these wont

static bool spi_repl_reg_write( spi_t *spi,  const char *cmd)
{

  /*
    these wont work. because will immediately be over-written by the mode write.
    when repl gets a '\n'

  */

  char s0[100 + 1 ];
  uint32_t u0, u1;

  if( sscanf(cmd, "reg! %lu %100s", &u0, s0) == 2
    && str_decode_uint( s0, &u1)
  ) {
    spi_port_configure( spi);

    spi_ice40_reg_write32( spi, u0 , u1 );
    spi_print_register(  spi, u0);
  }
  else if( sscanf(cmd, "mode! %lu", &u0 ) == 1) {

    spi_port_configure( spi);

    spi_ice40_reg_write32( spi, REG_MODE, u0 );
    spi_print_register(  spi, REG_MODE);
  }
  else if( sscanf(cmd, "direct! %100s", s0) == 1
    && str_decode_uint( s0, &u0)
  ) {

    spi_port_configure( spi);

    spi_ice40_reg_write32( spi, REG_DIRECT, u0 );
    spi_print_register(  spi, REG_DIRECT);
  }


/*
  we don't really have good test points.  for this. without controlling dcv-source output. u1006,u1007.

  else if( sscanf(cmd, "dac %s", s0 ) == 1
    && str_decode_uint( s0, &u0)
  ) {
      spi_port_configure( spi);

      spi_ice40_reg_write32( spi, REG_SPI_MUX,  SPI_MUX_DAC );
      spi_port_configure_mdac0( spi);

      spi_mdac0_write16( spi, u0 );
      spi_mux_ice40( spi);
    }
*/

  else
    return 0;


  return 1;
}
#endif



bool app_repl_statement(app_t *app,  const char *cmd)
{

  assert(app);
  assert(app->magic == APP_MAGIC);


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
    spi_mode_transition_state( app->spi_fpga0, app->spi_4094, app->spi_mdac0, app->mode_current, &app->system_millis);
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
    mcu_reset();
  }


#if 0
  else if(strcmp(cmd, "configure") == 0) {

    // reset fpga. load bitstream. turn on 4094.
    // init().   is better name.

    app_configure( app );
  }
#endif


#if 0
  else if(strcmp(cmd, "reset fpga") == 0) {

    ice40_port_extra_creset_enable();
    // wait
    msleep(1, &app->system_millis);
    ice40_port_extra_creset_disable();
  }
#endif




  // TODO remove. and move to normal 4094. transition-state() july 2024.
  // we dont

  else if( sscanf(cmd, "iso dac %s", s0 ) == 1
    && str_decode_uint( s0, &u0)
  ) {
      // working july
      assert(0);
      // spi_mux_ice40( app->spi);
      spi_ice40_reg_write32(app->spi_fpga0, REG_SPI_MUX,  SPI_MUX_ISO_DAC );

      assert(0);
      // spi_port_configure_mdac0( app->spi);
      // spi_mdac0_write16(app->spi, u0 );

      assert(0);
      // spi_mux_ice40(app->spi);
    }


  // TODO move to test code.

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



  ///////////////////////

  // need better name u202 load bitstream
  else if(strcmp(cmd, "bitstream test") == 0) {

    FILE *f = flash_open_file( FLASH_U202_ADDR );
    spi_ice40_bitstream_send( app->spi_u202, f, FLASH_UP5K_SIZE, & app->system_millis );
    fclose(f);
  }



  else if(strcmp(cmd, "beep") == 0) {

    app_beep( app, 1 );
  }

  // need better name
  // u202 reg write
  else if( sscanf(cmd, "whoot %lu", &u0 ) == 1) {

    // write a value to u202 register

    char buf[100];
    printf("writing v %lu  %s\n",  u0,  str_format_bits(buf, 4, u0));

    spi_ice40_t *spi = app->spi_u202;

    spi_port_configure( spi );
    spi_ice40_reg_write32( spi, REG_DIRECT, u0 );
    spi_print_register( spi, REG_DIRECT );
  }

  ///////////////////////



  // don't we have some code - to handle sscan as binary/octal/hex ?


  /*
    - the cal function requires mode. which is only available in app.
     - should take an argument. for model_id.
    - no. only set the id, when it is saved to flash.
  */


  else if( sscanf(cmd, "cal %lu", &u0 ) == 1) {

    _mode_t mode = *app->mode_initial;
    unsigned model_spec = u0;


    data_cal( app->data,  app->spi_fpga0, app->spi_4094,  app->spi_mdac0,  &mode, model_spec, &app->system_millis, (void (*)(void *))app_update_simple_led_blink, app  );
  }

  else if(strcmp(cmd, "cal") == 0) {
    // cal with default model

    _mode_t mode = *app->mode_initial;
    unsigned model_spec = 3;

    data_cal( app->data,  app->spi_fpga0, app->spi_4094,  app->spi_mdac0,  &mode, model_spec, &app->system_millis, (void (*)(void *))app_update_simple_led_blink, app  );
  }


  /*
      would be better if this could be done in mode, rather than app.
  */

  else if(strcmp(cmd, "reset") == 0) {
    // reset mode. distinct from  trigger control
    // reset the mode.
    *app->mode_current = *app->mode_initial;
  }


  /*
    TODO would be much better if could move this to mode.
    do read cal at startup.
    and move reset. to mode reset.
  */
#if 0
  else if( strcmp(cmd, "dcv") == 0) {

    // sample ref-lo via dcv-source
    app_repl_statements(app, "        \
        set k407 0;   set k405 1;  set k406 1;  \
        nplc 10; set mode 7 ; azero s3 s7;  trig; \
      " );

    // check_data( == 7.000 )  etc.
    // return 1;
  }

#endif

  // better name dcv-chan1.  this is just noaz chan1.  operation.

  else if( strcmp(cmd, "dcv") == 0) {

    // sample ref-lo via dcv-source
    app_repl_statements(app, "        \
        nplc 10; set mode 7 ;         \
      " );


    // we want to be able to sample any input, easily.
    // app->mode->first.K407 = SR_SET;
    // mode_set_dcv_source_channel( app->mode, 1 ); // dcv

    // sa.p_seq0 = (0b01 << 4) | S1;        // dcv
/*

    dcv should open channel. 1.
      and setup the muxing.
      can be az or noaz.
      eg. 'dcv az' or dcv noaz'

    - we could perhaps look at the azmux - to determine the input. to what values to use if switching between az/noaz even boot..
    - actually the first element can stay the same.  we just change p_seq_n to 2. and change the data handler.

    remove the 'dcv-source chan 1' command.
*/

    sa_state_t *sa = &app->mode_current->sa;
    sa->p_seq_n = 1;
    sa->p_seq_elt[ 0].azmux = S1;
    sa->p_seq_elt[ 0].pc = 0b01;

    sa->p_trig = 1;

    // clear the interrupt handler, will be re-enabled at the end of mode transition state
    interrupt_set_handler( app->fpga0_interrupt, NULL, NULL );

    data_t *data = app->data;

    // clear the data buffers
    data_reset( data );
    data_rdy_clear( data);

    // set the data handler/catcher
    data->handler_computed_val = data_sa_simple_computed_val;
    data->ctx_computed_val = NULL;


    // check_data( == 7.000 )  etc.
    // return 1;
  }





  ///////////////////////
  // We want clear separation - for setting mode versus setting anything directly on fpga.
  // actually just remoe any thing that bypasses the mode.



  // else if ( spi_repl_reg_write( app->spi_fpga0,  cmd)) { }

  else if ( spi_repl_reg_query( app->spi_fpga0,  cmd, app->data->line_freq)) { }





  else if( mode_repl_statement( app->mode_current,  cmd, app->data->line_freq )) { }

  else if( data_repl_statement( app->data, cmd )) { }

  else if( data_flash_repl_statement(app->data, cmd)) { }


  /*
    these can apply the mode state, that has previously been setup.
    this can simplify, the code in these functions.
  */
  else if( app_test01( app, cmd  )) { }
  else if( app_test02( app, cmd  )) { }

  else if( app_test08( app, cmd  )) { }
  else if( app_test09( app, cmd  )) { }

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

    return 0;
  }


  // handled something.
  return 1;
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
      spi_mode_transition_state( app->spi_fpga0, app->spi_4094, app->spi_mdac0, app->mode_current, &app->system_millis);
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





