


#include <stdio.h>      // printf, scanf
#include <string.h>     // strcmp
#include <assert.h>
#include <malloc.h>     // malloc_stats()
#include <strings.h>    // strcasecmp
#include <errno.h>




#include <lib3/util.h>        // UNUSED
#include <lib3/format.h>      // str_format_bits()
#include <lib3/cbuffer.h>
#include <lib3/cstring.h>

// TODO - file should  be opened as dependency in main.c
#include <lib3/file-flash.h>  // file_open_flash()





#include <peripheral/spi-ice40.h>
#include <peripheral/spi-4094.h>
#include <peripheral/spi-ice40-pc.h>
// #include <peripheral/spi-dac8811.h>
#include <peripheral/spi-ad5446.h>
#include <peripheral/gpio.h>
#include <peripheral/interrupt.h>

#include <peripheral/vfd-font-large.h>  // vfd-test()

#include <peripheral/vfd.h>
#include <peripheral/tft.h>



#include <device/support.h>    // mcu_reset, print_stack_pointer
#include <support.h>             // str_decode_uint



#include <app.h>
#include <test/test.h>

#include <cal/transfer.h>

#include <mode.h>
#include <data/cal.h>
#include <data/data.h>
#include <data/decode.h>
#include <data/buffer.h>
#include <data/range.h>
#include <ranging.h>



#include <device/vfd0.h>      // device needed here, because we defer initialization of device
#include <display-vfd.h>
#include <agg/display-tft.h>





/*
// last 128 . on 512k.
// this be declared in /periphal  perhaps. it's an arch/build dependency

  actually should be declared in main.c
  which is where dependencies for app are created
*/

#define FLASH_U202_ADDR   0x08060000
#define FLASH_UP5K_SIZE  104090           // fits in one sect.

// analog board.
#define FLASH_U102_ADDR   0x08080000
#define FLASH_HX8K_SIZE   135100          // needs two sect




// fix me - forward declaration should be from header.
int flash_lzo_test(void);





void app_interrupt_usart( app_t *app, void *arg)
{
  assert(app && app->magic == APP_MAGIC);
  UNUSED( arg);


  assert( 0);

}


void app_interrupt_systick( app_t *app, void *arg)
{
  // interrupt context. avoid doing anything complicated here.

  assert(app && app->magic == APP_MAGIC);
  UNUSED( arg);


  ++ (*app->system_millis);
}




void app_interrupt_data_rdy( app_t *app, void *arg) // runtime context
{
  /* interrupt context.  avoid doiing anything compliicated here
    but relatively infrequent.
  */

  assert(app && app->magic == APP_MAGIC) ;    // this is wrong.
  UNUSED( arg);


  // if flag is still active, then record we missed processing some app.
  if(app->adc_interrupt_valid == true) {
    app->adc_interrupt_valid_missed = true;
    // ++app->adc_interrupt_valid_missed;     // count better? but harder to report.
  }

  // set flag
  app->adc_interrupt_valid = true;
}




static void app_update_simple_led_blink( app_t *app)
{
  // or just app_update_led_blink()
  // change name app_update_no_data() or app_update_limited()
  // or just app_update_simple()

  assert(app && app->magic == APP_MAGIC);

  // 500ms soft timer
  if( ( *(app->system_millis) - app->soft_500ms) > 500) {
    app->soft_500ms += 500;

    // blink mcu led
    app->led_state = ! app->led_state;
    gpio_write( app->gpio_status_led, app->led_state);

  }

}




void app_yield( app_t *app)
{
  /*
    simple api, that should handle most cases as default
    can always override the continuation

    if(app->yield)
      app->yield( app->yield_ctx);
  */
  assert(app && app->magic == APP_MAGIC);


  app_update_simple_led_blink( app);
}



void app_msleep( app_t *app, uint32_t delay)
{
  /*
    consider rename  app_yield_msleep()

    simple api, that should cover most cases as default

    caller should not know the yield context.
    can always override the continuation with
    app->yield before calling the func that calls this
  */

  assert(app && app->magic == APP_MAGIC);


  // remember system_millis is volatile.

  uint32_t start = *app->system_millis;
  while (true) {
    uint32_t elapsed = *(app->system_millis) - start;
    if(elapsed > delay)
      break;

    app_yield( app);
  };
}










static void state_format ( uint8_t *state, size_t n)
{
  assert(state);

  char buf[100];
  for( unsigned i = 0; i < n; ++i) {
    printf("v %s\n",  str_format_bits(buf, 8, state[ i]));
  }
}



/*
    feb 2026.
    moved spi_mode_transition_state() function to app level.
    gets rid of all the low level includes in mode. for fpga regs. much better.
    also get rid of the devices_t type
    ----

    obvious to type this on app_t, since depends on spi peripherals, and mode_t etc.
*/



void app_transition_state( app_t  *app)
{

  assert(app && app->magic == APP_MAGIC);

  _mode_t *mode = app->mode;
  assert(mode && mode->magic == MODE_MAGIC);


  // printf("app transition state\n");

  // printf("spi_mode_transition_state()  %p \n", devices);
  // printf("4094 size %u\n", sizeof(_4094_state_t));
  // assert( sizeof(_4094_state_t) == 3 );

  // mux spi to 4094. change mcu spi params, and set spi device to 4094
  // assert(0);
  // spi_mux_4094 ( spi);



#if 1

  assert( app->spi_4094);

  // configure
  spi_controller_configure( app->spi_4094);


/*
  printf("-----------\n");
  printf("write first state\n");
  state_format (  (void *) &mode->first, sizeof( mode->first));
*/

  spi_4094_write_n( app->spi_4094, (void *) &mode->serial, sizeof( mode->serial));

  // sleep 10ms, for relays
  app_msleep( app, 10);


/*
  // and format
  printf("write second state\n");
  state_format ( (void *) &mode->second, sizeof(mode->second));
*/

  // void _4094_state_clear_relays(_4094_state_t state *)

  _4094_state_t tmp = mode->serial;
  _4094_state_clear_relays( &tmp);

  spi_4094_write_n( app->spi_4094, (void *) &tmp, sizeof( tmp));
  // spi_4094_write_n( app->spi_4094, (void *) &mode->second, sizeof(mode->second));

#endif

  /////////////////////////////


  // write mdac0
  assert( app->spi_mdac0);
  spi_controller_configure( app->spi_mdac0);
  spi_ad5446_write16( app->spi_mdac0, mode->mdac0_val );


  // write mdac1
  assert( app->spi_mdac1);
  spi_controller_configure( app->spi_mdac1);
  spi_ad5446_write16( app->spi_mdac1, mode->mdac1_val );


  /////////////////////////////

  ///////////////

  // registers

  // spi_ice40_reg_write32( app->spi_fpga0, REG_CR, mode->reg_mode );
  _Static_assert ( sizeof( mode->reg_cr) == 4);
  spi_ice40_reg_write_n( app->spi_fpga0, REG_CR,  &mode->reg_cr,  sizeof( mode->reg_cr));

  // reg_direct for outputs under fpga control
  _Static_assert ( sizeof( mode->reg_direct) == 4);
  // TODO. review - why use write_n() rather than write32() here?
  spi_ice40_reg_write_n( app->spi_fpga0, REG_DIRECT,  &mode->reg_direct,  sizeof( mode->reg_direct));


  ///////////////

  // sample acquisition
  spi_ice40_reg_write32( app->spi_fpga0, REG_SA_P_CLK_COUNT_TRIG_DELAY, mode->sa.p_trig_delay);
  spi_ice40_reg_write32( app->spi_fpga0, REG_SA_P_CLK_COUNT_PRECHARGE,  mode->sa.p_precharge);


  spi_ice40_reg_write32( app->spi_fpga0, REG_SA_P_SEQ_N,  mode->sa.p_seq_n );

  // use write_n to work around strict aliasing
  // consider - consolidate to a single register
  _Static_assert ( sizeof( seq_elt_t) == 4);
  spi_ice40_reg_write_n( app->spi_fpga0, REG_SA_P_SEQ0, &mode->sa.p_seq_elt[ 0], sizeof( seq_elt_t));
  spi_ice40_reg_write_n( app->spi_fpga0, REG_SA_P_SEQ1, &mode->sa.p_seq_elt[ 1], sizeof( seq_elt_t));
  spi_ice40_reg_write_n( app->spi_fpga0, REG_SA_P_SEQ2, &mode->sa.p_seq_elt[ 2], sizeof( seq_elt_t));
  spi_ice40_reg_write_n( app->spi_fpga0, REG_SA_P_SEQ3, &mode->sa.p_seq_elt[ 3], sizeof( seq_elt_t));


  ///////////////

  // adc
  // printf("writing adc params - aperture %lu\n" ,   mode->adc.p_aperture  );
  spi_ice40_reg_write32( app->spi_fpga0, REG_ADC_P_CLK_COUNT_APERTURE,  mode->adc.p_aperture );
  spi_ice40_reg_write32( app->spi_fpga0, REG_ADC_P_CLK_COUNT_RESET,     mode->adc.p_reset );


  // write the mcu board trigger source,
  // this has nothing to do with the actual fpga/sample acquisition trigger
  gpio_write( app->gpio_trigger_source, mode->trigger_source);

  /*
    trigger state is not written here.
    instead, for c code manage manually/explicitly.
    for repl, the trigger update is when repl string is processed
    ie. after the call to update state
  */
}



void app_configure( app_t *app )
{

  assert(app && app->magic == APP_MAGIC);

#if 0

  assert( 0);
  FILE *f = file_open_flash( FLASH_U202_ADDR );
  spi_ice40_bitstream_send( app->spi_fpga1, f, FLASH_UP5K_SIZE, & app->system_millis );
  fclose(f);

  // if( ! spi_port_cdone_get()) {
  if( ! app->spi_fpga1->cdone( app->spi_fpga1)) {

    printf("fpga config failed\n");

  }


#endif

  printf("app_configure()\n");

  // assert( app->cdone_fpga0 );
  assert( spi_ice40_cdone( app->spi_fpga0_pc));

  // check/verify 4094 OE is not asserted
  assert( !spi_ice40_reg_read32( app->spi_fpga0, REG_4094_OE));

  // reset the mode.
  mode_init( app->mode);


  /* OK. this is tricky.
      OE must be enabled to pulse the relays. to align them to initial/current state.
      but want to configure as much 4094 state as possible (eg. muxes), before asserting 4094 OE.
  */
  // write the default 4094 state for muxes etc.
  printf("spi_mode_transition_state() for muxes\n");
  app_transition_state( app /*devices, app->mode, &app->system_millis */);


  /*
      nov 2024. 4094-oe could just be included in mode. to simplify this
  */

  // now assert 4094 OE
  // should check supply rails etc. first.
  printf("asserting 4094 OE\n");
  spi_controller_configure( app->spi_fpga0);
  spi_ice40_reg_write32( app->spi_fpga0, REG_4094_OE, 1 );


  // check/ensure 4094 OE asserted
  // serves as basic test of comms/miso also
  assert( spi_ice40_reg_read32( app->spi_fpga0, REG_4094_OE ));


  // now call transition state again. which will do relays
  printf("spi_mode_transition_state() for relays\n");
  app_transition_state( app );


  // interrupt_handler_set( app->interrupt_fpga0, app, (interrupt_handler_t ) app_interrupt_data_rdy);

  // set up the fpga0 interrupt handler
  // interrupt_t *i = app->interrupt_fpga0;
  assert( app->interrupt_fpga0 );

  interrupt_handler_set( app->interrupt_fpga0, app, (void (*)(void *, void*)) app_interrupt_data_rdy);

  // i->ctx = app;
  // i->handler = (interrupt_handler_t ) app_interrupt_data_rdy;



  // set start configuration
  // need a cbuf_push_str( ) function
  const char *cmd = "cal flash load; dcv 10; decode unshow; t;\r";

  for( const char *s = cmd; *s; ++s)
    cbuf_push( app->cbuf_console_in, *s);


/*
  const char *s = cmd;
  while( *s)  {
    cbuf_push( app->cbuf_console_in, *s) ;
    ++s;
  }
*/



}


static void spi_print_register( spi_t *spi, uint32_t reg );

static void spi_print_seq_register( spi_t *spi, uint32_t reg );



void app_beep( app_t * app, uint32_t n)
{
  /* function should not really be typed on app_t. since only requires app_msleep()
    should type on spi_t and system_millis.
      except sleep() also yields which requires additional context from app.
  */

  assert(app && app->magic == APP_MAGIC);

  assert( app->spi_fpga1);

    // double beep ok.
  uint32_t d = 70;

  printf("app_beep configure port\n");
  spi_controller_configure( app->spi_fpga1 );

  for(unsigned i = 0; i < n; ++i)  {
    printf("on\n");
    spi_ice40_reg_write32( app->spi_fpga1, REG_DIRECT, 1 /*1<<10 */);
    spi_print_register( app->spi_fpga1, REG_DIRECT );
    app_msleep( app, d);

    printf("off\n");
    spi_ice40_reg_write32( app->spi_fpga1, REG_DIRECT, 0 );
    spi_print_register( app->spi_fpga1, REG_DIRECT );
    app_msleep( app, d);
  }
}



void app_led_dance( app_t * app )
{
  /* function should not really be typed on app_t. since only requires app_msleep()
    should type on spi_t and system_millis.

    except sleep() also yields which requires additional context from app.
  */


  assert(app && app->magic == APP_MAGIC);

  // must be in mode direct.

  spi_controller_configure( app->spi_fpga0);

  // led dance
  for(unsigned i = 0; i < 50; ++i )  {

    static uint32_t counter = 0;
    ++counter;
    uint32_t magic = counter  ^ (counter >> 1);

    // TODO should mask value - to avoid writing other fields of the direct register.

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

    app_msleep( app, 50);
  }

  spi_ice40_reg_write32( app->spi_fpga0, REG_DIRECT, 0 );
}






static void app_update_500ms(app_t *app)
{

  assert(app && app->magic == APP_MAGIC);

  /*
    function should reconstruct to localize scope of app. and then dispatch to other functions.
  */


  // led state
  app->led_state = ! app->led_state;

  // blink mcu led
  gpio_write( app->gpio_status_led, app->led_state);


  // tft 500ms. timer
  display_tft_update_500ms( app->display_tft);
  display_vfd_update_500ms( app->display_vfd);


  // check fpga0 configured
  if( !spi_ice40_cdone( app->spi_fpga0_pc)) {


    /* feb 2026.  consider just pass f as the constructor dependency on app_t construction.
        not pass the flash address and size.
        seek() would need to work.
    */
    FILE *f = file_open_flash( FLASH_U102_ADDR);
    int ret = spi_ice40_bitstream_send( app->spi_fpga0_pc, f, FLASH_HX8K_SIZE, app->system_millis );
    fclose(f);


    /* don't rely on cdone here.
      if no analog board is connected, this pin floats.
      instead use the return value of the spi_ice40_bitstream_send().
    */
    if( ret < 0) {

      printf("fpga config failed\n");
    } else {

      printf("fpga ok!\n");

      // deassert CS vec
      spi_cs_deassert( app->spi_fpga0 );

      // app_led_dance( app );

      // TODO better name
      app_configure( app);
    }
  }



  // check fpga1 configured
  if( /*false &&*/ !spi_ice40_cdone( app->spi_fpga1_pc)) {

    // dependency should be set/passed to app.
    FILE *f = file_open_flash( FLASH_U202_ADDR );
    spi_ice40_bitstream_send( app->spi_fpga1_pc, f, FLASH_UP5K_SIZE, app->system_millis );
    fclose(f);

    if( !spi_ice40_cdone( app->spi_fpga1_pc)) {

      printf("fpga config failed\n");
    } else {

      printf("fpga ok!\n");

      // we need both system_millis, and fpga available
      // before we can init here
#if 1
      printf("vfd init!\n");
      vfd_init( app->vfd0,  app->system_millis);
      vfd_test( app->vfd0);
#endif

      printf("tft lcd init!\n");

      tft_read_ddb( app->tft );     // contains 00000

      // init the lcd
      tft_init( app->tft, app->system_millis );

      tft_read_ddb( app->tft );
      /*
      reg 161 (a1)  r
      000  0000000000000000
      087  0000000001010111
      097  0000000001100001
      001  0000000000000001
      255  0000000011111111 */

    //  tft_set_tear_on( app->tft );

      // set scroll start to base of memory
      // tft_set_scrollstart( app->tft, 0  );

      tft_test_fill( app->tft);


      // display_tft2( app->display_tft);

#if 0
      app_beep( app, 2 );
#endif
    }
  }


  /*
    would be nice if could detect if we lose configuration eg. for power supply

    we may be able to detect with a cdone==lo.
    disable the 4094-oe. check the cdone flag. re-enable 4094-oe.
  */
}







static void app_console_update(app_t *app)
{

  assert(app && app->magic == APP_MAGIC);


  while( true) {

    // got a character
    // int32_t ch = cbuf_pop( app->cbuf_console_in);

    signed ch = fgetc( stdin);
    if( ch < 0) {

      if( errno == EAGAIN) {
        clearerr( stdin);
        break;
      }
      else {

        printf("error code: %d, Description: %s\n", errno, strerror(errno));
        assert( 0);
      }
    }


    assert(ch >= 0);


    if (ch == ';' || ch == '\r' ) {

      // a separator, update state with changes according to string.
      char *cmd = cstring_ptr( app->command);
      cmd = str_trim_whitespace_inplace( cmd );

      // TODO - transform lower case

      printf("\n");
      app_repl_statement(app, cmd);

      // clear the current command buffer,
      // note, still more data to process in cbuf_console_in
      cstring_clear( app->command);
    }
    else if( cstring_count( app->command) < cstring_reserve( app->command) ) {

      // normal character
      // must accept whitespace here, since used to demarcate args
      cstring_push_back( app->command, ch);
      // echo to output. required for minicom.
      putchar( ch);

    } else {

      // ignore overflow chars
      printf("too many chars!!\n");
    }

    if( ch == '\r')
    {
      // apply state changes

      if( spi_ice40_cdone( app->spi_fpga0_pc))  {

        // update analog board state by calling transition_state(),
        // juncture for transition/transfering state
        app_transition_state( app );

        ranging_t *ranging = app->ranging;

        if( ranging->retrigger) {

          // re-trigger
          printf("ranging retrigger\n");

          // clear for next time
          ranging->retrigger = false;

          // set trigger low
          gpio_write( app->gpio_trigger, 0);
        }


        gpio_write( app->gpio_trigger, app->repl_trigger_val);
      }

      // issue new prompt
      printf("\n> ");

    }
  }   // while
}




void app_update( app_t *app)
{

  assert(app && app->magic == APP_MAGIC);

  // process new adc data in priority
  if( app->adc_interrupt_valid) {

    app->adc_interrupt_valid = false;

    /* EXTR.  we can always wrap data in an extra structure
      // if there is additional info needed
      this organization of passing the data, allows easy customizing of operations
      aggregation, digial filter, null

    */

    /*
      may need/want to distinguish the interrupt type.
      adc value.  or comparator overload.
      can use status register to indicate. what we need to respond to.

    */
    data_t  data;
    data_init( &data);

    /*
        we can get the current range. and pass it...
        in the decode_update_data() function.
    */

    // EXTR.
    /* TODO just stamp the active range here??!!!!
        NO. reason not to.
        is because we want flexibility to call decode_update()
        from outside the context of app_update().
        for tests etc.
        and for cal,  and acal transfer
        --------
        This point determines
        how much app_t state needs to go into decode_t
        ----

    */

    // data->range = ranging_current_range( ranging );
    // data.line_freq = app->line_freq;
    // and same for the line_freq. maybe the provisional cal.

    // TODO change name decode_update_data to decode_update_data_data
    decode_update_data( app->decode, &data /* range_t *range */);
    buffer_update_data( app->buffer, &data);

    printf( "\n");

    // vfd and tft update_data()
    display_vfd_update_data( app->display_vfd, &data);
    display_tft_update_data( app->display_tft, &data);

  }


  // TODO - check - we brought the right code across in the interrupt handler also.
  if( app->adc_interrupt_valid_missed == true) {

    // just report for now
    printf("missed adc interrupt\n");
    app->adc_interrupt_valid_missed = false;
  }


  // vfd and tft update
  // do nothing for the moment
  display_vfd_update( app->display_vfd);
  display_tft_update( app->display_tft);



  // handle console
  // note this calls app_update_repl() that starts actions.
  // we could pass a flag indicicating if it whoudl be processed.
  app_console_update( app);


  /* to side-step overflow/wrap around issues
      just a dedicated signed upward counter
      and subtract 500 eachtime.
    */

  // 500ms soft timer
  if( ( *(app->system_millis) - app->soft_500ms) > 500) {

    // rename and make a signed down counter.   soft_500_down
    // and can then remove the subtraction
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
    app_update_500ms(app);
  }


}











/*
  move this spi_ prefixed code to top-level support.h ?
    or spi.h.

  or even to

  ./include/device/spi-fpga0-reg.h

*/

static void spi_print_register( spi_t *spi, uint32_t reg )
{
  // TODO this code does not here in app.c
  // move to /src/device/fpga0-reg.c ?

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
  // TODO
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
      mux_to_str( ret & 0b1111,  buf2, 100  )    // azmux value
    );
}



static bool spi_repl_reg_query( spi_t *spi, const char *cmd, uint32_t line_freq)
{
  /*
    repl query low-level registers

    function is typed on spi_t,
    so code does not belong here in app.c

    or move to, src/device/spi-fpga0.c   or support.c ?
  */


  uint32_t u0;

  // spi_controller_configure( spi_fpga);

  if( sscanf(cmd, "reg? %lu", &u0 ) == 1) {

    // any register if we known the number
    spi_print_register( spi, u0 );
  }
  else if( strcmp( cmd, "4094?") == 0) {

    spi_print_register( spi, REG_4094_OE);
  }
  else if( strcmp(cmd, "cr?") == 0) {

    spi_print_register( spi, REG_CR);
  }
  else if( strcmp( cmd, "direct?") == 0) {

    spi_print_register( spi, REG_DIRECT);
  }
  else if( strcmp( cmd, "precharge?") == 0) {

    spi_print_register( spi, REG_SA_P_CLK_COUNT_PRECHARGE);
  }
  else if( strcmp( cmd, "trig_delay?") == 0) {

    // tdelay ?
    spi_print_register( spi, REG_SA_P_CLK_COUNT_TRIG_DELAY);
  }
  else if( strcmp( cmd, "status?") == 0) {

    // TODO consider decode.
    // don't bother because not that useful.
    // since registed is updated too quickly
    spi_print_register( spi, REG_STATUS);

  }

  /////////////////////
  // sample acquisition

  else if( strcmp( cmd, "seqn?") == 0) {

    spi_print_register( spi, REG_SA_P_SEQ_N);
  }

  else if( strcmp( cmd, "seq0?") == 0) {

    // TODO consider decode.
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

  /////////////////////
  // adc

  else if( strcmp(cmd, "nplc?") == 0
    || strcmp(cmd, "aper?") == 0
  ) {

    uint32_t aperture = spi_ice40_reg_read32( spi, REG_ADC_P_CLK_COUNT_APERTURE);
    aperture_print( aperture,  line_freq);
  }

  else if( strcmp(cmd, "reset?") == 0) {

    spi_print_seq_register( spi, REG_ADC_P_CLK_COUNT_RESET);
  }




  else return 0;

  return 1;
}





bool app_repl_statement( app_t *app,  const char *cmd)
{

  assert(app && app->magic == APP_MAGIC);

  /*

      we can dig line_freq out of app->data here if want.
      in order to pass down.
      rather than store line_freq in app.
      ----

  */


  // printf("cmd '%s'  %u\n", cmd, strlen(cmd) );



  char s0[ 100 + 1];
  // char s1[ 100 + 1];
  // char s2[ 100 + 1];
  // uint32_t u0; //, u1;
  double f0;
  // int32_t i0;



  ////////////////////



  if(strcmp(cmd, "") == 0) {

    // ignore
    printf("empty\n" );
  }

  // "t" to trigger
  else if(strcmp(cmd, "trig") == 0 || strcmp(cmd, "t") == 0) {

    app->repl_trigger_val = true;
  }


  // "h" for halt
  else if(strcmp(cmd, "halt") == 0 || strcmp(cmd, "h") == 0) {

    app->repl_trigger_val = false;
  }




/*
  EXTR.  jul 2024.
    sleep time can be treated as a var , and moved into mode.
    And it will then be applied at the time of transition state.
    eg. on a '\r'

    that way we can use it embedded.
    --------
    alternatively if want a sleep. we should apply the current mode.

*/

  else if( sscanf(cmd, "sleep %100s", s0) == 1
    && str_decode_float( s0, &f0))
  {
    // this isn't great
    // update the current state
    app_transition_state( app);

    // and sleep
    app_msleep( app, (uint32_t ) (f0 * 1000) );
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


#if 0
  // verbosity?
  else if( sscanf(cmd, "verbose %lu", &u0 ) == 1) {

    printf("setting verbose %lu\n", u0  );
    app->verbose = u0;
  }
#endif


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


#if 0
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

#endif

  else if(strcmp(cmd, "flash lzo test") == 0) {
    flash_lzo_test();
    // int flash_raw_test(void);
  }



  ///////////////////////

  // need better name u202 load bitstream
  else if(strcmp(cmd, "bitstream test") == 0) {

    FILE *f = file_open_flash( FLASH_U202_ADDR );
    spi_ice40_bitstream_send( app->spi_fpga1_pc, f, FLASH_UP5K_SIZE, app->system_millis );
    fclose(f);
  }



  else if(strcmp(cmd, "beep") == 0) {

    app_beep( app, 1 );
  }

#if 0
  // test write a value to u202 register
  // u202 reg write
  else if( sscanf(cmd, "whoot %lu", &u0 ) == 1) {

    // write a value to u202 register

    char buf[100];
    printf("writing v %lu  %s\n",  u0,  str_format_bits(buf, 4, u0));

    spi_t *spi = app->spi_fpga1;

    spi_controller_configure( spi );
    spi_ice40_reg_write32( spi, REG_DIRECT, u0 );
    spi_print_register( spi, REG_DIRECT );
  }
#endif

  ///////////////////////



  else if( app_transfer_repl_statement( app, cmd)) { }

  else if( mode_repl_statement( app->mode, cmd, *app->line_freq )) { }

  else if( cal_repl_statement( app->cal, cmd)) { }

  else if( decode_repl_statement( app->decode, cmd )) { }

  else if( buffer_repl_statement( app->buffer, cmd )) { }

  else if ( spi_repl_reg_query( app->spi_fpga0,  cmd, *app->line_freq)) { }

  else if( app_test_repl_statement( app, cmd )) { }


  else if( display_tft_repl_statement( app->display_tft, cmd )) { }


  // do last because of catchall like range interpretation
  // needs to be fixed.
  else if ( ranging_repl_range( app->ranging, cmd)) { }

  else {

    printf("unknown cmd, or bad argument '%s'\n", cmd );

    return 0;
  }


  // handled something.
  return 1;
}




void app_repl_statements(app_t *app,  const char *s)
{

  assert(app && app->magic == APP_MAGIC);
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
      // spi_mode_transition_state( &app->devices, app->mode, &app->system_millis);
      app_transition_state( app);
    }

    ++s;
  }

}






#if 0
/*
  these functions are typeed on app.
  leave here, rather than moving to range.c
    --------------

  actually do not really need access to app.   because repl will automatically update.
  could inject into ranging.c  structure.

*/

bool app_range_dir_valid( app_t *app, uint32_t range_idx, bool dir)    // 1 up. 0 down
{
  assert(app && app->magic == APP_MAGIC);
  assert( range_idx < app->ranges_sz );   // watch out for signess casts.


  const range_t *range = &app->ranges[ range_idx];

  return (dir == 1 && !range->top_sentinal)
    || (dir == 0 && !range->bot_sentinal);
}



void app_range_switch( app_t *app, uint32_t range_idx)
{
  assert(app && app->magic == APP_MAGIC);

  printf("here2 range idx %lu\n", range_idx);   // is 50???

  // range_idx is unsigned and expected to be valid
  assert( range_idx < app->ranges_sz );   // watch out for signess casts.

  // set the current range_idx. used for decode_update_data
  *(app->range_idx) = range_idx;


  // apply the range mode state transition
  const range_t *range = &app->ranges[ range_idx];
  assert( range);
  assert( range->range_set_mode);

  printf( "switch to %s-%s\n", range->name, range->arg);

  range->range_set_mode( range, app->mode, app->range_10Meg);

  // force retrigger to clear buffers
  app->repl_retrigger = true;
}



void app_range_switch1( app_t *app, const char *name, const char *arg)
{
  assert(app && app->magic == APP_MAGIC);

  // assert() that the range exists

  int32_t range_idx = range_get_idx( app->ranges, app->ranges_sz, name, arg);


  assert( range_idx >= 0 && range_idx < (int) app->ranges_sz);

  app_range_switch( app, range_idx);
}






bool app_repl_range( app_t *app, const char *cmd)
{
  assert(app && app->magic == APP_MAGIC);

  char name[ 100 + 1];
  char arg[ 100 + 1];

  // perhaps sscanf will do this - if second argument is not found?
  arg[ 0] = 0;


  unsigned n = sscanf(cmd, "%100s %100s", name, arg);

  // handle no argument version of this also
  if( n == 2 || n == 1) {

    int32_t range_idx = range_get_idx( app->ranges, app->ranges_sz, name, arg);

    printf("here0 range idx %ld\n", range_idx);   // is 50???

    assert( range_idx < (int) app->ranges_sz);

    if( range_idx >= 0) {

      printf("calling range switch\n");
      app_range_switch( app, range_idx);
      return true;
    }
    else {

      printf("range not found\n");
    }

  }

  return false;
}

#endif




#if 0
static bool app_repl_range( app_t *app, const char *cmd)
{
  // consider rename app_repl_set_range()  app_repl_maybe_set_range()

  for( unsigned i = 0; i < app->ranges_sz; ++i )  {

    range_t *range = &app->ranges[ i];
    if( strcasecmp(cmd, range->name) == 0) {

      // update range index
      app->range_idx = i;

      // apply the mode...
      range->mode_f( app->mode );
      return true;
    }
  }

  return false;
}

#endif






#if 0
  else if( strcmp( cmd, "vfd") == 0) {
    // vfd


    fsmc_port_configure();


    // fsmc_setup( 12 );   // slow.
    // with divider == 1. is is easier to see the address is already well asserted on WR rising edge. before CS.
    // fsmc_setup( 1 );   // fase.
    // vfd_dev_gpio_init();

    // app_msleep( app, 10);


    ///////////
    display_vfd_init(  &app->system_millis);

    vfd_do_something();
  }
#endif





#if 0
  else if( strcmp(cmd, "dcv") == 0) {

    assert(0);

/*
    code cannot be put in mode, because needs app_repl parsing.
*/
    // sample ref
    // reset ; set ch2 ref;  set az ch2;  set mode 7; trig;
    app_repl_statements(app, "        \
        set ch2   ref;  \
        set az    ch2;  \
        set mode  7 ;   \
        trig;           \
      " );

  }
#endif

  // have a separate cal_repl ...
  // transfer


/*
  else if(strcmp(cmd, "cal_w") == 0)    { app_cal_w( app); }
  else if(strcmp(cmd, "cal_b") == 0)    { app_cal_b( app); }
  else if(strcmp(cmd, "cal_b10") == 0)  { app_cal_b10( app); }
  else if(strcmp(cmd, "cal_b100") == 0) { app_cal_b100( app); }
  else if(strcmp(cmd, "cal_b1000") == 0) { app_cal_b1000( app); }

  else if(strcmp(cmd, "cal_div1000") == 0) { app_cal_div1000( app); }

  else if(strcmp(cmd, "cal_all") == 0)  { app_cal_all( app); }
*/




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



#if 0

    /* enable the ice40 interrupt
    // to delay until after fpga is configured, else get spurious
    */
    // spi1_port_interrupt_handler_set( (void (*) (void *)) decode_rdy_interrupt, app->data );


    interrupt_t *x =  app->interrupt_fpga1;
    assert(x);
    x->handler = ( interrupt_handler_t ) decode_rdy_interrupt;
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



#if 0
    // we want to be able to sample any input, easily.
    // app->mode->first.K407 = SR_SET;
    // mode_dcv_source_set_channel( app->mode, 1 ); // dcv

    // sa.p_seq0 = (0b01 << 4) | S1;        // dcv
/*

    dcv should open channel. 1.
      and setup the muxing.
      can be az or noaz.
      eg. 'dcv az' or dcv noaz'

    - we could perhaps look at the azmux - to determine the input. to what values to use if switching between az/noaz even boot..
    - actually the first element can stay the same.  we just change p_seq_n to 2. and change the decode.handler.

    remove the 'dcv-source chan 1' command.
*/

    sa_state_t *sa = &app->mode->sa;
    sa->p_seq_n = 1;
    sa->p_seq_elt[ 0].azmux = S1;
    sa->p_seq_elt[ 0].pc = 0b01;

    // sa->p_trig = 1;

    // clear the interrupt handler, will be re-enabled at the end of mode transition state

    // JA.  feb. 2026. code looks completely wrong.
    assert(0);
    // interrupt_handler_set( app->devices.interrupt_fpga0, NULL, NULL );

#if 0
    decode_t *data = app->data;

    // clear the data buffer
    decode_reset( data );
    // decode_rdy_clear( data);

#endif

  // JA. disable feb 2026.
#if 0
    // set the decode.handler/catcher
    data->handler_computed_val = decode_sa_simple_computed_val;
    data->ctx_computed_val = NULL;
#endif


    // check_data( == 7.000 )  etc.
    // return 1;
#endif




#if 0

these wont

static bool spi_repl_reg_write( spi_t *spi,  const char *cmd)
{

  /*
    these no longer work. state will immediately be over-written by the write of the mode,
    when repl gets a '\n'

  */

  char s0[100 + 1 ];
  uint32_t u0, u1;

  if( sscanf(cmd, "reg! %lu %100s", &u0, s0) == 2
    && str_decode_uint( s0, &u1)
  ) {
    spi_controller_configure( spi);

    spi_ice40_reg_write32( spi, u0 , u1 );
    spi_print_register(  spi, u0);
  }
  else if( sscanf(cmd, "mode! %lu", &u0 ) == 1) {

    spi_controller_configure( spi);

    spi_ice40_reg_write32( spi, REG_CR, u0 );
    spi_print_register(  spi, REG_CR);
  }
  else if( sscanf(cmd, "direct! %100s", s0) == 1
    && str_decode_uint( s0, &u0)
  ) {

    spi_controller_configure( spi);

    spi_ice40_reg_write32( spi, REG_DIRECT, u0 );
    spi_print_register(  spi, REG_DIRECT);
  }


/*
  we don't really have good test points.  for this. without controlling dcv-source output. u1006,u1007.

  else if( sscanf(cmd, "dac %s", s0 ) == 1
    && str_decode_uint( s0, &u0)
  ) {
      spi_controller_configure( spi);

      spi_ice40_reg_write32( spi, REG_SPI_MUX,  SPI_MUX_DAC );
      spi_controller_configure_mdac0( spi);

      spi_mdac0_write16( spi, u0 );
      spi_mux_ice40( spi);
    }
*/

  else
    return 0;


  return 1;
}
#endif






#if 0
void app_update_simple_with_data(app_t *app)
{
  assert(app);
  assert(app->magic == APP_MAGIC);

  decode_t *data = app->data;
  assert(data);



  // process potential new incomming data in priority
  // decode_update_data_new_reading( app->data, app->spi/*, app->verbose*/);

  // process new incoming data.
  if(data->adc_interrupt_valid) {

    data->adc_interrupt_valid = false;
    decode_update_data_new_reading2( data, app->devices.spi_fpga0);
  }




  // 500ms soft timer
  if( (app->system_millis - app->soft_500ms) > 500) {
    app->soft_500ms += 500;

    // blink mcu led
    app->led_state = ! app->led_state;

    gpio_write( app->gpio_status_led, app->led_state);
  }

}

#endif



#if 0
  //////////////
    assert( sizeof(seq_elt_t) == 4);

    seq_elt_t x;
    memset(&x, 0, sizeof(x));

    x.azmux = S1;
    x.pc = 0b01;

    void *px = &x;

    assert( *(uint32_t *)  px  == ((0b01 << 4) | S1));

  //////////////
#endif


#if 0
  // only try to read registers if configured.
  if( spi_ice40_cdone( app->spi_fpga1)) {


    // spi_print_register( app->spi_fpga1, REG_STATUS );    // show fan speed.

    spi_controller_configure( app->spi_fpga1 );

    // perhaps should put on a separate register.
    uint8_t reg = REG_STATUS;
    uint32_t ret = spi_ice40_reg_read32( app->spi_fpga1, reg );

    uint32_t speed = ret & 0xffff;
    uint32_t rpm = speed * 60;

    printf("r %u  v %lu %lu\n",  reg, speed, rpm);
  }
#endif

  /// devices_t *devices = &app->devices;



#if 0
  /*
    blinking the led by pulsing the state transition.
    will will turn some analog switches on/off for 10ms..
    which means non consisten reading of values
  */

  if( app->led_blink_enable
    && spi_ice40_cdone( app->spi_fpga0_pc)
    && app->mode->reg_mode == MODE_DIRECT
  ) {

    // not doing anything interesting, so blink the led, to show activity.
    if(app->led_state)
      app->mode->reg_direct.leds_o |= 1;
    else
      app->mode->reg_direct.leds_o &= ~1;

    spi_mode_transition_state( &app->devices, app->mode, &app->system_millis);
  }
#endif




/*
  should be a better way to do these one-off tests
  move to a test?
  issue is the 500ms hook.

  - for tests etc. would be just to have a hook function.
  - 500ms_hook.  that tests could use.
  eg. to blink a led, or flick a relay.
  except we dont want state changed from outside the context of a test.

*/
#if 0
  // test toggle of 4094 cs
  if( spi_ice40_cdone( app->spi_fpga0_pc)) {

    // toggle the 4094 cs. only
    printf("toggle mdac1 cs\n");

    if(app->led_state )
      spi_cs_assert( app->spi_mdac1);
    else
      spi_cs_deassert( app->spi_mdac1);
  }
#endif

#if 0
  if( spi_ice40_cdone( app->spi_fpga0_pc)) {
    // toggle the trigger.

    gpio_write( app->gpio_trigger_internal, app->led_state);
  }
#endif





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





        /*
          whether to restore interrupt handler - could be predicated on trigger.

        */

        /*
          The better way to handle this.
          is handle the dispatch from the top level.
          eg. move the decode_rdy_flag. into app.

          and call like this -

          app_update() {
            decode_update_data( app->data );         <- this
            buffer_update_data( app->buffer );
            display_vfd_update_data( app->vfd  );
          }

          - EXTR - consider injecting data - into buffer.
          - and the buffer into the vfd display

          - there is clear nested structure exposure - as needed.

          eg. same as a game update() that displatches to all the submodules


          - and test that we have to do something is not handled at top level.  then we cannot precolate the update() .

          note that all the test code passes around app. so it is ok.

          - EXTR.   we don't even have to have data,buffer,display  structure pointers at top level.
                if just inject them.

        */

        // Feb 2026.  looks completely wrong.
        // interrupt_handler_set( app->devices.interrupt_fpga0, app->data, (interrupt_handler_t ) decode_rdy_interrupt);



