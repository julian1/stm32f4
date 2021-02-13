

DEPRECATED!

// #include "FreeRTOS.h"
// #include "task.h"

#include <libopencm3/stm32/gpio.h>


//////////////////////////////////////////


#include "sleep.h"
#include "usart.h"
#include "mux.h"
#include "dac8734.h"  // dac_write_register




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





void mux_setup(void)
{

  uint32_t all =
    VSET_CTL | VSET_INV_CTL | ISET_CTL | ISET_INV_CTL
      | VFB_CTL | VFB_INV_CTL | IFB_CTL | IFB_INV_CTL
      | MUX_MIN_CTL | MUX_MAX_CTL | MUX_MUX_UNUSED_CTL;


  usart_printf("mux setup\n\r");
  // call *before* bringing up rails
  gpio_set(MUX_PORT, all);   // active low.
  gpio_mode_setup(MUX_PORT, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, all);
  usart_printf("mux setup done\n\r");
}


static void source_current_test_old(void)
{
  // U1
  usart_printf("mux test \n\r");

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

  usart_printf("mux test finished\n\r");
}


static void source_voltage_test(void)
{
  // old before integrating error amp.
#if 0
  // set to source voltage / current compliance - first quadrant.
  // U1
  usart_printf("mux test new\n\r");


  dac_write_register(0x04, 5180 * 2 );  // Vset - 2V.
  dac_write_register(0x05, 25900 );     // Iset -5V at ierr


  // balance around 0V
  gpio_clear(MUX_PORT, VSET_INV_CTL); // so          Verr gets +1V.
  gpio_clear(MUX_PORT, VFB_CTL);

  gpio_clear(MUX_PORT, ISET_CTL);     // eg. inject +5V, so verr gets -5V. to turn off...

  // select max for sourcing...
  gpio_clear(MUX_PORT, MUX_MAX_CTL);

  usart_printf("mux test finished\n\r");
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
  usart_printf("mux test\n\r");

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

  usart_printf("mux test finished\n\r");
}





void mux_regulate_on_vfb(void)
{

  usart_printf("regulate on vfb\n\r");

  /*
    bypass
    regulates on vfb regardless of dac or other muxes
    Note - if we buffer vfb with inverting op-amp this won't work.
  */
  // gpio_clear(MUX_PORT, MUX_MUX_UNUSED_CTL);   // set active low. regulate on VFB. non inverted

  gpio_clear(MUX_PORT, VFB_INV_CTL);

  gpio_clear(MUX_PORT, MUX_MAX_CTL);

  usart_printf("regulate on vfb done\n\r");
}


#if 0
// should be moved to dac.
// should be digital...
// actually

static void dac_test(void)
{
  // this is a higher-level function... relying on dac_write_register()
  // so don't place in dac8734.c
  // actually belongs in mux...
  // as part of mux testing

  /*
  34,the digital supplies (DVDD and IOVDD) and logic inputs (UNI/BIP-x) must be
  applied before AVSS and AVDD. Additionally, AVSS must be applied before AVDD
  unless both can ramp up at the same time. REF-x should be applied after AVDD
  comes up in order to make sure the ESD protection circuitry does not turn on.
  */


  usart_printf("dac writing dac registers\n\r");


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
  usart_printf("dac write mon register for ain\n\r");
  // dac_write_register1( 0b00000001 << 16 | (1 << 11) ); // select AIN.
  // dac_write_register1( 0b00000001 << 16 | (1 << 13) ); // select dac 1.

  // ok!
  dac_write_register(0x01, (1 << 13) ); // select monitor dac1

  usart_printf("dac test finished\n\r");
}

#endif
