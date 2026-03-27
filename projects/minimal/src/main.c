
/*
  create app dependencies
  create app,
  run app_update()

*/

#include <stdio.h>    // printf, scanf
#include <assert.h>
#include <string.h>


#include <libopencm3/stm32/rcc.h>   // mcu clock initialization
#include <libopencm3/stm32/gpio.h>    // required to initialize critical_error_led blink.  context.
#include <libopencm3/stm32/timer.h>

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

#include <display_vfd.h>

#include <agg/test.h> // consider better name. display_agg_test



// the flash addresses used for cal. should be defined here, and passed as dependencies


// mar 2024.
// see link/f413rgt6.ld
// #  7: 0x00060000 (0x20000 128kB) not protected
// # 11: 0x000e0000 (0x20000 128kB) not protected
#define FLASH_SECT_ADDR   0x080e0000                  // uint32_t
#define FLASH_SECT_NUM    11                          // uint8_t





// this all looks ok. to me.
// is something else trying

static void timer_port_setup(void )
{
  assert(0);

  printf("timer port setup\n");

  // PA0.   TIM5/CH1


  //  port set up for alt function.
  gpio_mode_setup(GPIOA, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO0);
  gpio_set_af(GPIOA, GPIO_AF2, GPIO0 );
  gpio_set_output_options(GPIOA, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, GPIO0);



/*
  // ok. this code works. to set, pin value as gpio.
  gpio_mode_setup( GPIOA, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, GPIO0);
  gpio_set_output_options( GPIOA, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, GPIO0);
  gpio_write_val(GPIOA, GPIO0 , 0 );

*/
}




static void timer_setup( uint32_t timer )
{
  // timer counter is peripheral.

  // rcc_periph_clock_enable(RCC_TIM5);

  printf("timer setup\n");

  // uint32_t timer = TIM5;
  assert( timer == TIM5 );

  if(timer == TIM5)
    rcc_periph_reset_pulse( RST_TIM5 );   // is this needed?
  else
    assert(0);

  timer_set_prescaler(timer, 0 );  // No prescaler = 42Mhz.

  timer_set_mode(timer, TIM_CR1_CKD_CK_INT, TIM_CR1_CMS_CENTER_1, TIM_CR1_DIR_UP);  // alternating up/down

  // timer_enable_counter(timer);
}



static void timer_set_frequency( uint32_t timer, uint32_t freq /*, uint32_t deadtime */ )
{
  /* output channel is device.
    - but cannot share easily,
      although oc channel only needs halfperiod, the timer itselfs needs period.
      so cannot hang another simple square wave off the same timer.
      - so should probably treat everything as a device.
  */

  // assert(deadtime >= 1 /*&& deadtime <= 50 */);
  // assert(freq >= 40000 && freq <= 500000);
  // assert(freq >= 10000 && freq <= 500000);
  assert(freq >= 10000 && freq <= 100000);
  assert( timer == TIM5 );

  timer_disable_counter(timer);

  // uint32_t freq = 200 * 1000;               // in Hz.
  // double clk_period = 2 / 84000000.f ;

  uint32_t period = (84000000.f / freq) / 2; // calculated.
  uint32_t half_period = period / 2;

  printf("------\n");
  printf("freq          %.1f kHz\n", freq / 1000.f );
  printf("clk period    %lu\n", period );

  // timer_enable_break_main_output(timer);
  timer_set_period(timer, period );    // 42kHz

  // 1
  timer_enable_oc_output(timer, TIM_OC1 );
  timer_set_oc_mode(timer, TIM_OC1 , TIM_OCM_PWM1);    // Output is active (high) when counter is less than output compare value
  timer_set_oc_value(timer, TIM_OC1, half_period );


  timer_set_counter( timer, 0 );    // make sure timer count does  not escape when shortening period

  timer_enable_counter(timer);  // seems to need this

}




#if 0
static void msleep(uint32_t delay, volatile uint32_t *system_millis)
{
  // temporary. repeated in vfd.c

  // works for system_millis integer wrap around
  // could be a do/while block.
  uint32_t start = *system_millis;
  while (true) {
    uint32_t elapsed = *system_millis - start;
    if(elapsed > delay)
      break;

    // yield()
  };

}

#endif




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




  // rcc_set_mco1_source(RCC_CFGR_MCO1_LSE);



  /////////////////////////////
  /*
    peripheral/ports setup
  */



  //////////////////////
  // main app setup


  // nothing wrong with stack allocation here

/*
  we can probably use C11 style initialization. for app_t.
  and pass all the references on construction. and by name

  just avoid using the app_init() and set the magic.

  and in this way avoid setters. and dependency issues.
*/


  // app_init( &app);


  volatile uint32_t system_millis  = 0;

  ///////////////////

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




  ///////////////////

  gpio_t  gpio_status_led;
  // app.gpio_status_led = &gpio_status_led;
  gpio_status_led_init( &gpio_status_led);
  gpio_setup( &gpio_status_led);

  // setup external state required for critical error led blink in priority
  // because assert() cannot pass a context

  assert_critical_error_led_setup( GPIOA, GPIO9 );





#if 0
  // mcu clock
  // systick_setup(12000); // 12MHz. default lsi.
  systick_setup( 84000); // 84MHz.
  systick_handler_set( (void (*)(void *)) app_interrupt_systick, &app );
#endif


  // could put this in
  interrupt_systick_t   interrupt_systick;
  // app.interrupt_systick = &interrupt_systick;
  interrupt_systick_init( &interrupt_systick, 84000); // 84MHz.
  interrupt_setup( &interrupt_systick);




  ///////////////////


  /*
    the circular buffer on the output, makes no sense.
    instead just block control until the output is flushed.
  */
  //////////////
  // now can init usart peripheral using app console buffer
  usart1_setup_portB();

  usart1_set_buffers( &cbuf_console_in, &cbuf_console_out);

  printf("\n\n\n\n--------\n");
  printf("addr main() %p\n", main_f429);

  printf("\n--------");
  printf("\nstarting\n");



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
  // app.spi_fpga0_pc = &spi_fpga0_pc;
  spi_fpga0_pc_init( &spi_fpga0_pc);
  spi_setup( &spi_fpga0_pc );




  spi_t           spi_fpga0;
  // app.spi_fpga0 = &spi_fpga0;
  spi_fpga0_init( &spi_fpga0);
  spi_setup( &spi_fpga0);




  interrupt_t     interrupt_fpga0;
  // app.interrupt_fpga0 = &interrupt_fpga0;
  interrupt_fpga0_init( &interrupt_fpga0);
  interrupt_setup( &interrupt_fpga0);




  /////////

  spi_ice40_t      spi_fpga1_pc;
  // app.spi_fpga1_pc = &spi_fpga1_pc ;
  spi_fpga1_pc_init( &spi_fpga1_pc);
  spi_setup( &spi_fpga1_pc );


  spi_t    spi_fpga1;
  // app.spi_fpga1 = &spit_fpga1;
  spi_fpga1_init( &spi_fpga1);
  spi_setup( &spi_fpga1 );


  spi_t    spi_4094_0;
  // app.spi_4094 = &spi_4094_0;
  spi_4094_0_init( &spi_4094_0);
  spi_setup( &spi_4094_0);

  spi_t   spi_mdac0;
  // app.spi_mdac0 = &spi_mdac0;
  spi_mdac0_init( &spi_mdac0);
  spi_setup( &spi_mdac0 );


  spi_t   spi_mdac1;
  // app.spi_mdac1 = &spi_mdac1;
  spi_mdac1_init( &spi_mdac1);
  spi_setup( &spi_mdac1 );


  gpio_t    gpio_trigger;
  // app.gpio_trigger = &gpio_trigger;
  gpio_trigger_init( &gpio_trigger);
  gpio_setup( &gpio_trigger);


  gpio_t    gpio_trigger_source;
  // app.gpio_trigger_source = &gpio_trigger_source;
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
  // app.mode = &mode;


  // app.ranges    = range_init_values;
  // app.ranges_sz = range_init_sz;


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
  // app.cal = &cal;



  // app.line_freq = 50;
  uint32_t      line_freq = 50;
  unsigned      range_idx;
  // both line_freq and range_idx  are going to have to be defined here with memory..
  //


  decode_t        decode;
  decode_init(
    &decode,
    &spi_fpga0,
    &cal,
    range_init_values,
    &range_idx,
    &line_freq
  );
  // app.decode = & decode;






  double values[ 1000];

  buffer_t     buffer;
  buffer_init( &buffer, values, ARRAY_SIZE(values));
  // app.buffer = &buffer;





  // only gpio is possible for vfd, and tft, because
  // of dependency on fpga. also system_millis
  // full init must wait for system_millis and fpga
  vfd_t         vfd0;
  // app.vfd0      = &vfd0;
  vfd0_init( &vfd0);
  vfd0.vfd_gpio_setup( &vfd0 );   // polymorphic, for multiple instances


  display_vfd_t   display_vfd;
  display_vfd_init( &display_vfd, &vfd0, &buffer);
  // app.display_vfd = &display_vfd;






  // tft
  tft_t     tft0;
  // app.tft = &tft0;
  tft0_init( &tft0);    // device specific
  tft0.tft_gpio_setup( &tft0);    // polymorphic, for multiple instances


  /* consider make system_millis a pointer in app_t.
    since it is passed/shared to other structures at _init (eg. agg_test).
    and instantiate it here in main.c
  */

  // agg test
  agg_test_t    agg_test;
  // app.agg_test = &agg_test;
  agg_test_init( &agg_test, &tft0, &system_millis );

  // OK. system_millis is going to need to be a pointer. if share with
  // the agg test code.



  app_t app = {

    .magic              = APP_MAGIC,

    .system_millis      = &system_millis,

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

    // these are extern declared
    // not clear these even need to be in app_t
    .ranges             = range_init_values,
    .ranges_sz          = range_init_sz,

    .cal                = &cal,

    // review - does app_t need these
    .line_freq          = &line_freq,
    .range_idx          = &range_idx,

    .decode             = &decode,
    .buffer             = &buffer,

    .vfd0               = &vfd0,        // needed because in app_t. because init() called only after init.
    .display_vfd        = &display_vfd,


    .tft                = &tft0,
    .agg_test           = &agg_test,

  } ;

  // we have to delay this, until app has been instantiated
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


