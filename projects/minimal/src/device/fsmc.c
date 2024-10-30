


#include <stdio.h>



//#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/fsmc.h>


#include <device/fsmc.h>

// #include "util.h"      // msleep
// #include "streams.h"  // printf


#if 0
#define TFT_GPIO_PORT       GPIOE
#define TFT_REST            GPIO1
#define TFT_LED_A           GPIO2   // unused, due to jumper.

#endif

/*
  pin only shows a 1.5V signal...

  TE  p50 ssd1963 ref voltage is VDDLCD
  LCD interface supply power (VDDLCD): 1.65V to 3.6V
  ----
  Issue is that pin was bridged.
  Also old bodge was tied to interupt of xt2046.
*/

/*
// Mapped to spi2 nss2 unused. using bodge wire.  ssd1963 TE tear interupt pin.
#define TEAR_PORT           GPIOB
#define TEAR_IRQ            GPIO9
*/

#if 0
// control-panel-2 / spectra184
#define TEAR_PORT           GPIOE
#define TEAR_IRQ            GPIO3

#endif





void fsmc_gpio_setup()
{

  printf("fsmc_gpio_setup\n");
/*
  // Do pin setup separately from the fsmc peripheral setup. because we will call fsmc setup
  twice once for slow/hi speed operation.

  // Enable PORTD and PORTE
  rcc_periph_clock_enable(RCC_GPIOD);
  rcc_periph_clock_enable(RCC_GPIOE);

  // Enable FSMC
  rcc_periph_clock_enable(RCC_FSMC);

*/

  // uint8_t speed = GPIO_OSPEED_25MHZ;
  uint8_t speed = GPIO_OSPEED_100MHZ;

 /* config FSMC data lines */

  ///////////////
  // data lines.


  uint16_t portd_data_lines = GPIO0 | GPIO1 | GPIO8 | GPIO9 | GPIO10 | GPIO14 | GPIO15;
  // gpio_set_mode(GPIOD, GPIO_MODE_OUTPUT_2_MHZ, GPIO_CNF_OUTPUT_ALTFN_PUSHPULL, portd_data_lines);
  gpio_mode_setup(GPIOD, GPIO_MODE_AF, GPIO_PUPD_NONE, portd_data_lines);
  gpio_set_output_options(GPIOD, GPIO_OTYPE_PP, speed, portd_data_lines);
  gpio_set_af(GPIOD, GPIO_AF12, portd_data_lines);



  uint16_t porte_data_lines = GPIO7 | GPIO8 | GPIO9 | GPIO10 | GPIO11 | GPIO12 | GPIO13 | GPIO14 | GPIO15;
  // gpio_set_mode(GPIOE, GPIO_MODE_OUTPUT_2_MHZ, GPIO_CNF_OUTPUT_ALTFN_PUSHPULL, porte_data_lines);
  gpio_mode_setup(GPIOE, GPIO_MODE_AF, GPIO_PUPD_NONE, porte_data_lines);
  gpio_set_output_options(GPIOE, GPIO_OTYPE_PP, speed, porte_data_lines);
  gpio_set_af(GPIOE, GPIO_AF12, porte_data_lines);


  // could/can consolidate...

 /* config FSMC NOE -  PD4  RD */
  gpio_mode_setup(GPIOD, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO4);
  gpio_set_output_options(GPIOD, GPIO_OTYPE_PP, speed, GPIO4);
  gpio_set_af(GPIOD, GPIO_AF12, GPIO4);


 /* config FSMC NWE  -  PD5  WR */
  gpio_mode_setup(GPIOD, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO5);
  gpio_set_output_options(GPIOD, GPIO_OTYPE_PP, speed, GPIO5);
  gpio_set_af(GPIOD, GPIO_AF12, GPIO5);


 /* config FSMC NE1 -  PD7   CS */
  gpio_mode_setup(GPIOD, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO7);
  gpio_set_output_options(GPIOD, GPIO_OTYPE_PP, speed, GPIO7);
  gpio_set_af(GPIOD, GPIO_AF12, GPIO7);

  ///////////////
  // address lines.


 /* config FSMC A16 for D/C (select Data/Command ) */
  /* JA PD11  RS */
  // gpio_set_mode(GPIOD, GPIO_MODE_OUTPUT_2_MHZ, GPIO_CNF_OUTPUT_ALTFN_PUSHPULL, GPIO11);

  // may 2024.
  // fsmc a16,17,18
  uint16_t portd_address_lines = GPIO11 | GPIO12 | GPIO13;
  gpio_mode_setup(GPIOD, GPIO_MODE_AF, GPIO_PUPD_NONE, portd_address_lines);
  gpio_set_output_options(GPIOD, GPIO_OTYPE_PP, speed, portd_address_lines);
  gpio_set_af(GPIOD, GPIO_AF12, portd_address_lines);

  // fsmc a19,20,21,22
  uint16_t porte_address_lines = GPIO3 | GPIO4 | GPIO5 | GPIO6;
  gpio_mode_setup(GPIOE, GPIO_MODE_AF, GPIO_PUPD_NONE, porte_address_lines);
  gpio_set_output_options(GPIOE, GPIO_OTYPE_PP, speed, porte_address_lines);
  gpio_set_af(GPIOE, GPIO_AF12, porte_address_lines);





}




#if 0
  // led is required....
  gpio_mode_setup(TFT_GPIO_PORT, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, TFT_LED_A |  TFT_REST);

  // ssd1963 tear irq. bodge wire.
  gpio_mode_setup(TEAR_PORT, GPIO_MODE_INPUT, GPIO_PUPD_NONE, TEAR_IRQ);
#endif



#if 0

bool getTear()
{
  // TODO better prefix name. tft_get_tear() ?
  // hi tft stopped, means we should draw .
  // return gpio_get(TFT_GPIO_PORT, TFT_T_IRQ) & (0x01 << 3);
  return gpio_get(TEAR_PORT, TEAR_IRQ) != 0 ;
}



void tft_reset(void )
{

  usart1_printf("pull reset lo\n");
  // reset. pull lo then high.
  gpio_clear( TFT_GPIO_PORT, TFT_REST);
  msleep(20);
  usart1_printf("pull reset hi\n");
  gpio_set( TFT_GPIO_PORT, TFT_REST);
  msleep(20);

  // backlight must be on, to see anything.
  gpio_set( TFT_GPIO_PORT, TFT_LED_A ); // turn on backlight. works!!!
}

#endif


void fsmc_setup(uint8_t divider)
{

  printf("fsmc_ port setup \n");

  /*
    https://titanwolf.org/Network/Articles/Article?AID=198f4410-66a4-4bee-a263-bfbb244dbc45

    https://github.com/stm32f4/library/blob/master/SSD1963/GLCD.c

    https://community.st.com/s/question/0D50X00009XkgSPSAZ/stm32f4-discovery-ssd1963-fsmc

  FSMC_NORSRAMTimingInitStructureRead.FSMC_DataSetupTime = 5 * divider;
  FSMC_NORSRAMTimingInitStructureWrite.FSMC_DataSetupTime = 1 * divider;
  */


 /* config FSMC register */
  FSMC_BTR(0) = FSMC_BTR_ACCMODx(FSMC_BTx_ACCMOD_B) |
                FSMC_BTR_DATLATx(0)  |
                FSMC_BTR_CLKDIVx(0)  | // note. 0 not divider
                FSMC_BTR_BUSTURNx(0) |
                // FSMC_BTR_DATASTx(5 * divider)  |
                FSMC_BTR_DATASTx(1 * divider)  |   // JA seems ok. f4. 168MHz.
                FSMC_BTR_ADDHLDx(0)  |
                FSMC_BTR_ADDSETx(1 * divider);


  FSMC_BCR(0) = FSMC_BCR_WREN | FSMC_BCR_MWID | FSMC_BCR_MBKEN;

}



