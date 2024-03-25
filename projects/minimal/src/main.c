

#include <stdio.h>    // printf, scanf
#include <assert.h>



#include <libopencm3/stm32/rcc.h>   // mcu clock initialization
#include <libopencm3/stm32/spi.h>   // SPI1


#include <lib2/usart.h>
#include <lib2/util.h>      // systick_setup()



#include <peripheral/led.h>
#include <peripheral/spi-port.h>
#include <peripheral/ice40-extra.h>



#include <mode.h>
#include <app.h>
#include <util.h>   // CLK_FREQ


#include <data/data.h>     // to instantiate






static const _mode_t mode_initial =  {

  /*
    TODO,
    EXTR all relays need to be defined b01 or b10. otherwise a default initialization value of b00
    will mean they won't receive an initial pulse/value.

  */


  .first .K407  = LR_SET,     // disengage - dcv-source
  .first .K406  = LR_SET,     // disengage - accum cap
  .first .K405  = LR_RESET,   // select ch2 himux out


  .reg_mode = 0, // MODE_LO,                                  // default, blink led according to mcu

  // signal acquisition defaults
  .sa.reg_sa_p_clk_count_precharge = CLK_FREQ * 500e-6,             //  500us.

  .sa.reg_sa_p_seq_n = 2,

  .sa.reg_sa_p_seq0 = (0b01 << 4) |  S3,         // dcv
  .sa.reg_sa_p_seq1 = (0b00 << 4) | S7,         // star-lo
  .sa.reg_sa_p_seq2 = 0,  // channel-1 precharge switch
  .sa.reg_sa_p_seq3 = 0,  // channel-1 precharge switch



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



static _mode_t mode_current = { 0 } ;




static data_t data = {

  . magic = DATA_MAGIC,
  .line_freq = 50,

 } ;



static app_t app = {

  .magic = APP_MAGIC,

  .spi = SPI1 ,
  .mode_initial =  &mode_initial,
  .mode_current =  &mode_current,


  . data = &data
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
  assert_critical_error_led_setup( LED_PORT, LED_OUT);


  // mcu clock
  systick_setup(84000); // 84MHz.
  systick_handler_set( (void (*)(void *)) app_systick_interupt, &app );  // rename systick_handler_set()



  //////////////////////
  // main app setup
  // initialzes the console buffers, that support printf() and error reporting
  app_init_console_buffers( &app );


  //////////////
  // now can init usart peripheral using app console buffers
  usart1_setup_portB();

  usart1_set_buffers( &app.console_in, &app.console_out);



  printf("\n\n\n\n--------\n");
  printf("addr main() %p\n", main );

  printf("\n--------");
  printf("\nstarting\n");


  assert( sizeof(bool) == 1);
  assert( sizeof(float) == 4);
  assert( sizeof(double ) == 8);



  // init data
  data_init( app.data );



  ////////////////
  // init the spi port, for adum/ice40 comms
  spi1_port_setup();

  // spi1_port_interupt_setup( (void (*) (void *))spi1_interupt, &app);

  spi1_port_interupt_setup();

  // shouldnt setup the interupt handler - until fpga is configured, else looks like get
  //
  // spi1_port_interupt_handler_set( (void (*) (void *)) data_rdy_interupt, app.data );




  ice40_port_extra_setup();


  // outer app loop, eg. bottom of control stack
  while(true)
    app_update_main( &app);


  return 0;


}




