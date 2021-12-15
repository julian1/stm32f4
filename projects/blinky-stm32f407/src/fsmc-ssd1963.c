





//#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>


#include <libopencm3/stm32/fsmc.h>



#include "fsmc-ssd1963.h"


// #include "usart2.h"
// #include "cbuffer.h"

#include "util.h"   // printf, msleep








void fsmc_gpio_setup()
{
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

  uint16_t portd_gpios = GPIO0 | GPIO1 | GPIO8 | GPIO9 | GPIO10 | GPIO14 | GPIO15;
  // gpio_set_mode(GPIOD, GPIO_MODE_OUTPUT_2_MHZ, GPIO_CNF_OUTPUT_ALTFN_PUSHPULL, portd_gpios);
  gpio_mode_setup(GPIOD, GPIO_MODE_AF, GPIO_PUPD_NONE, portd_gpios);
  gpio_set_output_options(GPIOD, GPIO_OTYPE_PP, speed, portd_gpios);
  gpio_set_af(GPIOD, GPIO_AF12, portd_gpios);



  uint16_t porte_gpios = GPIO7 | GPIO8 | GPIO9 | GPIO10 | GPIO11 | GPIO12 | GPIO13 | GPIO14 | GPIO15;
  // gpio_set_mode(GPIOE, GPIO_MODE_OUTPUT_2_MHZ, GPIO_CNF_OUTPUT_ALTFN_PUSHPULL, porte_gpios);
  gpio_mode_setup(GPIOE, GPIO_MODE_AF, GPIO_PUPD_NONE, porte_gpios);
  gpio_set_output_options(GPIOE, GPIO_OTYPE_PP, speed, porte_gpios);
  gpio_set_af(GPIOE, GPIO_AF12, porte_gpios);


  // these could be consolidated...

 /* config FSMC NOE */
  // gpio_set_mode(GPIOD, GPIO_MODE_OUTPUT_2_MHZ, GPIO_CNF_OUTPUT_ALTFN_PUSHPULL, GPIO4);
  gpio_mode_setup(GPIOD, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO4);
  gpio_set_output_options(GPIOD, GPIO_OTYPE_PP, speed, GPIO4);
  gpio_set_af(GPIOD, GPIO_AF12, GPIO4);


 /* config FSMC NWE */
  // gpio_set_mode(GPIOD, GPIO_MODE_OUTPUT_2_MHZ, GPIO_CNF_OUTPUT_ALTFN_PUSHPULL, GPIO5);
  gpio_mode_setup(GPIOD, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO5);
  gpio_set_output_options(GPIOD, GPIO_OTYPE_PP, speed, GPIO5);
  gpio_set_af(GPIOD, GPIO_AF12, GPIO5);


 /* config FSMC NE1 */
  /* JA PD7 CS */
  // gpio_set_mode(GPIOD, GPIO_MODE_OUTPUT_2_MHZ, GPIO_CNF_OUTPUT_ALTFN_PUSHPULL, GPIO7);
  gpio_mode_setup(GPIOD, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO7);
  gpio_set_output_options(GPIOD, GPIO_OTYPE_PP, speed, GPIO7);
  gpio_set_af(GPIOD, GPIO_AF12, GPIO7);



 /* config FSMC A16 for D/C (select Data/Command ) */
  /* JA PD11  RS */
  // gpio_set_mode(GPIOD, GPIO_MODE_OUTPUT_2_MHZ, GPIO_CNF_OUTPUT_ALTFN_PUSHPULL, GPIO11);
  gpio_mode_setup(GPIOD, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO11);
  gpio_set_output_options(GPIOD, GPIO_OTYPE_PP, speed, GPIO11);
  gpio_set_af(GPIOD, GPIO_AF12, GPIO11);

  /////////////////



  #define TFT_GPIO_PORT       GPIOE
  #define TFT_LED_A           GPIO2
  #define TFT_REST            GPIO1
  // TFT_T_IRQ

  gpio_mode_setup(TFT_GPIO_PORT, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, TFT_LED_A | TFT_REST);
  // speed.


}





void tft_reset(void )
{

  usart_printf("pull reset lo\n");
  // reset. pull lo then high.
  gpio_clear( TFT_GPIO_PORT, TFT_REST);
  msleep(20);
  usart_printf("pull reset hi\n");
  gpio_set( TFT_GPIO_PORT, TFT_REST);
  msleep(20);


  gpio_set( TFT_GPIO_PORT, TFT_LED_A ); // turn on backlight. works!!!
}



void fsmc_setup(uint8_t divider)
{

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
                FSMC_BTR_DATASTx(5 * divider)  |
                FSMC_BTR_ADDHLDx(0)  |
                FSMC_BTR_ADDSETx(1 * divider);


  FSMC_BCR(0) = FSMC_BCR_WREN | FSMC_BCR_MWID | FSMC_BCR_MBKEN;

}

// JA
// #define __IO volatile
#define __IO 

/*
  __IO flag appears undefined.
*/
 typedef struct
 {

  __IO uint16_t LCD_REG;

  __IO uint16_t LCD_RAM;
 } LCD_TypeDef;

// #define LCD_BASE    ((uint32_t)(0x60000000 | 0x00020000 -2 ) )
// JA
#define LCD_BASE    ((uint32_t)(0x60000000 | (0x00020000 -2) ) )

/* JA
  >  (0x60000000 | (0x00020000 -2)).toString(16)
  "6001fffe"
  > (0x60000000 | (0x00020000 )).toString(16)
  "60020000"
*/

/*
When you access A16 becomes zero in the LCD-> LCD_REG. (Address 0x6001FFFE)
When you access A16 is 1 in LCD-> LCD_RAM. (Address 0x60020000)
*/

#define LCD         ((LCD_TypeDef *) LCD_BASE)




#if 1

void LCD_SetAddr(uint8_t LCD_Reg)
{
  /* Write 16-bit Index (then Read Reg) */
  LCD->LCD_REG = LCD_Reg;
}
#endif




//////////
/* 
  TODO.
  should be static incline...
  change later. 
  ----------

  it appears register writing isn't working.
  but the write command is 
  are we sure the registers.

*/



void LCD_WriteCommand(uint16_t cmd) {
  /* Write cmd */
  // LCD_REG = cmd;
  LCD->LCD_REG = cmd;
}

void LCD_WriteData(uint16_t data) {
  /* Write 16-bit data */
  // LCD_RAM = data;
  LCD->LCD_RAM = data;
}

uint16_t LCD_ReadData(void) {
  /* Read 16-bit data */
  // return LCD_RAM;
  return (LCD->LCD_RAM);
}




//////////////////


#if 0

uint16_t LCD_ReadReg(uint8_t LCD_Reg)
 {

  /* Write 16-bit Index (then Read Reg) */
  LCD->LCD_REG = LCD_Reg;

  msleep(1);

  /* Read 16-bit Reg */

  return (LCD->LCD_RAM);
 }


void LCD_WriteReg(uint8_t LCD_Reg, uint16_t LCD_RegValue)
 {

  /* Write 16-bit Index, then Write Reg */
  LCD->LCD_REG = LCD_Reg;

  /* Write 16-bit Reg */
  LCD->LCD_RAM = LCD_RegValue;
 }

#endif
