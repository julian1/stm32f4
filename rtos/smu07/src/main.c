/*
  *********************
  EXTREME.

  should be

  xQueueSendFromISR() not xQueueSend()
  see p369.
    - fixed for usart.c
    - but do we have other ISRs using it?

  *********************

TODO
  libwwg uses its own FreeRTOSCOnfig.h
    which may differ from local.

  see top-level ../../Makefile
  actually no. think it comes from Makefile.incl.

  p357.
    must fix.
    by copying src to local

    from ../Makefile.inc

    maybe just move...
    mini_vprintf_cooked()
*/


/*
  see, for
  ~/devel/stm32/FreeRTOSv10.3.1/FreeRTOS/Demo/CORTEX_M4F_STM32F407ZG-SK/FreeRTOSConfig.h

  doc,
  https://www.freertos.org/FreeRTOS-for-STM32F4xx-Cortex-M4F-IAR.html

  so, i think 'proper' usart will use dma.
*/
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"


#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>
// #include <libopencm3/stm32/timer.h>
// #include <libopencm3/stm32/spi.h>
// #include <libopencm3/stm32/gpio.h>
// #include <libopencm3/stm32/usart.h>
// #include <libopencm3/cm3/nvic.h>
// #include <libopencm3/stm32/adc.h>
// #include <libopencm3/stm32/exti.h>




#include "led.h"
#include "pvd.h"
#include "sleep.h"
#include "usart.h"
// #include "led.h"
#include "rails.h"
#include "ref.h"
#include "dac8734.h"
#include "mcu_adc.h"
// #include "mux.h"
#include "slope_adc.h"


// we need a continually running task to pull down rails - again... when the rails go down.
// *and* when digital goes down.

// we need setup()/ and clear()

static void rails_wait_for_voltage(void)
{
  /*
  // move to rails?
  // potentially want a continuous running task to monitor, with queue events - not just a blocking call.
  // also want to pause for a couple of ticks... before unblock
  */
  int tick = 0;

  int good = 0; // count of how many ticks it's ok...

  while(1) {
    uint16_t pa0 = mcu_adc_read_native(0);   // LP15VP
    uint16_t pa1 = mcu_adc_read_native(1);   // LN15VN

    // only report first time...
    // if(tick == 0)
    uart_printf("rails_wait_for_voltage, tick: %d: LP15VP=%u, LN15VN=%d\n", tick++, pa0, pa1);

    if(pa0 > 1000 && pa1 > 1000)
      ++good;
    else
      good = 0;

    if(good >= 3)
      break;

    task_sleep(500);
  }
}





////////////////////////////

#define RELAY_PORT              GPIOD
#define OUTPUT_RELAY_CTL        GPIO9
#define COMXY_RELAY_CTL         GPIO10
#define VRANGE_RELAY_CTL        GPIO11

static void relay_setup(void)
{
  const uint16_t all = OUTPUT_RELAY_CTL | COMXY_RELAY_CTL | VRANGE_RELAY_CTL;

  gpio_clear(RELAY_PORT, all);   // off
  gpio_mode_setup(RELAY_PORT, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, all);

}

static void relay_toggle_test_task(void *args __attribute((unused)))
{
  // need to move to set of useful functions.
  // albeit - must move all header definitinos to header file. or somewhere include
  // useful test function.
  int tick = 0;
	for (;;) {

    uart_printf("relay toggle %d\n", tick++);
    // gpio_toggle(RELAY_PORT, OUTPUT_RELAY_CTL);
    // gpio_toggle(RELAY_PORT, VRANGE_RELAY_CTL);
    gpio_toggle(RELAY_PORT, COMXY_RELAY_CTL);


    task_sleep(1000); // 1Hz
	}
}






/////////////////////////////


/*
// 65535 increment
// 6.55V   ref. for ref * 2.
*/

static int voltageToDac( float x)
{
  // return x / (6.5769 * 2) * 65535;
  // eg.
  return x / 2.0 * 10000;  // need to add ptf56 60ohm, and then use dac trim registers.
}



/////////////////////////////
/*
// need better names sense1 sense2 ctl.
// it's muxing the inas. so calls it irange_sense_mux etc...
//
#define IRANGE_MUX2_PORT  GPIOC_
#define IRANGE_MUX2_FET12_CTL         GPIO12
#define IRANGE_MUX2_FET34_CTL         GPIO13
#define IRANGE_MUX2_COMBUFFERED_CTL   GPIO14    // this is not buffered...
#define IRANGE_MUX2_UNUSED_CTL        GPIO15
*/

#define IRANGE_SENSE_PORT           GPIOC
#define IRANGE_SENSE_1_CTL          GPIO12
#define IRANGE_SENSE_2_CTL          GPIO13
#define IRANGE_SENSE_3_CTL          GPIO14    // this is not buffered...
#define IRANGE_SENSE_UNUSED_CTL     GPIO15


static void irange_sense_setup(void)
{
  const uint16_t all = IRANGE_SENSE_1_CTL | IRANGE_SENSE_2_CTL | IRANGE_SENSE_3_CTL | IRANGE_SENSE_UNUSED_CTL;
  gpio_set(IRANGE_SENSE_PORT, all);     // active lo
  gpio_mode_setup(IRANGE_SENSE_PORT, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, all);
}





/////////////////////////////

#define RANGE_OP_PORT     GPIOD // change name...
#define VRANGE_OP1_CTL      GPIO12
#define VRANGE_OP2_CTL      GPIO13
#define IRANGE_OP1_CTL      GPIO14
#define IRANGE_OP2_CTL      GPIO15

static void range_op_setup(void)
{
  const uint16_t all = VRANGE_OP1_CTL | VRANGE_OP2_CTL | IRANGE_OP1_CTL | IRANGE_OP2_CTL;

  // dg333 is active hi, at least - in that it changes from non default position.
  gpio_set(RANGE_OP_PORT, all);     // gain to x1 for all 4x ops.


  gpio_mode_setup(RANGE_OP_PORT, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, all);
}


// IRANGE_OP2_CTL is not right.  it's low. when should be hi.
// maybe because of unpopulated input voltages?
// pin 20. top pin on dg333.
// pin62 on mcu.
// bad solder join on mcu.

// want to fint ina128 - instrument amp - on the current sense resistor.
// eg. to check, working.


// want to check diode voltage drop - across dual fet current switch.
// for both source positive and negative voltage.

// rather than plug usb in endlessly. can we use gpio (on stm32, or usb connector) to turn on/off 5V power
// this would just about be worthwhile. to add to the board.
// we can already control rails.
// just a single p-fet in front of everything else. mcu can already control rails.


/////////////////////////////

// U14 power n&p fet switches
#define IRANGE_PORT         GPIOC
#define IRANGE_SW1_CTL      GPIO0
#define IRANGE_SW2_CTL      GPIO1
#define IRANGE_SW3_CTL      GPIO2
#define IRANGE_SW4_CTL      GPIO3


// U46 jfets
#define IRANGE_SW9_CTL      GPIO8
#define IRANGE_SW10_CTL     GPIO9
#define IRANGE_SW11_CTL     GPIO10
#define IRANGE_SW12_CTL     GPIO11



static void irange_sw_setup(void)
{

  const uint16_t u14 = IRANGE_SW1_CTL | IRANGE_SW2_CTL | IRANGE_SW3_CTL | IRANGE_SW4_CTL;     // power n&p fet switches
  const uint16_t u46 = IRANGE_SW9_CTL | IRANGE_SW10_CTL | IRANGE_SW11_CTL | IRANGE_SW12_CTL;  // jfet switches.
  const uint16_t all = u14 | u46;

  // set +15V to gate. not default
  gpio_clear(IRANGE_PORT, all);


  gpio_mode_setup(IRANGE_PORT, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, all);

}


// -15top
// 15bottom



/////////////////////////////

// U12
#define MUX_PORT            GPIOE

#define MUX_MIN_CTL         GPIO8
#define MUX_INJECT_AGND_CTL GPIO9
#define MUX_INJECT_VFB_CTL  GPIO10
#define MUX_MAX_CTL         GPIO11

// U1
#define MUX_VSET_INV_CTL    GPIO0
#define MUX_VFB_INV_CTL     GPIO1
#define MUX_VFB_CTL         GPIO2
#define MUX_VSET_CTL        GPIO3

// U8
#define MUX_ISET_INV_CTL    GPIO4
#define MUX_IFB_INV_CTL     GPIO5
#define MUX_IFB_CTL         GPIO6
#define MUX_ISET_CTL        GPIO7




static void mux_setup(void)
{
  const uint16_t u12 = MUX_MIN_CTL | MUX_INJECT_AGND_CTL | MUX_INJECT_VFB_CTL | MUX_MAX_CTL;
  const uint16_t u1 = MUX_VSET_INV_CTL | MUX_VFB_INV_CTL | MUX_VFB_CTL | MUX_VSET_CTL;
  const uint16_t u8 = MUX_ISET_INV_CTL | MUX_IFB_INV_CTL | MUX_IFB_CTL | MUX_ISET_CTL;

  const uint16_t all = u12 | u1 | u8;


  gpio_set(MUX_PORT, all); // active lo
  gpio_mode_setup(MUX_PORT, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, all);
}

static void mux_regulate_vfb_direct(void)
{
  gpio_clear(MUX_PORT, MUX_INJECT_VFB_CTL); // active lo

}


// it's pretty hard to test this...
// think we need to step through everything.  
// it's confusing that the set input to the summer - is not inverted.
// maybe we have it wrong. and we need the inv_fb. and on both?
// eg. everything is inverted.

static void mux_regulate_p5v(void)
{

  // set for 10V
  dac_write_register(DAC_VSET_REGISTER, voltageToDac( 10.0 ));
  dac_write_register(DAC_ISET_REGISTER, voltageToDac( 0.2 ));   // 2mA
  // dac_write_register(DAC_ISET_REGISTER, voltageToDac( 2 )); // 20mA.


  // summer is non-inverting. so must give 2x inputs . else it will multiply single by x2.
  // otherwise we get value multiplied by 2.
  gpio_clear(MUX_PORT, MUX_VSET_INV_CTL | MUX_VFB_CTL ); // source positive voltage. regulate +6V eg. source.
  // gpio_clear(MUX_PORT, MUX_VSET_CTL | MUX_VFB_CTL );        //  source negative voltage. still source. regulate -6V, eg. sink. still max.


  gpio_clear(MUX_PORT, MUX_ISET_INV_CTL | MUX_IFB_CTL );    // this sources positive current. works.
  // gpio_clear(MUX_PORT, MUX_ISET_CTL | MUX_VFB_CTL );        // source negative current
                                                            // issue is ne5532 does not handle large input offset voltage
                                                            // and input protection starts sinks current.

  // ok. it's not regulating in reverse on current...

  // max is correct for sourcing. because verr,ierr, and err are inverted.
  gpio_clear(MUX_PORT, MUX_MAX_CTL);      // outputs .... 
  // gpio_clear(MUX_PORT, MUX_MIN_CTL);  // minimum . 

  // ok. maybe negative is not sorking...

  // turn fets 1 and 2 on - for current range 1
  // gpio_set(IRANGE_PORT, IRANGE_SW1_CTL | IRANGE_SW2_CTL);

  // turn on fets 2,3 for current range 2
  gpio_set(IRANGE_PORT, IRANGE_SW3_CTL | IRANGE_SW4_CTL);

  // turn on current sense1 ina
  // gpio_clear(IRANGE_SENSE_PORT, IRANGE_SENSE_1_CTL);

  // turn on current sense2 ina. builtin gain is 10x
  gpio_clear(IRANGE_SENSE_PORT, IRANGE_SENSE_2_CTL);


  // set x1 gain for both vrange ops
  gpio_set(RANGE_OP_PORT, VRANGE_OP1_CTL);
  gpio_set(RANGE_OP_PORT, VRANGE_OP2_CTL);


  // set x1 gain for irange ops... default
  gpio_set(RANGE_OP_PORT, IRANGE_OP1_CTL);

  // set x10  for irange op. works.
  // gpio_clear(RANGE_OP_PORT, IRANGE_OP1_CTL);


  // turn output relay on
  gpio_set(RELAY_PORT, OUTPUT_RELAY_CTL);   // on

/*
  ina gain is 10x. op-gain is 10x.

  eg.
    3A    range use 10x. across 0.1ohm (not 1000x).
    1A    range use 100x  (10x + 10x) across 0.1ohm.
    100mA range use 10x across 10ohm 1/2 watt.
    10mA  range use 100x acroos 10ohm (test). or 10x across 100ohm, or 1x across 1k. - need fets / or dg444 maybe.
*/
}



static void mux_regulate_n5v(void)
{
  // OK. working to source negative voltage. and negative current.
  // but cannot combine.

  // set for 10V
  dac_write_register(DAC_VSET_REGISTER, voltageToDac( 10.0 ));
  dac_write_register(DAC_ISET_REGISTER, voltageToDac( 0.2 ));   // 2mA
  // dac_write_register(DAC_ISET_REGISTER, voltageToDac( 2 )); // 20mA.


  gpio_clear(MUX_PORT, MUX_VSET_CTL | MUX_VFB_CTL ); // ok - this works by itself. to source negative 10V voltage
  gpio_clear(MUX_PORT, MUX_ISET_CTL | MUX_IFB_CTL  );  // source negative current.  
  gpio_clear(MUX_PORT, MUX_MIN_CTL);  // minimum . 


  // turn fets 1 and 2 on - for current range 1
  // gpio_set(IRANGE_PORT, IRANGE_SW1_CTL | IRANGE_SW2_CTL);

  // turn on fets 2,3 for current range 2
  gpio_set(IRANGE_PORT, IRANGE_SW3_CTL | IRANGE_SW4_CTL);

  // turn on current sense1 ina
  // gpio_clear(IRANGE_SENSE_PORT, IRANGE_SENSE_1_CTL);

  // turn on current sense2 ina. builtin gain is 10x
  gpio_clear(IRANGE_SENSE_PORT, IRANGE_SENSE_2_CTL);


  // set x1 gain for both vrange ops
  gpio_set(RANGE_OP_PORT, VRANGE_OP1_CTL);
  gpio_set(RANGE_OP_PORT, VRANGE_OP2_CTL);


  // set x1 gain for irange ops... default
  gpio_set(RANGE_OP_PORT, IRANGE_OP1_CTL);

  // set x10  for irange op. works.
  // gpio_clear(RANGE_OP_PORT, IRANGE_OP1_CTL);


  // turn output relay on
  gpio_set(RELAY_PORT, OUTPUT_RELAY_CTL);   // on

}



//////////////////////////


static void mux_regulate_jfet(void)
{

  // set for 10V
  dac_write_register(DAC_VSET_REGISTER, voltageToDac( 10.0 ));
  dac_write_register(DAC_ISET_REGISTER, voltageToDac( 8.0 ));   // 5V across 10k == 0.5mA

  // gpio_clear(MUX_PORT, MUX_VSET_INV_CTL | MUX_VFB_CTL ); // source positive voltage. compliance. 
                                                              // ok - single elment feedback works. cool. just regulating on current.
  gpio_clear(MUX_PORT, MUX_ISET_INV_CTL | MUX_IFB_CTL );   // source positive current. function. 
  // gpio_clear(MUX_PORT, MUX_ISET_CTL | MUX_IFB_CTL );       // source negative current. function. 

  // max for correct for sourcing. because verr,ierr, and err are inverted.
  gpio_clear(MUX_PORT, MUX_MAX_CTL);


  // turn on sense amp 3.
  gpio_clear(IRANGE_SENSE_PORT, IRANGE_SENSE_3_CTL);


  // turn on jfet 1. switch 9.
  // gpio_set(IRANGE_PORT, IRANGE_SW9_CTL );

  // turn on jfet 2, switch 10 
  // gpio_set(IRANGE_PORT, IRANGE_SW10_CTL );
  // turn on jfet 3. switch 11,
  gpio_set(IRANGE_PORT, IRANGE_SW11_CTL );

  // set x1 gain for both vrange ops
  gpio_set(RANGE_OP_PORT, VRANGE_OP1_CTL);
  gpio_set(RANGE_OP_PORT, VRANGE_OP2_CTL);

  // set x1 gain for irange ops... default
  gpio_set(RANGE_OP_PORT, IRANGE_OP1_CTL);


  // set to Y return current path 
  gpio_set(RELAY_PORT, COMXY_RELAY_CTL);

  // turn output relay on
  gpio_set(RELAY_PORT, OUTPUT_RELAY_CTL);   // on

/*
  ina gain is 10x. op-gain is 10x.

  eg.
    3A    range use 10x. across 0.1ohm (not 1000x).
    1A    range use 100x  (10x + 10x) across 0.1ohm.
    100mA range use 10x across 10ohm 1/2 watt.
    10mA  range use 100x acroos 10ohm. or 1x across 1k.

*/
}



/////////////////////////////



static void test01(void *args __attribute((unused)))
{

  uart_printf("test01\n");

  dac_reset();
  refa_off();

  // needs to be a task that ressets everything if voltages go off again.
  // would be much easier.
  // we don't have to keep doing a reset.
  rails_wait_for_voltage();

  task_sleep(50);
  rails_negative_on();
  task_sleep(50);
  rails_positive_on();
  task_sleep(50);


  refa_on();

  dac_write_register(DAC_VSET_REGISTER, /*voltageToDac( 0 )*/ 0 );
  dac_write_register(DAC_ISET_REGISTER, /* voltageToDac( 0 ) */ 0);


  task_sleep(50);

  // mux_regulate_vfb_direct();
  // mux_regulate_p5v();
  mux_regulate_n5v();
  // mux_regulate_jfet();

  // turn on rails... should set regulate on vfb first... then bring up rails.. then regulate
  rails_p30V_on();
  rails_n30V_on();



#if 0
  // source_current_test();
  // adc_test();
  // adc_ifb_test();
  // adc_vfb_test();
#endif

  uart_printf("test01 done\n");


  // sleep forever
  for(;;) {
    task_sleep(1000);
  }
}








/////////////////////////////






int main(void)
{

  // main clocks
  // disable HSE
  // must edit, configCPU_CLOCK_HZ also
  // clocks
  // rcc_clock_setup_pll(&rcc_hse_8mhz_3v3[RCC_CLOCK_3V3_168MHZ]);


  // led
  rcc_periph_clock_enable(RCC_GPIOA);

  // dac / gpio
  rcc_periph_clock_enable(RCC_GPIOB);

  // irange
  rcc_periph_clock_enable(RCC_GPIOC);

  // relay
  rcc_periph_clock_enable(RCC_GPIOD);

  // rails / mux
  rcc_periph_clock_enable(RCC_GPIOE);


  // usart
  rcc_periph_clock_enable(RCC_GPIOA);
  rcc_periph_clock_enable(RCC_USART1);

  // spi1 / dac
  rcc_periph_clock_enable(RCC_SPI1);

  // mcu adc1
	rcc_periph_clock_enable(RCC_ADC1);

  // power voltage detect
	rcc_periph_clock_enable(RCC_PWR);


  ///////////////
  // setup
  led_setup();

  usart_setup();
  uart_printf("------------------\n\r");
  uart_printf("starting\n\r");


  power_voltage_detect_setup();

  relay_setup();
  rails_setup();
  mcu_adc_setup();

  ref_setup();
  dac_setup_spi();


  mux_setup();
  irange_sw_setup();
  range_op_setup();
  irange_sense_setup();



  ///////////////
  // setup

	xTaskCreate(led_blink_task,  "LED",100,NULL,configMAX_PRIORITIES-1,NULL);


  // IMPORTANT changing from 100 to 200, stops deadlock
  xTaskCreate(uart_task,        "UART",200,NULL,configMAX_PRIORITIES-1,NULL); /* Highest priority */
  xTaskCreate(usart_prompt_task,"PROMPT",200,NULL,configMAX_PRIORITIES-2,NULL); /* Lower priority */



  xTaskCreate(test01,        "TEST01",200,NULL,configMAX_PRIORITIES-2,NULL); // Lower priority

	// xTaskCreate(relay_toggle_test_task,  "RELAY_TEST",100,NULL,configMAX_PRIORITIES-1,NULL);


  // xTaskCreate(report_pvd_test_task,    "PVD_TEST",200,NULL,configMAX_PRIORITIES-2,NULL); // Lower priority



	vTaskStartScheduler();

  // should never get here?
	for (;;);
}




#if 0

int main_old(void) {

  // main clocks
  // disable HSE
  // must edit, configCPU_CLOCK_HZ also
  // clocks
  // rcc_clock_setup_pll(&rcc_hse_8mhz_3v3[RCC_CLOCK_3V3_168MHZ]);

  // led
  rcc_periph_clock_enable(RCC_GPIOE); // JA

  // Dac
  rcc_periph_clock_enable(RCC_GPIOB); // JA

  // usart
  rcc_periph_clock_enable(RCC_GPIOA);
  rcc_periph_clock_enable(RCC_USART1);

  // spi1 / dac
  rcc_periph_clock_enable(RCC_SPI1);

  // mcu adc1
	rcc_periph_clock_enable(RCC_ADC1);

  // adc
  rcc_periph_clock_enable(RCC_GPIOD);



  ///////////////
  // setup
  // TODO maybe change names setup_led() setup_uart() ?
  led_setup();
  usart_setup();
  uart_printf("------------------\n\r");
  uart_printf("starting\n\r");

  // /////////////
  // EXTREME - gpio, clocks, and peripheral configuration ONLY.
  // no actual spi calls
  mcu_adc_setup();
  rails_setup();
  dac_setup_spi();
  // dac_setup_bitbash();
  ref_setup();
  mux_setup();
  slope_adc_setup();

  uart_printf("------------------\n\r");

  ///////////////
  // tasks
  // value is the stackdepth.
	xTaskCreate(led_blink_task,  "LED",100,NULL,configMAX_PRIORITIES-1,NULL);
  xTaskCreate(uart_task,        "UART",200,NULL,configMAX_PRIORITIES-1,NULL); /* Highest priority */

  // IMPORTANT changing from 100 to 200, stops deadlock
  xTaskCreate(usart_prompt_task,"PROMPT",200,NULL,configMAX_PRIORITIES-2,NULL); /* Lower priority */

  xTaskCreate(test01,        "DAC_TEST",200,NULL,configMAX_PRIORITIES-2,NULL); // Lower priority

	vTaskStartScheduler();

  // should never get here?
	for (;;);
	return 0;
}

#endif


