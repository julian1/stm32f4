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


// #include <libopencm3/cm3/nvic.h>
#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>
// #include <libopencm3/stm32/timer.h>
// #include <libopencm3/stm32/spi.h>
// #include <libopencm3/stm32/gpio.h>
// #include <libopencm3/stm32/usart.h>
// #include <libopencm3/cm3/nvic.h>
// #include <libopencm3/stm32/adc.h>


#include "sleep.h"
#include "usart.h"
#include "led.h"
#include "rails.h"
#include "ref.h"
#include "dac8734.h"
#include "mcu_adc.h"




static void led_blink_task2(void *args __attribute((unused)))
{
  // should put this in led.c?
  // already there....

	for (;;) {

    // led_toggle
		led_toggle();


#if 0
    static int tick = 0;

    uart_printf("hi %d %d %d\n\r",
      tick++,
      gpio_get(DAC_PORT, DAC_GPIO0),
      gpio_get(DAC_PORT, DAC_GPIO1 )
    );
#endif

    task_sleep(500);
  }
}





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






////////////////////////////



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

  // rails
  rcc_periph_clock_enable(RCC_GPIOE);

  // mcu adc1
	rcc_periph_clock_enable(RCC_ADC1);




  ///////////////
  // setup
  led_setup();
  usart_setup();
  uart_printf("------------------\n\r");
  uart_printf("starting\n\r");


  relay_setup();
  rails_setup();
  mcu_adc_setup();




  ///////////////
  // setup

	xTaskCreate(led_blink_task2,  "LED",100,NULL,configMAX_PRIORITIES-1,NULL);


  // IMPORTANT changing from 100 to 200, stops deadlock
  xTaskCreate(uart_task,        "UART",200,NULL,configMAX_PRIORITIES-1,NULL); /* Highest priority */
  xTaskCreate(usart_prompt_task,"PROMPT",200,NULL,configMAX_PRIORITIES-2,NULL); /* Lower priority */



  xTaskCreate(mcu_adc_print_task,"MCU_ADC",200,NULL,configMAX_PRIORITIES-2,NULL); /* Lower priority */


	//xTaskCreate(relay_toggle_task,  "LED",100,NULL,configMAX_PRIORITIES-1,NULL);

  // rails_positive_on();
  // rails_negative_on();


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


