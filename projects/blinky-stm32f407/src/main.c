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


#include "str.h"  //format_bits


typedef struct app_t
{
  CBuf console_in;
  CBuf console_out;

  usbd_device *usbd_dev ;

} app_t;


static void update_console_cmd(app_t *app)
{

  if( !cBufisEmpty(&app->console_in) && cBufPeekLast(&app->console_in) == '\r') {

    // usart_printf("got CR\n");

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
    usart_printf("got command '%s'\n", tmp);

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

    // usart_output_update(); // shouldn't be necessary


    // 500ms soft timer. should handle wrap around
    if( (system_millis - soft_500ms) > 500) {
      soft_500ms += 500;
      led_toggle();
      // usart_printf("here\n");
    }

  }
}




static char buf_console_in[1000];
static char buf_console_out[1000];


static app_t app;



static void tft_gpio_init(void)
{
  /*
    OK. first goal should be to read and write a register using fsmc and bus .
    probably want the reset also.

    see. kicad5/projects/control-panel-2/notes.tx
  /projects/fsmc-tests/doc/andy.txt
  */

  // clocks are external
  // SHOULD PUT ALL TFT stuff in header... or at least predeclare.
  // parallel tft / ssd1963
  // rcc_periph_clock_enable(RCC_GPIOD);
  // rcc_periph_clock_enable(RCC_GPIOE);


  #define TFT_GPIO_PORT       GPIOE
  #define TFT_LED_A           GPIO2
  #define TFT_REST            GPIO1

  gpio_mode_setup(TFT_GPIO_PORT, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, TFT_LED_A | TFT_REST);
  gpio_set( TFT_GPIO_PORT, TFT_LED_A ); // turn on backlight. works!!!


  // reset. pull lo then high.
  gpio_clear( TFT_GPIO_PORT, TFT_REST);
  msleep(20);
  gpio_set( TFT_GPIO_PORT, TFT_REST);
  msleep(200);

}



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

  uint16_t portd_gpios = GPIO0 | GPIO1 | GPIO8 | GPIO9 | GPIO10 | GPIO14 | GPIO15;
  // gpio_set_mode(GPIOD, GPIO_MODE_OUTPUT_2_MHZ, GPIO_CNF_OUTPUT_ALTFN_PUSHPULL, portd_gpios);
  gpio_mode_setup(GPIOD, GPIO_MODE_AF, GPIO_PUPD_NONE, portd_gpios);
  gpio_set_output_options(GPIOD, GPIO_OTYPE_PP, GPIO_OSPEED_25MHZ, portd_gpios);
  gpio_set_af(GPIOD, GPIO_AF12, portd_gpios);



  uint16_t porte_gpios = GPIO7 | GPIO8 | GPIO9 | GPIO10 | GPIO11 | GPIO12 | GPIO13 | GPIO14 | GPIO15;
  // gpio_set_mode(GPIOE, GPIO_MODE_OUTPUT_2_MHZ, GPIO_CNF_OUTPUT_ALTFN_PUSHPULL, porte_gpios);
  gpio_mode_setup(GPIOE, GPIO_MODE_AF, GPIO_PUPD_NONE, porte_gpios);
  gpio_set_output_options(GPIOE, GPIO_OTYPE_PP, GPIO_OSPEED_25MHZ, porte_gpios);
  gpio_set_af(GPIOE, GPIO_AF12, porte_gpios);


  // these could be consolidated...

 /* config FSMC NOE */
  // gpio_set_mode(GPIOD, GPIO_MODE_OUTPUT_2_MHZ, GPIO_CNF_OUTPUT_ALTFN_PUSHPULL, GPIO4);
  gpio_mode_setup(GPIOD, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO4);
  gpio_set_output_options(GPIOD, GPIO_OTYPE_PP, GPIO_OSPEED_25MHZ, GPIO4);
  gpio_set_af(GPIOD, GPIO_AF12, GPIO4);


 /* config FSMC NWE */
  // gpio_set_mode(GPIOD, GPIO_MODE_OUTPUT_2_MHZ, GPIO_CNF_OUTPUT_ALTFN_PUSHPULL, GPIO5);
  gpio_mode_setup(GPIOD, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO5);
  gpio_set_output_options(GPIOD, GPIO_OTYPE_PP, GPIO_OSPEED_25MHZ, GPIO5);
  gpio_set_af(GPIOD, GPIO_AF12, GPIO5);


 /* config FSMC NE1 */
  /* JA PD7 CS */
  // gpio_set_mode(GPIOD, GPIO_MODE_OUTPUT_2_MHZ, GPIO_CNF_OUTPUT_ALTFN_PUSHPULL, GPIO7);
  gpio_mode_setup(GPIOD, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO7);
  gpio_set_output_options(GPIOD, GPIO_OTYPE_PP, GPIO_OSPEED_25MHZ, GPIO7);
  gpio_set_af(GPIOD, GPIO_AF12, GPIO7);



 /* config FSMC A16 for D/C (select Data/Command ) */
  /* JA PD11  RS */
  // gpio_set_mode(GPIOD, GPIO_MODE_OUTPUT_2_MHZ, GPIO_CNF_OUTPUT_ALTFN_PUSHPULL, GPIO11);
  gpio_mode_setup(GPIOD, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO11);
  gpio_set_output_options(GPIOD, GPIO_OTYPE_PP, GPIO_OSPEED_25MHZ, GPIO11);
  gpio_set_af(GPIOD, GPIO_AF12, GPIO11);



#if 0
#endif


 /* config FSMC register */
  FSMC_BTR(0) = FSMC_BTR_ACCMODx(FSMC_BTx_ACCMOD_B) |
                FSMC_BTR_DATLATx(0)  |
                FSMC_BTR_CLKDIVx(0)  |
                // FSMC_BTR_CLKDIVx(16)  |
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


#if 0

static uint16_t LCD_ReadRAM(void)
 {

  /* Write 16-bit Index (then Read Reg) */

  // LCD->LCD_REG = R34 /* Select GRAM Reg */

  // JA
  LCD->LCD_REG = 0x0A ; /* Select GRAM Reg */
                          // 0x0A get power mode....
                          // hang on. these are commands... that accept parameters...

  /* Read 16-bit Reg */

  return LCD->LCD_RAM;
 }
#endif


static uint16_t LCD_ReadReg(uint8_t LCD_Reg);

static void LCD_WriteReg(uint8_t LCD_Reg, uint16_t LCD_RegValue);








int main(void)
{

  // required for usb
	rcc_clock_setup_pll(&rcc_hse_8mhz_3v3[RCC_CLOCK_3V3_84MHZ] );  // stm32f411  upto 100MHz. works stm32f407 too.
	// rcc_clock_setup_pll(&rcc_hse_12mhz_3v3[RCC_CLOCK_3V3_168MHZ] );  // stm32f407

  /*
  // http://libopencm3.org/docs/latest/stm32f4/html/f4_2rcc_8h.html

    see here for rcc.c example, defining 100MHz, using 25MHz xtal.
      https://github.com/insane-adding-machines/unicore-mx/blob/master/lib/stm32/f4/rcc.c
  */

  // clocks
  rcc_periph_clock_enable(RCC_SYSCFG); // maybe required for external interupts?

  // LED
  // rcc_periph_clock_enable(RCC_GPIOA); // f410/f411 led.
  rcc_periph_clock_enable(RCC_GPIOB); // f410/f411 led.

  // USART
  // rcc_periph_clock_enable(RCC_GPIOA);     // f407
  rcc_periph_clock_enable(RCC_GPIOB); // F410/f411
  rcc_periph_clock_enable(RCC_USART1);


  // USB
	// rcc_periph_clock_enable(RCC_GPIOA);
	rcc_periph_clock_enable(RCC_OTGFS);

  // SHOULD PUT ALL TFT stuff in header... or at least predeclare.
  // parallel tft / ssd1963
  rcc_periph_clock_enable(RCC_GPIOE);
  rcc_periph_clock_enable(RCC_GPIOD);



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
  // systick_setup(16000);


  // led
  led_setup();


  memset(&app, 0, sizeof(app_t));

  // uart/console
  cBufInit(&app.console_in,  buf_console_in, sizeof(buf_console_in));
  cBufInit(&app.console_out, buf_console_out, sizeof(buf_console_out));

  // usart_setup_gpio_portA();
  usart_setup_gpio_portB();

  usart_set_buffers(&app.console_in, &app.console_out);

  // setup print
  // usart_printf_set_buffer()
  usart_printf_init(&app.console_out);


  ////////////////////////////
  // usb
  // might be better to pass as handler?
	app.usbd_dev = usb_setup();
  ASSERT(app.usbd_dev);




  // make sure have access to usart_printf
  fsmc_setup();

  // do the reset.
  tft_gpio_init();


  // uint16_t x = LCD_ReadRAM();



  usart_printf("\n--------");
  usart_printf("\nstarting\n");

  // 0x0A is the power mode?????   - we could test if power
  // need bit presentation

  char buf[100];
  // uint16_t reg = 0x0A; // get_power_mode.   successive read can change 4th bit.
  uint16_t reg = 0x0B;  // get_address_mode.  nothing changes????
  volatile uint16_t x;


  for(reg = 0x0A; reg < 72 ; reg++) {
    x = LCD_ReadReg( reg );
    // usart_printf("reg %u (%x)  read %u   %s\n", reg, reg, x, format_bits(buf, 16, x));
    // EXTR... there's a memory issue is this string gets too long?
    usart_printf("reg %u (%02x)  r %u  %s\n", reg,  reg, x, format_bits(buf, 16, x));
    msleep(10);
  }

/*
  usart_printf("----\n");
  msleep(20);
  for(reg = 0x0A; reg < 72 ; reg++) {
    x = LCD_ReadReg( reg );
    // usart_printf("reg %u (%x)  read %u   %s\n", reg, reg, x, format_bits(buf, 16, x));
    // EXTR... there's a memory issue is this string gets too long?
    usart_printf("reg %u (%02x)  r %u  %s\n", reg,  reg, x, format_bits(buf, 16, x));
    msleep(10);
  }
*/


  /*
    OK. the bottom two bits of 0x0D  get_display_mode should be hi. according to doc.
    while our code. bottom bits of 0x0E are high.

    because the sequencing is out of alignment???
    NOP (0) and soft reset (1) take no parameters
    --------------

    bits are mixed up.  reg 0x0a should have D3. on at POR.
    instead             reg 0x0b has this bit.

                        reg 0x0d should have D0 and D1 at POR
    instead             reg 0x0e has these set.

    possible high byte of register - should be configured differently?
    manual states on 8bits used.

    TRY
    - should try a write - and see which values get written.
    - should read all registers twice. make sure nothing changes.

    - make sure we are not sending a parameter. which could throw things off.
    - need to probe everything.

    - bitbang as gpio. eg. just try to read a copule of registers.

    - does it need an entry in the linker script?

    - we need to understand how arguments are presented.
        is it a series of writes.

    - is a write with no arguments the same as a read.

    - setup on loop.  just on the d register...
        nothing else - that could be interpreted differently as a write.

first time.
  reg 10 (a)  r 0  0000000000000000
  reg 11 (b)  r 8  0000000000001000
  reg 12 (c)  r 0  0000000000000000
  reg 13 (d)  r 0  0000000000000000
  reg 14 (e)  r 3  0000000000000011
  reg 15 (f)  r 0  0000000000000000
  reg 16 (10)  r 0  0000000000000000


second time. (think that just accessing a address - may be a command,  )...
should try a loop for the same address...
  reg 10 (a)  r 0  0000000000000000
  reg 11 (b)  r 92  0000000001011100
  reg 12 (c)  r 0  0000000000000000
  reg 13 (d)  r 0  0000000000000000
  reg 14 (e)  r 35  0000000000100011
  reg 15 (f)  r 0  0000000000000000


  so reading is changing values somehow...
*/

/*
  /////////////
  usart_printf("reset cmd\n");
  LCD_WriteReg( 0x01 , 0x0 ); // soft reset. takes no parameter. but we are giving it one.
  msleep(10); // sleep is required.
*/



  // conssequtive reads and the value changes...
#if 0
  x = LCD_ReadReg( reg );
  usart_printf("read %u   %s\n", x, format_bits(buf, 8, x));

  x = LCD_ReadReg( reg );
  usart_printf("read %u   %s\n", x, format_bits(buf, 8, x));


  // any kind of write value... sets a bit...
  usart_printf("write ram \n");
  LCD_WriteReg( reg , 0x00 );

  x = LCD_ReadReg( reg );
  usart_printf("read %u   %s\n", x, format_bits(buf, 8, x));

  x = LCD_ReadReg( reg );
  usart_printf("read %u   %s\n", x, format_bits(buf, 8, x));

  /////////////
  usart_printf("reset cmd\n");
  LCD_WriteReg( 0x01 , 0x0 ); // soft reset. takes no parameter. but we are giving it one.
  msleep(10); // sleep is required.

  x = LCD_ReadReg( reg );
  usart_printf("read %u   %s\n", x, format_bits(buf, 8, x));



  // ok writing a 0, and we end up with a value...
  // no it appears the second time we read... it changes the value....

  usart_printf("write 0 \n");
  LCD_WriteReg( reg , 0x00 );
  x = LCD_ReadReg( reg );

  x = LCD_ReadReg( reg );
  usart_printf("read %u   %s\n", x, format_bits(buf, 8, x));

  x = LCD_ReadReg( reg );
  usart_printf("read %u   %s\n", x, format_bits(buf, 8, x));


#endif


  // so a write



/*
    - should try a soft reset command. and see if it clears it?

    - also need to probe the scope - to make sure all bits/ are soldered ok.
*/

/*
  usart_printf("\n--------\n");
  usart_printf("starting loop\n");
  usart_printf("sizeof bool   %u\n", sizeof(bool));
  usart_printf("sizeof float  %u\n", sizeof(float));
  usart_printf("sizeof double %u\n", sizeof(double));
*/
  // test assert failure
  ASSERT(1 == 2);

  usart_printf("a float formatted %g\n", 123.456f );


  loop(&app);
}

// put the code after the function. to prevent inlining.

static uint16_t LCD_ReadReg(uint8_t LCD_Reg)
 {

  /* Write 16-bit Index (then Read Reg) */

  LCD->LCD_REG = LCD_Reg;

  /* Read 16-bit Reg */

  return (LCD->LCD_RAM);
 }


static void LCD_WriteReg(uint8_t LCD_Reg, uint16_t LCD_RegValue)
 {

  /* Write 16-bit Index, then Write Reg */

  LCD->LCD_REG = LCD_Reg;

  /* Write 16-bit Reg */

  LCD->LCD_RAM = LCD_RegValue;
 }


