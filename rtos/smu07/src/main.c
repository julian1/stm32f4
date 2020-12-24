/*
  *********************
  EXTREME.

  should be

  xQueueSendFromISR() not xQueueSend()
  see p369.

  *********************

  libwwg uses its own FreeRTOSCOnfig.h
    which may differ from local.

  p357.
    must fix.
    by copying src to local


    from ../Makefile.inc

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


// #include <libopencm3/cm3/nvic.h>
#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>
// #include <libopencm3/stm32/timer.h>
// #include <libopencm3/stm32/spi.h>
// #include <libopencm3/stm32/gpio.h>
// #include <libopencm3/stm32/usart.h>
// #include <libopencm3/cm3/nvic.h>

#include <libopencm3/stm32/adc.h>


#include "sleep.h"
#include "usart.h"
#include "led.h"
#include "rails.h"
#include "ref.h"
#include "dac8734.h"


static void adc_out_print(void);


static void led_blink_task2(void *args __attribute((unused)))
{

	for (;;) {

    // led_toggle
		led_toggle();

    // adc_out_print();

  // we are going to have to set up a timer/0v
  // get adc value...



#if 0
    static int tick = 0;

    uart_printf("hi %d %d %d\n\r",
      tick++,
      gpio_get(DAC_PORT, DAC_GPIO0),
      gpio_get(DAC_PORT, DAC_GPIO1 )
    );
#endif

#if 0
    // So we need to wait until supplies come up when initializing.
    // which means factorizing this code.
    // but where do we put it...
    // actually - monitoring supplies should almost be a separate task...

    // Note that we should be able to talk to the dac / gpio - even if do not have
    // rails or ref up.

		uint16_t pa0 = mcu_adc_read_native(0);   // LP15VP
		uint16_t pa1 = mcu_adc_read_native(1);   // LN15VN
		uint16_t pa2 = mcu_adc_read_native(2);   // dacmon - need to cut trace
                                        // should test it works though

		uart_printf("tick: %d: LP15VP=%u, LN15VN=%d, pa2=%d\n", tick++, pa0, pa1, pa2 );

#endif

#if 0
    // at gnd 0-2, at 3.3V supply get 4095.  eg. 4096 = 12bit. good. but maybe resolution is off.
    /*
      ADC channel numbers
      http://libopencm3.org/docs/latest/stm32f4/html/group__adc__channel.html
    */

    // don't think this works...
		uint16_t vref = mcu_adc_read_native( ADC_CHANNEL_VREF);
		uint16_t vbat = mcu_adc_read_native( ADC_CHANNEL_VBAT);
		uint16_t temp = mcu_adc_read_native( ADC_CHANNEL_TEMP_F40 );

    /*
      OK
      VREF seems quite stable...  // 700 to 704
      VBAT also 745 to 750
      so maybe its working...
      TEMP_F40 went 770 to 850 with heat gun... pointed at it. and back again.
    */
		uart_printf("tick: %d: pa0=%u vbat=%d  vref=%d temp=%d\n", tick++, pa0, vbat, vref, temp);
#endif

    task_sleep(500);
  }
}




static uint16_t mcu_adc_read_native(uint8_t channel);

static void rails_wait_for_voltage(void)
{
  // move to rails?
  // potentially want a continuous running task to monitor, with queue events - not just a blocking call.
  // also want to pause for a couple of ticks... before unblock
  int tick = 0;

  while(1) {
    uint16_t pa0 = mcu_adc_read_native(0);   // LP15VP
    uint16_t pa1 = mcu_adc_read_native(1);   // LN15VN

		uart_printf("rails_wait_for_voltage, tick: %d: LP15VP=%u, LN15VN=%d\n", tick++, pa0, pa1);
    if(pa0 > 300 && pa1 > 300)
      break;

    task_sleep(500);
  }
}


static void power_up(void)
{
  uart_printf("\n\r");
  uart_printf("power_up start\n\r");

  // important - configure before rails
  dac_reset();


  rails_wait_for_voltage();

  task_sleep(50);
  rails_negative_on();
  task_sleep(50);
  rails_positive_on();
  task_sleep(50);

  ref_on();         // OK. this
  task_sleep(50);


  uart_printf("power_up done\n\r");
}

static void dac_test(void)
{


  /*
  34,the digital supplies (DVDD and IOVDD) and logic inputs (UNI/BIP-x) must be
  applied before AVSS and AVDD. Additionally, AVSS must be applied before AVDD
  unless both can ramp up at the same time. REF-x should be applied after AVDD
  comes up in order to make sure the ESD protection circuitry does not turn on.
  */


  uart_printf("dac writing dac registers\n\r");


  // dac_write_register(0x04, 51800 );  // Vout
  dac_write_register(0x04, 5180 * 2 );  // Vout - 2V.
  dac_write_register(0x05, 25900 );     // Iout -5V at ierr

  /*
    ok for v reference of 6.5536V
    rails need to be 6.5536 * 2 + 1 == 14.1V.
  */
#if 0
  dac_write_register1( 0b00000110 << 16 | 0x7f7f ); // write dac 2
  dac_write_register1( 0b00000111 << 16 | 0x7f7f ); // write dac 3
  task_sleep(1);  // must wait for update - before we read
#endif


  // select ain auxillary monitor.
  // 11 is ain. 13 is dac1.
  uart_printf("dac write mon register for ain\n\r");
  // dac_write_register1( 0b00000001 << 16 | (1 << 11) ); // select AIN.
  // dac_write_register1( 0b00000001 << 16 | (1 << 13) ); // select dac 1.

  // ok!
  dac_write_register(0x01, (1 << 13) ); // select monitor dac1

  uart_printf("dac test finished\n\r");
}

////////////////////////////////////


#define MUX_PORT GPIOE

// TODO, fix should prefix all thse with MUX_ in the schematic.
// maybe differentiate from bootstrap mux. also.
#define VSET_CTL      GPIO1
#define VSET_INV_CTL  GPIO2
#define ISET_CTL      GPIO3
#define ISET_INV_CTL  GPIO4


#define VFB_CTL       GPIO5
#define VFB_INV_CTL   GPIO6
#define IFB_CTL       GPIO7
#define IFB_INV_CTL   GPIO8

#define MUX_MIN_CTL   GPIO9
#define MUX_MAX_CTL   GPIO10

// DAC_REF65_CTL    11
#define MUX_MUX_UNUSED_CTL    GPIO12
// LN15V_LCT  13
// LP15V_LCT 14





static void mux_setup(void)
{

  uint32_t all =
    VSET_CTL | VSET_INV_CTL | ISET_CTL | ISET_INV_CTL
      | VFB_CTL | VFB_INV_CTL | IFB_CTL | IFB_INV_CTL
      | MUX_MIN_CTL | MUX_MAX_CTL | MUX_MUX_UNUSED_CTL;


  uart_printf("mux setup\n\r");
  // call *before* bringing up rails
  gpio_set(MUX_PORT, all);   // active low.
  gpio_mode_setup(MUX_PORT, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, all);
  uart_printf("mux setup done\n\r");
}


static void source_current_test_old(void)
{
  // U1
  uart_printf("mux test \n\r");

  gpio_clear(MUX_PORT, VSET_CTL);  // eg. +10V so Verr gets -10V. good for testing ifb
  // gpio_clear(MUX_PORT, VSET_INV_CTL); // so          Verr gets +10V.
  // gpio_clear(MUX_PORT, VFB_CTL);
  // gpio_clear(MUX_PORT, VFB_INV_CTL);



  // gpio_clear(MUX_PORT, ISET_CTL);     // eg. inject +5V, so verr gets -5V.
  gpio_clear(MUX_PORT, ISET_INV_CTL);     // eg. inject -5V, so verr gets +5V.
  // gpio_clear(MUX_PORT, IFB_CTL);          // fb
  gpio_clear(MUX_PORT, IFB_INV_CTL);          // fb

  /*
    - if an input is not turned on - then it gets 0V/AGND rather than high impedance which may
    which may be the min/max and is a bit confusing.
    - to test in isolation - we can always set the other value as -10V etc..
      we don't really want/ high-impedance - for a min/max function - as its a completely unrelated state
    - alternatively if we used a single op-amp and dg-444 for 4 diodes, then we could control all throughput.
      - no, because have to control the 10k bias resistors also.
  */

  // select max...
  // gpio_clear(MUX_PORT, MUX_MAX_CTL);
  gpio_clear(MUX_PORT, MUX_MIN_CTL);

  uart_printf("mux test finished\n\r");
}


static void source_voltage_test(void)
{
  // old before integrating error amp.
#if 0
  // set to source voltage / current compliance - first quadrant.
  // U1
  uart_printf("mux test new\n\r");


  dac_write_register(0x04, 5180 * 2 );  // Vset - 2V.
  dac_write_register(0x05, 25900 );     // Iset -5V at ierr


  // balance around 0V
  gpio_clear(MUX_PORT, VSET_INV_CTL); // so          Verr gets +1V.
  gpio_clear(MUX_PORT, VFB_CTL);

  gpio_clear(MUX_PORT, ISET_CTL);     // eg. inject +5V, so verr gets -5V. to turn off...

  // select max for sourcing...
  gpio_clear(MUX_PORT, MUX_MAX_CTL);

  uart_printf("mux test finished\n\r");
#endif
}


#define DAC_VSET_REG 0x04
#define DAC_ISET_REG 0x05

static void source_current_test(void)
{
  // updated - to new integrating error amp.
  // set to source current control / voltage compliance - .. quadrant.
  // ifb amp is 10x.  10mA == 100mV.

  // so there's no difference between control and compliance order.

  // U1
  uart_printf("mux test\n\r");

  // compliance function. max of 5V.
  dac_write_register(DAC_VSET_REG, 5180 * 5 );  // Vset 5v max compliance function
  gpio_clear(MUX_PORT, VSET_CTL);
  gpio_clear(MUX_PORT, VFB_INV_CTL);

  // control function 20mA
  dac_write_register(DAC_ISET_REG, 518 * 3 );   // Iset 3V == 30mA control function, 10x ifb gain.
  gpio_clear(MUX_PORT, ISET_CTL);
  gpio_clear(MUX_PORT, IFB_INV_CTL);

  // select max for sourcing...
  gpio_clear(MUX_PORT, MUX_MAX_CTL);

  uart_printf("mux test finished\n\r");
}



//////////////////////////////////////////


#define ADC_PORT              GPIOD

// TODO, fix should prefix all thse with MUX_ in the schematic.
// maybe differentiate from bootstrap mux. also.
#define ADC_OUT               GPIO0
#define ADC_REFP10V_CTL       GPIO1
#define ADC_REFN10V_CTL       GPIO2
#define ADC_IN_CTL            GPIO3
#define ADC_RESET_CTL         GPIO4
#define ADC_MUX_VFB_CTL       GPIO5
#define ADC_MUX_IFB_CTL       GPIO6
#define ADC_MUX_DAC_VMON_CTL  GPIO7
#define ADC_MUX_AGND_CTL      GPIO8



static  uint32_t all_ctl =
    ADC_REFP10V_CTL | ADC_REFN10V_CTL | ADC_IN_CTL | ADC_RESET_CTL
      | ADC_MUX_VFB_CTL | ADC_MUX_IFB_CTL | ADC_MUX_DAC_VMON_CTL | ADC_MUX_AGND_CTL;


static void adc_setup(void)
{
/*
  // active low.
  uint32_t all_ctl =
    ADC_REFP10V_CTL | ADC_REFN10V_CTL | ADC_IN_CTL | ADC_RESET_CTL
      | ADC_MUX_VFB_CTL | ADC_MUX_IFB_CTL | ADC_MUX_DAC_VMON_CTL | ADC_MUX_AGND_CTL;
*/

  uart_printf("adc setup\n\r");

  gpio_set(ADC_PORT, all_ctl);

  gpio_clear(ADC_PORT, ADC_RESET_CTL);    // turn on - to short cap, and hold integrator at agnd.
                                          // it's the diode clamp that appears to oscillate ...
  // small amount of hysteriss could be useful - but don't think is the problem - unless its coupling?

  // ok all of these inputs will oscillate... with the cap shorted/reset by switch...
  // so there's no point - adding a smaller offset...
  gpio_clear(ADC_PORT, ADC_IN_CTL | ADC_MUX_AGND_CTL);   // put agnd on the input
  // gpio_clear(ADC_PORT, ADC_REFP10V_CTL);  // put 10V on the input
  // gpio_clear(ADC_PORT, ADC_REFN10V_CTL);  // put -10V on the input

  gpio_mode_setup(ADC_PORT, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, all_ctl);
  gpio_mode_setup(ADC_PORT, GPIO_MODE_INPUT, GPIO_PUPD_NONE, ADC_OUT);

  uart_printf("adc setup done\n\r");
}



static void adc_out_print(void)
{
  static uint32_t tick = 0;

  uart_printf("hi tick %d %d\n\r",
      tick++,
      gpio_get(ADC_PORT, ADC_OUT)
    );
}


static void adc_test(void)
{
  // uart_printf("adc test disable\n\r");
  // return;
  uart_printf("adc test\n\r");

  // dac_write_register(0x01, (1 << 12) );  // select monitor dac0  // works
  // dac_write_register(0x01, (1 << 11) );     // ain, which is wired to vref65  // works
  // gpio_clear(ADC_PORT, ADC_MUX_DAC_VMON_CTL);

  // integrates up
  gpio_set(ADC_PORT, ADC_IN_CTL | ADC_MUX_AGND_CTL);      // turn off agnd in
  gpio_clear(ADC_PORT, ADC_REFN10V_CTL);                  // turn on N10V ref
  gpio_set(ADC_PORT, ADC_RESET_CTL);                    // start integrating
  task_sleep(3);

  // integrate down
  gpio_set(ADC_PORT, ADC_REFN10V_CTL);                  // turn off N10V ref
  gpio_clear(ADC_PORT, ADC_REFP10V_CTL);                  // turn on N10V ref
  task_sleep(3);

  // reset
  gpio_set(ADC_PORT, ADC_REFP10V_CTL);                  // turn off N10V ref
  gpio_clear(ADC_PORT, ADC_IN_CTL | ADC_MUX_AGND_CTL);  // turn on agnd in
  gpio_clear(ADC_PORT, ADC_RESET_CTL);                    // short the cap, to stop integrating

  uart_printf("adc test done\n\r");
}



// IFB looks ok. goes in the correct direction, and looks nice and linear.

static void adc_clear(void)
{
  gpio_set(ADC_PORT, all_ctl);
  gpio_clear(ADC_PORT, ADC_RESET_CTL);                    // short cap.  stop integrating
  gpio_clear(ADC_PORT, ADC_IN_CTL | ADC_MUX_AGND_CTL);   // put agnd on the input
}


static void adc_ifb_test(void)
{
  uart_printf("adc test\n\r");

  adc_clear();
  task_sleep(3);

#if 1
  // integrate up
  gpio_set(ADC_PORT, ADC_IN_CTL | ADC_MUX_AGND_CTL);    // turn off agnd in
  gpio_clear(ADC_PORT, ADC_IN_CTL | ADC_MUX_IFB_CTL);    // turn on IFB ref
  gpio_set(ADC_PORT, ADC_RESET_CTL);                    // clear the short of the cap - start integrating
  task_sleep(5);

  // integrate down
  gpio_set(ADC_PORT, ADC_IN_CTL | ADC_MUX_IFB_CTL);    // turn off IFB ref
  gpio_clear(ADC_PORT, ADC_REFN10V_CTL);                  // turn on P10V ref
  task_sleep(3);

  // reset
  adc_clear();
#endif
  uart_printf("adc test done\n\r");
}




// VFB looks ok. it's hard to trigger on.
// but its a bit curved rather than linear - because not buffered.


static void adc_vfb_test(void)
{
  uart_printf("adc test\n\r");

  // VFB is 2.5V on the input. no problem. but not buffered...
  // gpio_set(ADC_PORT, ADC_IN_CTL | ADC_MUX_AGND_CTL);    // turn off agnd in
  // gpio_clear(ADC_PORT, ADC_IN_CTL | ADC_MUX_VFB_CTL);    // turn on VFB ref

  // IFB is 0.3VDC - no problem.
  // gpio_set(ADC_PORT, ADC_IN_CTL | ADC_MUX_AGND_CTL);    // turn off agnd in
  // gpio_clear(ADC_PORT, ADC_IN_CTL | ADC_MUX_IFB_CTL);    // turn on VFB ref

  adc_clear();
  task_sleep(3);

  // integrate down
  gpio_set(ADC_PORT, ADC_IN_CTL | ADC_MUX_AGND_CTL);    // turn off agnd in
  gpio_clear(ADC_PORT, ADC_IN_CTL | ADC_MUX_VFB_CTL);    // turn on VFB ref
  gpio_set(ADC_PORT, ADC_RESET_CTL);                    // clear the short of the cap - start integrating
  task_sleep(3);

  // integrate up
  gpio_set(ADC_PORT, ADC_IN_CTL | ADC_MUX_VFB_CTL);    // turn off VFB ref
  gpio_clear(ADC_PORT, ADC_REFN10V_CTL);                  // turn on P10V ref
  task_sleep(3);

  // reset
  adc_clear();

  uart_printf("adc test done\n\r");
}




static void regulate_on_vfb(void)
{

  uart_printf("regulate on vfb\n\r");

  /*
    bypass
    regulates on vfb regardless of dac or other muxes
    Note - if we buffer vfb with inverting op-amp this won't work.
  */
  // gpio_clear(MUX_PORT, MUX_MUX_UNUSED_CTL);   // set active low. regulate on VFB. non inverted

  gpio_clear(MUX_PORT, VFB_INV_CTL);

  gpio_clear(MUX_PORT, MUX_MAX_CTL);

  uart_printf("regulate on vfb done\n\r");
}



static void test01(void *args __attribute((unused)))
{
  power_up();

  regulate_on_vfb();

#if 0
  // dac_test();
  source_current_test();
  // adc_test();
  // adc_ifb_test();
  adc_vfb_test();
#endif


  // sleep forever
  for(;;) {
    task_sleep(1000);
  }
}



/////////////////////////////
// this code should be where?

static void mcu_adc_setup(void)
{
	gpio_mode_setup(GPIOA, GPIO_MODE_ANALOG, GPIO_PUPD_NONE, GPIO0);
	gpio_mode_setup(GPIOA, GPIO_MODE_ANALOG, GPIO_PUPD_NONE, GPIO1);

	adc_power_off(ADC1);
	adc_disable_scan_mode(ADC1);
	adc_set_sample_time_on_all_channels(ADC1, ADC_SMPR_SMP_3CYC); // is this enough for all bits?

	adc_power_on(ADC1);

}


static uint16_t mcu_adc_read_native(uint8_t channel)
{
  // set up the arry of channels to read.
	uint8_t channel_array[16];
	channel_array[0] = channel;
	adc_set_regular_sequence(ADC1, 1, channel_array); // 1 indicates number of channels to read. eg. 1

  // start the read
	adc_start_conversion_regular(ADC1);
	while (!adc_eoc(ADC1));
	uint16_t reg16 = adc_read_regular(ADC1);
	return reg16;
}



static void relay_setup()
{
  gpio_mode_setup(GPIOD, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, GPIO9);
  gpio_clear(GPIOD, GPIO9);   // off.
}


static void relay_toggle_task(void *args __attribute((unused))) {

	for (;;) {
		vTaskDelay(pdMS_TO_TICKS(5 * 1000)); // 1Hz
    gpio_toggle(GPIOD, GPIO9);
	}
}

/////////////////////////////

int main(void) {

  // main clocks
  // disable HSE
  // must edit, configCPU_CLOCK_HZ also
  // clocks
  // rcc_clock_setup_pll(&rcc_hse_8mhz_3v3[RCC_CLOCK_3V3_168MHZ]);

  // led
  rcc_periph_clock_enable(RCC_GPIOA);

  // usart
  rcc_periph_clock_enable(RCC_GPIOA);
  rcc_periph_clock_enable(RCC_USART1);


  // relay
  rcc_periph_clock_enable(RCC_GPIOD);




  ///////////////
  // setup
  led_setup();
  usart_setup();
  uart_printf("------------------\n\r");
  uart_printf("starting\n\r");


  relay_setup();

  ///////////////
  // setup

	xTaskCreate(led_blink_task2,  "LED",100,NULL,configMAX_PRIORITIES-1,NULL);


  // IMPORTANT changing from 100 to 200, stops deadlock
  xTaskCreate(uart_task,        "UART",200,NULL,configMAX_PRIORITIES-1,NULL); /* Highest priority */
  xTaskCreate(usart_prompt_task,"PROMPT",200,NULL,configMAX_PRIORITIES-2,NULL); /* Lower priority */



	xTaskCreate(relay_toggle_task,  "LED",100,NULL,configMAX_PRIORITIES-1,NULL);



	vTaskStartScheduler();

  // should never get here?
	for (;;);
}






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

  // spi1
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
  adc_setup();

  uart_printf("------------------\n\r");

  ///////////////
  // tasks
  // value is the stackdepth.
	xTaskCreate(led_blink_task2,  "LED",100,NULL,configMAX_PRIORITIES-1,NULL);
  xTaskCreate(uart_task,        "UART",200,NULL,configMAX_PRIORITIES-1,NULL); /* Highest priority */

  // IMPORTANT changing from 100 to 200, stops deadlock
  xTaskCreate(usart_prompt_task,"PROMPT",200,NULL,configMAX_PRIORITIES-2,NULL); /* Lower priority */

  xTaskCreate(test01,        "DAC_TEST",200,NULL,configMAX_PRIORITIES-2,NULL); // Lower priority

	vTaskStartScheduler();

  // should never get here?
	for (;;);
	return 0;
}


