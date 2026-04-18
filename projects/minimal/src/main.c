
/*
  create app_t dependencies
  create app_t,
  loop app_update()

*/

#include <stdio.h>    // printf, scanf
#include <assert.h>
#include <string.h>

#include <libopencm3/stm32/rcc.h>   // mcu clock initialization
#include <libopencm3/stm32/gpio.h>    // assert_critical_error_led_setup()


#include <lib2/usart.h>
#include <lib2/util.h>          // UNUSED,ARRAY_SIZE
#include <lib2/streams.h>
#include <lib2/cbuffer.h>
#include <lib2/cstring.h>


#include <device/spi1-port.h>
#include <device/spi2-port.h>

#include <device/spi-fpga0-pc.h>
#include <device/spi-fpga0.h>
#include <device/interrupt-fpga0.h>
#include <device/interrupt-systick.h>

#include <device/spi-4094-0.h>
#include <device/spi-mdac0.h>
#include <device/spi-mdac1.h>
#include <device/gpio-status-led.h>
#include <device/gpio-trigger-internal.h>
#include <device/gpio-trigger-selection.h>
#include <device/fsmc.h>



#include <peripheral/spi-ice40-pc.h>    // naming is not quite right.

#include <peripheral/vfd.h>           // OK. need storage size.
#include <peripheral/spi-ice40.h>
#include <peripheral/interrupt.h>
#include <peripheral/interrupt-systick.h>

// #include <peripheral/vfd-fonts.h>


#include <device/vfd0.h>
#include <device/tft0.h>

#include <device/spi-fpga1-pc.h>
#include <device/spi-fpga1.h>




#include <mode.h>
#include <app.h>
#include <data/cal.h>
#include <data/range.h>
#include <data/data.h>
#include <data/decode.h>
#include <data/buffer.h>
// #include <data/aggregate.h>
//#include <data/ranging.h>

#include <display-vfd.h>
#include <agg/display-tft.h>



// the flash addresses used for cal. should be defined here, and passed as dependencies


// mar 2024.
// see link/f413rgt6.ld
// #  7: 0x00060000 (0x20000 128kB) not protected
// # 11: 0x000e0000 (0x20000 128kB) not protected
#define FLASH_SECT_ADDR   0x080e0000                  // uint32_t
#define FLASH_SECT_NUM    11                          // uint8_t







static int main_f429(void)
{


 _Static_assert( sizeof(bool) == 1);
 _Static_assert( sizeof(float) == 4);
 _Static_assert( sizeof(double ) == 8);
 _Static_assert( sizeof(size_t) == 4);


  // use  f429.ld
  // hse
	// rcc_clock_setup_pll(&rcc_hse_8mhz_3v3[RCC_CLOCK_3V3_84MHZ] );  //  f413.
	// rcc_clock_setup_pll(&rcc_hse_8mhz_3v3[RCC_CLOCK_3V3_84MHZ] );  //  f429 will use ethernet clock, and different setup.

  rcc_clock_setup_pll(&rcc_hsi_configs[RCC_CLOCK_3V3_84MHZ ]);    // HSI WORKS stm32f410, may 2022, f407 2025.

  // clocks - TODO move after gpio ports?
  rcc_periph_clock_enable(RCC_SYSCFG); // maybe required for external interrupts?

  // gpio

  // rcc_periph_clock_enable(RCC_GPIOA  | RCC_GPIOB | RCC_GPIOC);
  rcc_periph_clock_enable(RCC_GPIOA);
  rcc_periph_clock_enable(RCC_GPIOB);
  rcc_periph_clock_enable(RCC_GPIOC);

  // for fsmc
  rcc_periph_clock_enable(RCC_GPIOD);
  rcc_periph_clock_enable(RCC_GPIOE);

  // more gpio.
  rcc_periph_clock_enable(RCC_GPIOF);
  rcc_periph_clock_enable(RCC_GPIOG);

  // USART
  rcc_periph_clock_enable(RCC_USART1);

  // spi / ice40
  rcc_periph_clock_enable(RCC_SPI1);
  rcc_periph_clock_enable(RCC_SPI2);

  // adc/temp
  rcc_periph_clock_enable(RCC_ADC1);


  // Enable FSMC
  rcc_periph_clock_enable(RCC_FSMC);


  // Tim 5
  rcc_periph_clock_enable(RCC_TIM5);



  // not for f4?
  // rcc_set_mco1_source(RCC_CFGR_MCO1_LSE);



  /////////////////////////////


  /*
    setup the critical error led blink in priority
    because assert() uses it
  */
  assert_critical_error_led_setup( GPIOA, GPIO9 );



  char buf_cbuf_console_in[ 1000];
  char buf_cbuf_console_out[1000];    // changing this and it freezes. indicates. bug
  char buf_command[1000];


  cbuf_t        cbuf_console_in;
  cbuf_t        cbuf_console_out;
  cstring_t     command;


  // uart/console
  cbuf_init( &cbuf_console_in,  buf_cbuf_console_in,  sizeof(buf_cbuf_console_in));
  cbuf_init( &cbuf_console_out, buf_cbuf_console_out, sizeof(buf_cbuf_console_out));

  cbuf_init_stdout_streams( &cbuf_console_out );
  cbuf_init_stdin_streams(  &cbuf_console_in );


  cstring_init( &command, buf_command, buf_command + sizeof( buf_command));


  /*
    the circular buffer on the output, makes no sense.
    instead consider block control, until the output is flushed.
  */
  //////////////
  // now can init usart peripheral using app console buffer
  usart1_setup_portB();

  usart1_set_buffers( &cbuf_console_in, &cbuf_console_out);

  printf("\n\n\n\n--------\n");
  printf("addr main() %p\n", main_f429);

  printf("\n--------");
  printf("\nstarting\n");




  ///////////////////


  volatile uint32_t system_millis = 0;


  gpio_t  gpio_status_led;
  gpio_status_led_init( &gpio_status_led);
  gpio_setup( &gpio_status_led);


#if 0
  // mcu clock
  // systick_setup(12000); // 12MHz. default lsi.
  systick_setup( 84000); // 84MHz.
  systick_handler_set( (void (*)(void *)) app_interrupt_systick, &app );
#endif


  // systick interrupt, but do not set handler
  interrupt_systick_t   interrupt_systick;
  interrupt_systick_init( &interrupt_systick, 84000); // 84MHz.
  interrupt_setup( &interrupt_systick);



  /////////////////////////////////


  ////////////////
  // init spi related port state. before do spi port.
  // to prevent ice40 wanting to become spi master


  // now init spi port controllers
  spi1_port_setup();


  spi2_port_setup();




  ///////////////////////////////
  // devices


  spi_ice40_t     spi_fpga0_pc;
  spi_fpga0_pc_init( &spi_fpga0_pc);
  spi_setup( &spi_fpga0_pc );



  spi_t           spi_fpga0;
  spi_fpga0_init( &spi_fpga0);
  spi_setup( &spi_fpga0);



  interrupt_t     interrupt_fpga0;
  interrupt_fpga0_init( &interrupt_fpga0);
  interrupt_setup( &interrupt_fpga0);


  /////////

  spi_ice40_t      spi_fpga1_pc;
  spi_fpga1_pc_init( &spi_fpga1_pc);
  spi_setup( &spi_fpga1_pc );


  spi_t    spi_fpga1;
  spi_fpga1_init( &spi_fpga1);
  spi_setup( &spi_fpga1 );


  spi_t    spi_4094_0;
  spi_4094_0_init( &spi_4094_0);
  spi_setup( &spi_4094_0);

  spi_t   spi_mdac0;
  spi_mdac0_init( &spi_mdac0);
  spi_setup( &spi_mdac0 );


  spi_t   spi_mdac1;
  spi_mdac1_init( &spi_mdac1);
  spi_setup( &spi_mdac1 );


  gpio_t    gpio_trigger;
  gpio_trigger_init( &gpio_trigger);
  gpio_setup( &gpio_trigger);


  gpio_t    gpio_trigger_source;
  gpio_trigger_source_init( &gpio_trigger_source);
  gpio_setup( &gpio_trigger_source);




  ///////////////////////////////



#if 0
  competes with mic1557

  printf("----------\nsetting up timer\n");
  uint32_t timer = TIM5;
  timer_port_setup();
  timer_setup( timer );
  timer_set_frequency( timer, 15000 );    // need to know the main mcu clock freq to do this.
#endif
                                          // perhaps record with the timer?
                                          // as device.


  ///////////////////////////////
  fsmc_gpio_setup();
  fsmc_setup( 12 );

  /////////////////////////////


  _mode_t       mode;
  mode_init( &mode);




  // structure just references state in app.
  // and makes it eay to serialize/deserialize to flash
  cal_t         cal;
  cal_init(
    &cal,
    // consider - pass the already opened file, as the correct dependency
    // use rewind. seek. etc. to position correctly
    FLASH_SECT_ADDR,
    FLASH_SECT_NUM
  );



  uint32_t      line_freq = 50;
  unsigned      range_idx = 0;        // TODO consider change to size_t. because indexes and allow bound check

  // both line_freq and range_idx  are going to have to be defined here with memory..
  //

  /*
      we want to be able to call decode_update_data()
      outside the context of app_update()
      eg. for tests etc.
      this point determines how much shared state from app_t,
      that has to be passed to decode_t on construction.
  */

  decode_t        decode;
  decode_init(
    &decode,
    &spi_fpga0,
    &cal,

    // TODO pass the range _sz also, customary and permits bounds checks.
    // OR consider passing the range_t
    /* - passing three separate values all associated with range. is a bit much.

        consider just pass single ranging structure. to decode.
        or else add a _decode( data_t *data) function to fill in the ranging info

        just range_t * ranging->get_active_range( ranging );     or something like this.

    */
    range_init_values,

    // deode_t will stamp these into data_t. to make available to other modules.
    &range_idx,
    &line_freq
  );


  double values[ 1000];

  buffer_t     buffer;
  buffer_init( &buffer, values, ARRAY_SIZE(values));




  // only gpio is possible for vfd, and tft, because
  // of dependency on fpga. also system_millis
  // full init must wait for system_millis and fpga
  vfd_t         vfd0;
  vfd0_init( &vfd0);
  vfd0.vfd_gpio_setup( &vfd0 );   // polymorphic, for multiple instances


  display_vfd_t   display_vfd;
  display_vfd_init( &display_vfd, &vfd0, &buffer);




  // tft
  tft_t     tft0;
  tft0_init( &tft0);    // device specific
  tft0.tft_gpio_setup( &tft0);    // polymorphic, for multiple instances



  /* OK. system_millis is going to need to be a pointer.
    if we want to share it with the agg test module.
  */

  // agg test
  display_tft_t    display_tft;
  display_tft_init( &display_tft, &tft0, &buffer, &system_millis );



  app_t app = {

    .magic              = APP_MAGIC,

    .system_millis      = &system_millis,

    // consider - can move to dedicated repl module?
    .cbuf_console_in    = &cbuf_console_in,
    .cbuf_console_out   = &cbuf_console_out,
    .command            = &command,


    .gpio_status_led    = &gpio_status_led,


    .interrupt_systick  = &interrupt_systick,

    .spi_fpga0_pc       = &spi_fpga0_pc,
    .spi_fpga0          = &spi_fpga0,
    .interrupt_fpga0    = &interrupt_fpga0,


    .spi_fpga1_pc       = &spi_fpga1_pc,
    .spi_fpga1          = &spi_fpga1,


    .spi_4094           = &spi_4094_0,

    .spi_mdac0          = &spi_mdac0,
    .spi_mdac1          = &spi_mdac1,

    .gpio_trigger       = &gpio_trigger,

    .gpio_trigger_source = &gpio_trigger_source,


    .mode               = &mode,

    // review. declared extern
    // not clear required in app_t, can limit scope to decode_t
    .ranges             = range_init_values,
    .ranges_sz          = range_init_sz,

    .cal                = &cal,

    // review - does app_t need these
    // or just inject into the decode_t and ranging_t.  etc.
    .line_freq          = &line_freq,
    .range_idx          = &range_idx,

    .decode             = &decode,
    .buffer             = &buffer,

    .vfd0               = &vfd0,        // needed because in app_t. because init() called only after init.
    .display_vfd        = &display_vfd,


    .tft                = &tft0,
    .display_tft           = &display_tft,

  } ;


  // must delay handler setup, until app is instantiated
  interrupt_handler_set( &interrupt_systick, &app, (void (*)(void *, void *)) app_interrupt_systick);




  // loop, bottom of the control stack
  while( true) {

    app_update( &app);
  }


  return 0;
}






// https://interrupt.memfault.com/blog/how-to-write-a-bootloader-from-scratch

// static void start_app(uint32_t pc, uint32_t sp) __attribute__((naked))
static  __attribute__((naked)) void start_app(uint32_t pc, uint32_t sp)
{
  UNUSED(pc);
  UNUSED(sp);
    __asm("           \n\
          msr msp, r1 /* load r1 into MSP */\n\
          bx r0       /* branch to the address at r0 */\n\
    ");
}




int main(void)
{
  return main_f429();
  // return main_f413();
}





#if 0
/*
  - for mcu startup
    - must have correct link script (eg. f413rgt6.) declared in Makefile declared, ie with correct ram for heap placement.
    - and if hse, then xtal populated
    -----------

    - note iso7662 isolators *appear* to draw a lot of current (43mA) on the fpga/secondary side..
        if mcu primary side is unpowered.
        - quite odd.  because tryng to drive unconfigured fpga gpio?
        - otherwise fpga/ needs 5V/15mA.  with only the leds/ no xtal. etc.

      Default output high (ISO776x) and low (ISO776xF) Options

*/


static int main_f413(void)
{
  // hse
	// rcc_clock_setup_pll(&rcc_hse_8mhz_3v3[RCC_CLOCK_3V3_84MHZ] );  // stm32f411  upto 100MHz.

  // hsi
  rcc_clock_setup_pll(&rcc_hsi_configs[RCC_CLOCK_3V3_84MHZ ]);

  // clocks
  rcc_periph_clock_enable(RCC_SYSCFG); // maybe required for external interrupts?

  // gpio

  // rcc_periph_clock_enable(RCC_GPIOA  | RCC_GPIOB | RCC_GPIOC);
  rcc_periph_clock_enable(RCC_GPIOA);
  rcc_periph_clock_enable(RCC_GPIOB);
  rcc_periph_clock_enable(RCC_GPIOC);


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
  assert_critical_error_led_setup( LED_PORT, LED_OUT);

  // mcu clock
  // systick_setup(12000); // 12MHz. default lsi.
  systick_setup(84000); // 84MHz. hsi/hse
  systick_handler_set( (void (*)(void *)) app_interrupt_systick, &app );  // rename systick_handler_set()

  //////////////////////
  // main app setup
  // initialzes the console buffer, that support printf() and error reporting
  app_init_console_buffer( &app );

  /*
    dont' think it makes any sense to use a circular buffer on the output.
    instead just block control until the output is flushed.
  */
  //////////////
  // now can init usart peripheral using app console buffer
  usart1_setup_portB();

  usart1_set_buffer( &app.cbuf_console_in, &app.cbuf_console_out);

  printf("\n\n\n\n--------\n");
  printf("addr main() %p\n", main_f413);

  printf("\n--------");
  printf("\nstarting\n");

  assert( sizeof(bool) == 1);
  assert( sizeof(float) == 4);
  assert( sizeof(double ) == 8);

  printf("4094 size %u\n", sizeof(_4094_state_t));
  assert( sizeof(_4094_state_t) == 12 );



  // init data
  decode_init( app.data );

  ////////////////
  // init the spi port, for adum/ice40 comms
  spi1_port_setup();

  // spi1_port_interrupt_setup( (void (*) (void *))spi1_interrupt, &app);

  spi1_port_interrupt_setup();

  // shouldnt setup the interrupt handler - until fpga is configured, else looks like get
  // spi1_port_interrupt_handler_set( (void (*) (void *)) decode_rdy_interrupt, app.data );
  ice40_port_extra_setup();





  // outer app loop, eg. bottom of control stack
  while(true)
    app_update_main( &app);


  return 0;
}
#endif


