/*
nix-shell ~/devel/nixos-config/examples/arm.nix
  rlwrap -a picocom -b 115200 /dev/ttyUSB0

  nix-shell ~/devel/nixos-config/examples/arm.nix
  make

  nix-shell ~/devel/nixos-config/examples/arm.nix
  cd smu11
  openocd -f ../../openocd.cfg

  nix-shell ~/devel/nixos-config/examples/arm.nix
  rlwrap nc localhost 4444

  reset halt ; flash write_image erase unlock ./projects/minimal/main.elf; sleep 1500; reset run


// vim :colorscheme default. loooks good.

// cjmcu  stm32f407.
// issue. is that board/stlink doesn't appear to reset cleanly. needs sleep.

// - can plug a thermocouple straight into the sense +p, +n, and
// then use the slow (digital) integrator/pid loop - as a temperature controler..




*/


/*
  EXTR. we shouldn't need to include low-level peripheral headers stuff here.
  instead peripheral should be configured externally.
  ---

  we could put low-level peripheral stuff in peripheral directory.

*/
#include <libopencm3/stm32/rcc.h>   // for clock initialization
#include <libopencm3/cm3/scb.h>  // reset()
#include <libopencm3/stm32/spi.h>   // SPI1


#include <stdio.h>    // printf, scanf
#include <string.h>   // strcmp, memset
#include <ctype.h>    // isspace
#include <assert.h>
#include <malloc.h> // malloc_stats()



// library code
#include <lib2/usart.h>
#include <lib2/streams.h>
#include <lib2/util.h>   // msleep()
#include <lib2/cbuffer.h>
#include <lib2/cstring.h>
#include <lib2/format.h>   // trim_whitespace()



#include <peripheral/led.h>
#include <peripheral/spi-port.h>
#include <peripheral/ice40-extra.h>


#include <spi-ice40.h>
#include <spi-4094.h>
#include <spi-ice40-bitstream.h>

#include <lib2/format.h>   // format_bits()
#include <app.h>
#include <reg.h>

#include <mode.h>


// fix me
int flash_lzo_test(void);






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
  if(! ice40_port_extra_cdone_get()) {

    spi_ice40_bitstream_send(app->spi, & app->system_millis );
  }





  if(ice40_port_extra_cdone_get() /* && app->led_blink */ ) {

    // EXTR - we don'really want the electrical/comms activity of a heart-beat/led blink, during sample acquisition.
    // but it is a useful test.

    mux_spi_ice40( app->spi );


    if(app->led_blink) {
      // we need to not blink the led, if we want to use repl to write directly.

      // uint32_t magic = app->led_state ? 0b01010101 : 0b10101010 ;

/*
keep 
 15   always@(posedge clk) begin
 16     counter <= counter + 1;
 17     outcnt <= counter >> LOG2DELAY;
 18   end
 19 
 20   assign { LED1, LED2} = outcnt ^ (outcnt >> 1);
  */
      
      static uint32_t counter = 0;
      ++counter;

      uint32_t magic = counter  ^ (counter >> 1 ); 
/*
      static uint32_t magic = 0;
      ++magic;
*/

      // blink led... want option. so can write reg_direct
      // note - led will only, actually light if fpga in default mode. 0.
      spi_ice40_reg_write32( app->spi, REG_DIRECT, magic);

      // check the magic numger
      uint32_t ret = spi_ice40_reg_read32( app->spi, REG_DIRECT);
      if(ret != magic ) {
        // comms no good
        char buf[ 100] ;
        printf("comms failed, returned reg value %s\n",  str_format_bits(buf, 32, ret ));
      } else {
        // printf("comms ok\n");
      }
    }


    if(1) {
        // click the relays
        _4094_state_t mode;
        memset(&mode, 0, sizeof(mode));
        
        static bool flip = 0;
        flip = ! flip;
        mode.U705_UNUSED =  flip  ?   0b01 :  0b10; 


        // make sure assert 4094 OE is asserted.
        spi_ice40_reg_write32( app->spi, REG_4094, 1 );

        // make sure we are muxing spi,
        mux_spi_4094( app->spi );

        // can probe 4094 signals - by connecting scope to 4094 extension header pins.
        // write single byte - should be enough to flip a relay.
        // JA spi_4094_reg_write_n(app->spi, (uint8_t *)& magic , 1 );
        spi_4094_reg_write_n(app->spi, (uint8_t *)& mode, sizeof(mode) );

        // sleep 10ms.
        msleep(10, &app->system_millis);

        // now clear relay
        // uint8_t zero = 0;
        // spi_4094_reg_write_n(app->spi, & zero, 1 );

        mode.U705_UNUSED =  0b00; 
        spi_4094_reg_write_n(app->spi, (uint8_t *)& mode, sizeof(mode) );



        // EXTR. IMPORTANT. must call mux_spi_ice40 again - to stop signal emission on 4094 spi clk,data lines.
        mux_spi_ice40(app->spi);

      }

  }

  //////////

#if 0
  if(app->led_state) {

    ice40_port_extra_creset_enable();  // enable

    // printf("spi enable\n");
    assert(app->spi == SPI1);
    // spi_enable(app->spi);
    // spi_port_cs1_enable();
  }
  else {
    ice40_port_extra_creset_disable(); // hold fpga in reset.

    // Hmmmm.. not being respected????

    // printf("spi disable\n");
    // spi_disable(app->spi);

    // spi_disable(app->spi);
    // spi_port_cs1_disable();
  }
#endif



}




/*
  keep general repl stuff (related to flashing, reset etc) here,
  put app specific/ tests in a separatefile.

*/


static void app_repl(app_t *app,  const char *cmd)
{

  UNUSED(app);

  // useful for debug
  // printf("cmd '%s'  %u\n", cmd, strlen(cmd) );


/*
  char s0[100 + 1 ];
*/

  uint32_t u0 , u1;


  ////////////////////

  if(strcmp(cmd, "help") == 0) {

    printf("help <command>\n" );
  }


  else if(strcmp(cmd, "sleep") == 0) {
    msleep(1000, &app->system_millis);
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


  else if( sscanf(cmd, "blink %lu", &u0 ) == 1) {
    // turn off fpga blink in mode 0, avoid spi transmission, during acquisition.
    app->led_blink = u0;
  }



  /// change name fpga bitstrea load/test
  else if(strcmp(cmd, "bitstream test") == 0) {

    spi_ice40_bitstream_send(app->spi, & app->system_millis );
  }

  // don't we have some code - to handle sscan as binary/octal/hex ?

  else if( sscanf(cmd, "direct %lu", &u0 ) == 1) {

    // set the direct register.
    printf("set direct value to, %lu\n", u0 );

    mux_spi_ice40(app->spi);
    spi_ice40_reg_write32(app->spi, REG_DIRECT, u0 );
    // confirm.
    uint32_t ret = spi_ice40_reg_read32(app->spi, REG_DIRECT );
    char buf[ 100 ] ;
    printf("r %u  v %lu  %s\n",  REG_DIRECT, ret,  str_format_bits(buf, 32, ret ));
  }

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

    mux_spi_ice40(app->spi);
    uint32_t ret = spi_ice40_reg_read32(app->spi, REG_DIRECT );
    if(u1)
      ret |= 1 << u0 ;
    else
      ret &= ~( 1 << u0 );

    char buf[ 100 ] ;
    printf("r %u  v %lu  %s\n",  REG_DIRECT, ret,  str_format_bits(buf, 32, ret ));
    spi_ice40_reg_write32(app->spi, REG_DIRECT, ret );
  }


  else if( strcmp( cmd, "direct?") == 0) {

    mux_spi_ice40(app->spi);
    uint32_t ret = spi_ice40_reg_read32(app->spi, REG_DIRECT );
    char buf[ 100];
    printf("r %u  v %lu  %s\n",  REG_DIRECT, ret,  str_format_bits(buf, 32, ret ));
  }
  else if( strcmp( cmd, "status?") == 0) {

    mux_spi_ice40(app->spi);
    uint32_t ret = spi_ice40_reg_read32(app->spi, REG_STATUS);
    char buf[ 100];
    printf("r %u  v %lu  %s\n",  REG_STATUS, ret,  str_format_bits(buf, 32, ret ));
  }


  ////////////////////

  // could probably


  else if( sscanf(cmd, "mode %lu", &u0 ) == 1) {

    // set the fpga mode.
    mux_spi_ice40(app->spi);
    spi_ice40_reg_write32(app->spi, REG_MODE, u0 );

    uint32_t ret = spi_ice40_reg_read32(app->spi, REG_MODE );
    printf("reg_mode return value %lu\n", ret);
  }

  else if( strcmp(cmd, "mode?") == 0) {

    // TODO add some decoding here.
    mux_spi_ice40(app->spi);
    uint32_t ret = spi_ice40_reg_read32(app->spi, REG_MODE );
    printf("reg_mode return value %lu\n", ret);

    // Mode *mode = app->mode_current;
    // printf("app      return value %lu\n", mode->reg_mode );

  }

/*
  -- don't really need, just query direct reg for monitor and right shift.
*/
  else if( strcmp( cmd, "monitor?") == 0) {

    // this is no longer corrent. should query the REG_STATUS. which includes the monitor
    // regardless of the mode.
    mux_spi_ice40(app->spi);
    uint32_t ret = spi_ice40_reg_read32(app->spi, REG_DIRECT);
    ret >>= 14;
    char buf[ 100];
    printf("r %u  v %lu  %s\n",  REG_DIRECT, ret, str_format_bits(buf, 8, ret ));
  }





  else {

    printf("unknown cmd, or bad argument '%s'\n", cmd );

  }


}




static void app_update_console_cmd(app_t *app)
{

  while( ! cbuf_is_empty(&app->console_in)) {

    // got a character
    int32_t ch = cbuf_pop(&app->console_in);
    assert(ch >= 0);

    if(ch != '\r' && ch != ';' && cstring_count(&app->command) < cstring_reserve(&app->command) ) {
      // must accept whitespace here, since used to demarcate args
      // normal character
      cstring_push(&app->command, ch);
      // echo to output. required for minicom.
      putchar( ch);

    }  else {

      // process...
      // newline or overflow
      putchar('\n');

      char *cmd = cstring_ptr(&app->command);

      cmd = str_trim_whitespace_inplace( cmd );
      app_repl(app, cmd);

      // reset buffer
      cstring_clear( &app->command);

      // issue new command prompt
      printf("> ");
    }
  }
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





static void sys_tick_interupt(app_t *app)
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
  Maybe move raw buffers into app structure?


  Why not put on the stack? in app_t ?
*/

static char buf_console_in[1000];
static char buf_console_out[1000];    // changing this and it freezes. indicates. bug

// static char buf_cmds[1000];

static char buf_command[1000];


// move to main ?
// no reason to be static.
// no... because we collect/assemble dependencies. ok in main()
static app_t app;






// static Mode mode_current;


int main(void)
{
  // hse
	rcc_clock_setup_pll(&rcc_hse_8mhz_3v3[RCC_CLOCK_3V3_84MHZ] );  // stm32f411  upto 100MHz.

  // clocks
  rcc_periph_clock_enable(RCC_SYSCFG); // maybe required for external interupts?

  // gpio
  rcc_periph_clock_enable(RCC_GPIOA);
  rcc_periph_clock_enable(RCC_GPIOB);
  rcc_periph_clock_enable(RCC_GPIOC);
  // rcc_periph_clock_enable(RCC_GPIOE);

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

  // setup external state for critical error led blink
  // because assert() cannot pass a context
  assert_critical_error_led_setup(LED_PORT, LED_OUT);


  // this is the mcu clock.  not the adc clock. or the fpga clock.
  // systick_setup(16000);

  // extern void systick_setup(uint32_t tick_divider, void (*pfunc)(void *),  void *ctx);
  systick_setup(84000,  (void (*)(void *)) sys_tick_interupt, &app);  // 84MHz.


  //////////////////////
  // main app setup

  memset(&app, 0, sizeof(app_t));


  app.spi = SPI1 ;
  app.led_blink = true;


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

  printf("sizeof app_t %u\n", sizeof(app_t));



  ////////////////
  // spi1, for adum/ice40

  spi1_port_cs1_cs2_setup();

  spi1_port_interupt_setup( (void (*) (void *))spi1_interupt, &app);

  ice40_port_extra_setup();


 // spi_ice40_setup( app.spi );




  // modes_init();

  // go to main loop
  app_loop(&app);

  for (;;);
  return 0;


}




