

#include <stdio.h>    // printf, scanf
#include <assert.h>



#include <libopencm3/stm32/rcc.h>   // mcu clock initialization

#include <libopencm3/stm32/gpio.h>    // needed to initialize critical_error_led blink.  context.

#include <lib2/usart.h>
#include <lib2/util.h>      // systick_setup()







#include <mode.h>
#include <app.h>
#include <util.h>   // CLK_FREQ


#include <data/data.h>     // to instantiate

/* split vfd - so the rst gpio setup  is moved to device.
*/
#include <peripheral/vfd.h>
#include <peripheral/spi-ice40.h>

#include <peripheral/interrupt.h>


#include <device/spi1-port.h>
#include <device/spi2-port.h>



#include <device/led0.h>
#include <device/fpga0.h>
#include <device/fpga0-pc.h>
#include <device/fpga0_interrupt.h>

#include <device/4094-0.h>
#include <device/mdac0.h>
#include <device/mdac1.h>


#include <device/trigger-internal.h>
#include <device/trigger-selection.h>


#include <device/fsmc.h>



#include <device/fpga1.h>


// #include <hal/hal.h>


static const _mode_t mode_initial =  {

  /*
  EXTR all relays should be defined b01 or b10.
  a default initialization of b00
  means they won't receive any initial pulse/value.


  mode->first.K402 = SR_RESET;    // input off
  mode->first.K406 = SR_RESET;    // accum off
  mode->first.K407 = SR_RESET;    // dcv-source off

  mode->first.K401 = SR_RESET;    // ohms off
  mode->first.K404 = SR_RESET;    // lts source off
  mode->first.K403 = SR_RESET;    // 10Meg impedance off

  */


  // U401
  .first. K404    = SR_RESET,
  .first. K402    = SR_RESET,
  .first. K405    = SR_RESET,
  .first. K406    = SR_RESET,


  // U402
  .second. U409    = DOFF,           //  input hi/lo mux. adg1209. 2x04.  dec 2024.

  .first. K407    = SR_RESET,
  .first. K403		= SR_RESET,


  // u405
  .first. K401    = SR_RESET,



  // amplifier
  // dont let feedback float, else current sources lock up
  .first . U506    =  S8,  // 1x gain, adg1208.  july 2025
  .second. U506    =  S8,  // 1x gain, adg1208.  july 2025


    //////////////

  .reg_mode = 0, // MODE_LO,                                  // default, blink led according to mcu

  // signal acquisition defaults
  .sa.p_clk_count_precharge = CLK_FREQ * 500e-6,             //  500us.

/*
  .sa.p_seq_n = 2,
  .sa.p_seq0 = (0b01 << 4) | S3,         // dcv
  .sa.p_seq1 = (0b00 << 4) | S7,         // star-lo
  .sa.p_seq2 = 0,  // channel-1 precharge switch
  .sa.p_seq3 = 0,  // channel-1 precharge switch
*/


  // adc
  .adc.p_aperture = CLK_FREQ * 0.2,   // 200ms. 10nplc 50Hz.  // Not. should use current calibration?  // should be authoritative source of state.
  .adc.p_reset = CLK_FREQ * 500e-6                // 500us.


#if 0

  //  maybe make explicit all values  U408_SW_CTL. at least for the initial mode, from which others derive.

  .first .K406_CTL  = SR_SET,     // accumulation relay off
  .first. K405_CTL  = SR_RESET,     // dcv input relay k405 switch off - works.
  .first. K402_CTL  = SR_RESET,     // dcv-div/directz relay off
                                // must match app->persist_fixedz

  .first. K401_CTL  = SR_SET,     // dcv-source relay off.
  .first. K403_CTL  = SR_RESET,     // ohms relay off.

  .first .U408_SW_CTL = 0,      // b2b fets/ input protection off/open


  // AMP FEEDBACK SHOULD NEVER BE TURNED OFF.
  // else draws current, and has risk damaging parts. mux pin 1. of adg. to put main amplifier in buffer/G=1 configuration.
  .first. U506 =  D1,     // should always be on

  // .second.K406_CTL  = LR_OFF,     // clear relay. default.
  // .second.K405_CTL  = LR_OFF,     // clear relay
  .second.U408_SW_CTL = 0,

  .second.U506 =  D1,           // amplifier should always be on.

  .first. K603_CTL  = SR_RESET,     // ohms relay off.


  /////////////////////////
  // 700
  // has inverting cmos buffer
  .first. K702_CTL  = SR_RESET,
  .second.K702_CTL  = 0b11,

  // 0.1R shunt off. has inverting cmos buffer
  .first. K703_CTL  = SR_RESET,
  .second.K703_CTL  = 0b11,

  // shunts / TIA - default to shunts
  .first. K709_CTL  = SR_SET,

  // agn200 shunts are off.
  .first. K707_CTL  = SR_SET,
  .first. K706_CTL  = SR_SET,
  .first. K704_CTL  = SR_SET,
  .first. K705_CTL  = SR_SET,


#endif

};



static _mode_t mode_current = { 0 } ;








static data_t data = {

  . magic = DATA_MAGIC,
  .line_freq = 50,

 } ;



static app_t app = {

  .magic = APP_MAGIC,

  // .spi = SPI1 ,

  .led_blink_enable = true,


  //////////////
  // device.
  // the led ought to be a structure with a function.

  // initialization
  // .led_status = PIN('A', 9 ),

  // device
  // . spi_u202 = &spi_u202,

  // .cdone_fpga0 = false,

  // ugly.
  .mode_initial =  &mode_initial,
  .mode_current =  &mode_current,


  . data = &data
};



#include <support.h>     // gpio_write_val()

// this all looks ok. to me.
// is something else trying

static void timer_port_setup(void )
{

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



#include <libopencm3/stm32/timer.h>

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
















static int main_f429(void)
{
  // use  f429.ld
  // hse
	// rcc_clock_setup_pll(&rcc_hse_8mhz_3v3[RCC_CLOCK_3V3_84MHZ] );  //  f413.
	// rcc_clock_setup_pll(&rcc_hse_8mhz_3v3[RCC_CLOCK_3V3_84MHZ] );  //  f429 will use ethernet clock, and different setup.

  rcc_clock_setup_pll(&rcc_hsi_configs[RCC_CLOCK_3V3_84MHZ ]);    // HSI WORKS stm32f410, may 2022, f407 2025.

  // clocks - TODO move after gpio ports?
  rcc_periph_clock_enable(RCC_SYSCFG); // maybe required for external interupts?

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

  /////////////////////////////
  /*
    peripheral/ports setup
  */


  // led0
  // app.status_led = led0_create();

  app.status_led = led0_create();
  gpio_setup( app.status_led);



  // setup external state required for critical error led blink in priority
  // because assert() cannot pass a context

  assert_critical_error_led_setup( GPIOA, GPIO9 );

  // mcu clock
  // systick_setup(12000); // 12MHz. default lsi.
  systick_setup(84000); // 84MHz.
  systick_handler_set( (void (*)(void *)) app_systick_interupt, &app );  // rename systick_handler_set()

  //////////////////////
  // main app setup
  // initialzes the console buffers, that support printf() and error reporting
  app_init_console_buffers( &app );

  /*
    dont' think it makes any sense to use a circular buffer on the output.
    instead just block control until the output is flushed.
  */
  //////////////
  // now can init usart peripheral using app console buffers
  usart1_setup_portB();

  usart1_set_buffers( &app.console_in, &app.console_out);

  printf("\n\n\n\n--------\n");
  printf("addr main() %p\n", main_f429);

  printf("\n--------");
  printf("\nstarting\n");

  assert( sizeof(bool) == 1);
  assert( sizeof(float) == 4);
  assert( sizeof(double ) == 8);

  // init data
  data_init( app.data );

  ////////////////
  // init spi related port state. before do spi port.
  // to prevent ice40 wanting to become spi master



  // now init spi ports
  spi1_port_setup();


  spi2_port_setup();



  // devices

  devices_t *devices = &app.devices;

  devices->spi_fpga0_pc = spi_fpga0_pc_create();
  spi_setup( (spi_t *) devices->spi_fpga0_pc );                // note upcast

  devices->spi_fpga0 = spi_fpga0_create();
  spi_setup( devices->spi_fpga0 );



  devices->fpga0_interrupt = fpga0_interrupt_create();
  interrupt_setup( devices->fpga0_interrupt);


  devices->spi_4094 = spi_4094_0_create();
  spi_setup( devices->spi_4094 );


  devices->spi_mdac0 = spi_mdac0_create();
  spi_setup( devices->spi_mdac0 );


  devices->spi_mdac1 = spi_mdac1_create();
  spi_setup( devices->spi_mdac1 );

  // should prefix with gpio?
  // probably. do it with sed.
  devices->trigger_selection = trigger_selection_create();
  gpio_setup( devices->trigger_selection);



  // trigger internal
  app.trigger_internal = trigger_internal_create();
  gpio_setup( app.trigger_internal);


  ///////////



  printf("----------\nsetting up timer\n");
  uint32_t timer = TIM5;
  timer_port_setup();
  timer_setup( timer );
  timer_set_frequency( timer, 15000 );    // need to know the main mcu clock freq to do this.
                                          // perhaps record with the timer?
                                          // as device.



#if 0


  ///////////////////////////////
    fsmc_gpio_setup();

    // fsmc_setup( 12 );   // slow.
    // with divider == 1. is is easier to see the address is already well asserted on WR rising edge. before CS.
    fsmc_setup( 1 );   // fase.
    vfd_init_gpio();
    msleep( 10, &app.system_millis );
    vfd_init(  &app.system_millis);

    // vfd_do_something();
  //////////////////

#endif


  // outer app loop, eg. bottom of control stack
  while(true) {
    app_update_main( &app);
  }


  return 0;
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
  rcc_periph_clock_enable(RCC_SYSCFG); // maybe required for external interupts?

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
  systick_handler_set( (void (*)(void *)) app_systick_interupt, &app );  // rename systick_handler_set()

  //////////////////////
  // main app setup
  // initialzes the console buffers, that support printf() and error reporting
  app_init_console_buffers( &app );

  /*
    dont' think it makes any sense to use a circular buffer on the output.
    instead just block control until the output is flushed.
  */
  //////////////
  // now can init usart peripheral using app console buffers
  usart1_setup_portB();

  usart1_set_buffers( &app.console_in, &app.console_out);

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
  data_init( app.data );

  ////////////////
  // init the spi port, for adum/ice40 comms
  spi1_port_setup();

  // spi1_port_interupt_setup( (void (*) (void *))spi1_interupt, &app);

  spi1_port_interupt_setup();

  // shouldnt setup the interupt handler - until fpga is configured, else looks like get
  // spi1_port_interupt_handler_set( (void (*) (void *)) data_rdy_interupt, app.data );
  ice40_port_extra_setup();





  // outer app loop, eg. bottom of control stack
  while(true)
    app_update_main( &app);


  return 0;
}
#endif



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

