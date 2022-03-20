/*
  nix-shell ~/devel/nixos-config/examples/arm.nix
  make

  serial,
  rlwrap -a picocom -b 115200 /dev/ttyUSB0

  usb
  screen /dev/ttyACM0 115200

  screen
  openocd -f openocd.cfg
  rlwrap nc localhost 4444  # in new window

   reset halt ; flash write_image erase unlock ../blinky-stm32f410cbt3/main.elf; sleep 1500; reset run

  *********
  with gnu sprintf, and floating point code, this still fits in 27k, tested by editing f410.ld.  good!!
  *********
  -------------------

  https://titanwolf.org/Network/Articles/Article?AID=198f4410-66a4-4bee-a263-bfbb244dbc45
*/


#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>

// #include <libopencm3/cm3/nvic.h>
#include <libopencm3/cm3/systick.h>



#include <libopencm3/usb/usbd.h>


#include <stddef.h> // size_t
//#include <math.h> // nanf
//#include <stdio.h>
#include <string.h>   // memset


#include "cbuffer.h"
#include "usart2.h"
#include "util.h"
#include "assert.h"
#include "cdcacm.h"




typedef struct app_t
{
  CBuf console_in;
  CBuf console_out;

  usbd_device *usbd_dev ;

} app_t;


static void update_console_cmd(app_t *app)
{

  if( !cBufisEmpty(&app->console_in) && cBufPeekLast(&app->console_in) == '\r') {

    // usart1_printf("got CR\n");

    // we got a carriage return
    static char tmp[1000];

    size_t nn = cBufCount(&app->console_in);
    size_t n = cBufCopyString(&app->console_in, tmp, ARRAY_SIZE(tmp));
    ASSERT(n <= sizeof(tmp));
    ASSERT(tmp[n - 1] == 0);
    ASSERT( nn == n - 1);

    // chop off the CR to make easier to print
    ASSERT(((int) n) - 2 >= 0);
    tmp[n - 2] = 0;

    // TODO first char 'g' gets omitted/chopped here, why? CR handling?
    usart1_printf("got command '%s'\n", tmp);

    // process_cmd(app, tmp);
  }
}




static void loop(app_t *app)
{
  /*
    loop() subsumes update()
  */

  // TODO move to app_t structure?.
  static uint32_t soft_500ms = 0;

  while(true) {


		usbd_poll(app->usbd_dev);

    update_console_cmd(app);

    // usart1_enable_output_interupt(); // shouldn't be necessary


    // 500ms soft timer. should handle wrap around
    if( (system_millis - soft_500ms) > 500) {
      soft_500ms += 500;
      led_toggle();
      usart1_printf("here\n");
    }

  }
}




static char buf_console_in[1000];
static char buf_console_out[1000];


static app_t app;






#include <libopencm3/stm32/fsmc.h>



static void fsmc_setup(void)
{

  /*
    https://titanwolf.org/Network/Articles/Article?AID=198f4410-66a4-4bee-a263-bfbb244dbc45
  */

 /* Enable PORTD and PORTE */
  rcc_periph_clock_enable(RCC_GPIOD);

  rcc_periph_clock_enable(RCC_GPIOE);


 /* Enable FSMC */
  rcc_periph_clock_enable(RCC_FSMC);


 /* config FSMC data lines */
#if 0
  uint16_t portd_gpios = GPIO0 | GPIO1 | GPIO8 | GPIO9 | GPIO10 | GPIO14 | GPIO15;

  gpio_set_mode(GPIOD, GPIO_MODE_OUTPUT_2_MHZ, GPIO_CNF_OUTPUT_ALTFN_PUSHPULL, portd_gpios);


  uint16_t porte_gpios = GPIO7 | GPIO8 | GPIO9 | GPIO10 | GPIO11 | GPIO12 | GPIO13 | GPIO14 | GPIO15;

  gpio_set_mode(GPIOE, GPIO_MODE_OUTPUT_2_MHZ, GPIO_CNF_OUTPUT_ALTFN_PUSHPULL, porte_gpios);


 /* config FSMC NOE */
  gpio_set_mode(GPIOD, GPIO_MODE_OUTPUT_2_MHZ, GPIO_CNF_OUTPUT_ALTFN_PUSHPULL, GPIO4);


 /* config FSMC NWE */
  gpio_set_mode(GPIOD, GPIO_MODE_OUTPUT_2_MHZ, GPIO_CNF_OUTPUT_ALTFN_PUSHPULL, GPIO5);


 /* config FSMC NE1 */
  gpio_set_mode(GPIOD, GPIO_MODE_OUTPUT_2_MHZ, GPIO_CNF_OUTPUT_ALTFN_PUSHPULL, GPIO7);


 /* config FSMC A16 for D/C (select Data/Command ) */
  gpio_set_mode(GPIOD, GPIO_MODE_OUTPUT_2_MHZ, GPIO_CNF_OUTPUT_ALTFN_PUSHPULL, GPIO11);
#endif


 /* config FSMC register */
  FSMC_BTR(0) = FSMC_BTR_ACCMODx(FSMC_BTx_ACCMOD_B) |
                FSMC_BTR_DATLATx(0)  |
                FSMC_BTR_CLKDIVx(0)  |
                FSMC_BTR_BUSTURNx(0) |
                FSMC_BTR_DATASTx(5)  |
                FSMC_BTR_ADDHLDx(0)  |
                FSMC_BTR_ADDSETx(1);


  FSMC_BCR(0) = FSMC_BCR_WREN | FSMC_BCR_MWID | FSMC_BCR_MBKEN;

}

// JA
#define __IO volatile

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


static uint16_t LCD_ReadRAM(void)
 {

  /* Write 16-bit Index (then Read Reg) */

  // LCD->LCD_REG = R34 /* Select GRAM Reg */

  // JA
  LCD->LCD_REG = 0; /* Select GRAM Reg */

  /* Read 16-bit Reg */

  return LCD->LCD_RAM;
 }





int main(void)
{

  // required for usb
	rcc_clock_setup_pll(&rcc_hse_8mhz_3v3[RCC_CLOCK_3V3_84MHZ] );  // stm32f411  upto 100MHz.

  /*
  // http://libopencm3.org/docs/latest/stm32f4/html/f4_2rcc_8h.html

    see here for rcc.c example, defining 100MHz, using 25MHz xtal.
      https://github.com/insane-adding-machines/unicore-mx/blob/master/lib/stm32/f4/rcc.c
  */

  // clocks
  rcc_periph_clock_enable(RCC_SYSCFG); // maybe required for external interupts?

  // LED
  rcc_periph_clock_enable(RCC_GPIOA); // f410/f411 led.

  // USART
  // rcc_periph_clock_enable(RCC_GPIOA);     // f407
  rcc_periph_clock_enable(RCC_GPIOB); // F410/f411
  rcc_periph_clock_enable(RCC_USART1);


  // USB
	// rcc_periph_clock_enable(RCC_GPIOA);
	rcc_periph_clock_enable(RCC_OTGFS);

  // spi / ice40
  // rcc_periph_clock_enable(RCC_SPI1);

  //////////////////////
  // setup

/*
  // 16MHz. from hsi datasheet.
  systick_setup(16000);
*/
  // 84MHz.
  systick_setup(84000);


  // led
  led_setup();


  memset(&app, 0, sizeof(app_t));

  // uart/console
  cBufInit(&app.console_in,  buf_console_in, sizeof(buf_console_in));
  cBufInit(&app.console_out, buf_console_out, sizeof(buf_console_out));

  // usart1_setup_gpio_portA();
  usart1_setup_gpio_portB();

  usart1_set_buffers(&app.console_in, &app.console_out);

  // setup print
  // usart1_printf_set_buffer()
  usart1_printf_init(&app.console_out);


  ////////////////////////////
  // usb
  // might be better to pass as handler?
	app.usbd_dev = usb_setup();
  ASSERT(app.usbd_dev);



  usart1_printf("\n--------\n");
  usart1_printf("starting loop\n");
  usart1_printf("sizeof bool   %u\n", sizeof(bool));
  usart1_printf("sizeof float  %u\n", sizeof(float));
  usart1_printf("sizeof double %u\n", sizeof(double));
  // test assert failure
  ASSERT(1 == 2);

  usart1_printf("a float formatted %g\n", 123.456f );


  loop(&app);
}


