

#include <stdio.h>    // printf, scanf
#include <string.h>   // memset
#include <assert.h>



#include <libopencm3/stm32/rcc.h>   // for clock initialization
#include <libopencm3/stm32/spi.h>   // SPI1


#include <lib2/usart.h>
#include <lib2/streams.h>
#include <lib2/util.h>   // msleep(), UNUSED
#include <lib2/format.h>   // trim_whitespace()  format_bits()



#include <peripheral/led.h>
#include <peripheral/spi-port.h>
#include <peripheral/ice40-extra.h>
#include <peripheral/spi-ice40.h>
#include <peripheral/spi-ice40-bitstream.h>



#include <mode.h>
#include <app.h>
#include <util.h>
#include <ice40-reg.h>






static void app_update_soft_500ms_configured(app_t *app)
{
  UNUSED(app);


}






static void app_update_soft_500ms(app_t *app)
{
  assert(app);

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
  }


  if(ice40_port_extra_cdone_get()) {

    app_update_soft_500ms_configured( app);
  }


}






static void app_update_console_cmd(app_t *app)
{
  // process console_in buffer

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






static void app_loop(app_t *app)
{

  while(true) {

    // handle console
    app_update_console_cmd(app);

    // 500ms soft timer
    if( (app->system_millis - app->soft_500ms) > 500) {
      app->soft_500ms += 500;
      app_update_soft_500ms(app);
    }

  }
}





static void systick_interupt(app_t *app)
{
  // interupt context. don't do anything compliicated here.

  ++ app->system_millis;
}




static void spi1_interupt(app_t *app)
{
  UNUSED(app);
  // now on a positive transition.

/*
  // if flag is still active, then record we missed processing some data.
  if(app->adc_measure_valid == true) {
    app->adc_measure_valid_missed = true;
    // ++app->adc_measure_valid_missed;     // count better? but harder to report.
  }

  // set adc_measure_valid flag so that update() knows to read the adc...
  app->adc_measure_valid = true;
*/
}








/////////////////////////
/*
  TODO.
  could put raw buffers directly in app structure? but poointer keeps clearer
  Why not put on the stack? in app_t ?
*/

static char buf_console_in[1000];
static char buf_console_out[1000];    // changing this and it freezes. indicates. bug


static char buf_command[1000];




static const _mode_t mode_initial =  {

  /*
    TODO,
    EXTR all relays should belhave to be defined. not left default initialization of 00 which means
    they won't get an initial value. pulse.

  */


  .first .K407  = LR_SET,     // disconnect dcv-source
  .first .K406  = LR_SET,     // accumulation cap off
  .first .K405  = LR_RESET,   // mux the himux2 - so dcv is free


  .reg_mode = 0, // MODE_LO,                                  // default, blink led according to mcu

  // signal acquisition defaults
  .sa.reg_sa_p_clk_count_precharge = CLK_FREQ * 500e-6,             //  500us.
  .sa.reg_sa_p_azmux_lo_val = S7,         // star-lo
  .sa.reg_sa_p_azmux_hi_val = S3,         // dc
  .sa. reg_sa_p_sw_pc_ctl_hi_val = 0b01,  // channel-1 precharge switch



  // adc
  .adc.reg_adc_p_aperture = CLK_FREQ * 0.2,   // 200ms. 10nplc 50Hz.  // Not. should use current calibration?  // should be authoritative source of state.
  .adc.reg_adc_p_reset = CLK_FREQ * 500e-6                // 500us.


#if 0

  //  maybe make explicit all values  U408_SW_CTL. at least for the initial mode, from which others derive.

  .first .K406_CTL  = LR_SET,     // accumulation relay off

  .first. K405_CTL  = LR_RESET,     // dcv input relay k405 switch off - works.
  .first. K402_CTL  = LR_RESET,     // dcv-div/directz relay off
                                // must match app->persist_fixedz

  .first. K401_CTL  = LR_SET,     // dcv-source relay off.
  .first. K403_CTL  = LR_RESET,     // ohms relay off.

  .first .U408_SW_CTL = 0,      // b2b fets/ input protection off/open


  // AMP FEEDBACK SHOULD NEVER BE TURNED OFF.
  // else draws current, and has risk damaging parts. mux pin 1. of adg. to put main amplifier in buffer/G=1 configuration.
  .first. U506 =  W1,     // should always be on

  // .second.K406_CTL  = LR_OFF,     // clear relay. default.
  // .second.K405_CTL  = LR_OFF,     // clear relay
  .second.U408_SW_CTL = 0,

  .second.U506 =  W1,           // amplifier should always be on.

  .first. K603_CTL  = LR_RESET,     // ohms relay off.


  /////////////////////////
  // 700
  // has inverting cmos buffer
  .first. K702_CTL  = LR_RESET,
  .second.K702_CTL  = 0b11,

  // 0.1R shunt off. has inverting cmos buffer
  .first. K703_CTL  = LR_RESET,
  .second.K703_CTL  = 0b11,

  // shunts / TIA - default to shunts
  .first. K709_CTL  = LR_SET,

  // agn200 shunts are off.
  .first. K707_CTL  = LR_SET,
  .first. K706_CTL  = LR_SET,
  .first. K704_CTL  = LR_SET,
  .first. K705_CTL  = LR_SET,


#endif

};



static _mode_t mode_current;



static app_t app = {

  .spi = SPI1 ,
  .mode_initial =  &mode_initial,
  .mode_current =  &mode_current,

  .line_freq = 50

};




int main(void)
{
  // hse
	rcc_clock_setup_pll(&rcc_hse_8mhz_3v3[RCC_CLOCK_3V3_84MHZ] );  // stm32f411  upto 100MHz.

  // clocks
  rcc_periph_clock_enable(RCC_SYSCFG); // maybe required for external interupts?

  // gpio

  // rcc_periph_clock_enable(RCC_GPIOA  | RCC_GPIOB | RCC_GPIOC);
  rcc_periph_clock_enable(RCC_GPIOA);
  rcc_periph_clock_enable(RCC_GPIOB);
  rcc_periph_clock_enable(RCC_GPIOC);
  // rcc_periph_clock_enable(RCC_GPIOE);

  // rcc_periph_clock_enable(RCC_USART1 | RCC_SPI1 | RCC_ADC1);

  // USART
  rcc_periph_clock_enable(RCC_USART1);

  // spi / ice40
  rcc_periph_clock_enable(RCC_SPI1);

  // adc/temp
  rcc_periph_clock_enable(RCC_ADC1);



  /////////////////////////////
  /*
    peripheral/ports setup
  */

  led_setup();

  // setup external state for critical error led blink in priority
  // because assert() cannot pass a context
  assert_critical_error_led_setup(LED_PORT, LED_OUT);


  // this is the mcu clock.  not the adc clock. or the fpga clock.
  // systick_setup(16000);

  // extern void systick_setup(uint32_t tick_divider, void (*pfunc)(void *),  void *ctx);
  // systick_setup(84000,  (void (*)(void *)) systick_interupt, &app);  // 84MHz.

  systick_setup(84000);
  systick_interupt_setup( (void (*)(void *)) systick_interupt, &app );



  //////////////////////
  // main app setup
/*
  memset(&app, 0, sizeof(app_t));

  // would be neater/possible, to use a static initializer for app ?

  app.spi = SPI1 ;
  app.mode_initial =  &mode_initial;
  app.mode_current =  &mode_current;

  app.line_freq = 50;
*/

  /////////////////


  // uart/console
  cbuf_init(&app.console_in,  buf_console_in, sizeof(buf_console_in));
  cbuf_init(&app.console_out, buf_console_out, sizeof(buf_console_out));

  cbuf_init_stdout_streams(  &app.console_out );
  cbuf_init_stdin_streams( &app.console_in );


  cstring_init(&app.command, buf_command, buf_command + sizeof( buf_command));

  //////////////
  // initialize usart before start all the app constructors, so that can print.
  // uart
  // usart1_setup_gpio_portA();
  usart1_setup_gpio_portB();

  usart1_set_buffers(&app.console_in, &app.console_out);



  printf("\n\n\n\n--------\n");
  printf("addr main() %p\n", main );

  printf("\n--------");
  printf("\nstarting\n");


  assert( sizeof(bool) == 1);
  assert( sizeof(float) == 4);
  assert( sizeof(double ) == 8);


  // printf("sizeof app_t %u\n", sizeof(app_t));



  ////////////////
  // spi1, for adum/ice40

  spi1_port_cs1_cs2_setup();

  spi1_port_interupt_setup( (void (*) (void *))spi1_interupt, &app);

  ice40_port_extra_setup();


  // go to main loop
  app_loop(&app);

  for (;;);
  return 0;


}




