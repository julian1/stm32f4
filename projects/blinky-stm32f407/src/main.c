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

#include "fsmc-ssd1963.h"

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




/*********************************************************/
static void LCD_Init(void) 
{
  usart_printf("-----------\n");
  usart_printf("LCD_Init\n");

  // LCD_Configuration();
  // fsmc_gpio_setup();
  fsmc_setup(12);
  tft_reset();

#if 1
  /* Set MN(multipliers) of PLL, VCO = crystal freq * (N+1) */
  /* PLL freq = VCO/M with 250MHz < VCO < 800MHz */
  /* The max PLL freq is around 120MHz. To obtain 120MHz as the PLL freq */
  LCD_WriteCommand(0xE2); /* Set PLL with OSC = 10MHz (hardware) */
  /* Multiplier N = 35, VCO (>250MHz)= OSC*(N+1), VCO = 360MHz */
  LCD_WriteData(0x23);
  LCD_WriteData(0x02); /* Divider M = 2, PLL = 360/(M+1) = 120MHz */
  LCD_WriteData(0x54); /* Validate M and N values */
#endif

}



int main(void)
{

  // required for usb
	rcc_clock_setup_pll(&rcc_hse_8mhz_3v3[RCC_CLOCK_3V3_84MHZ] );  // stm32f411  upto 100MHz. works stm32f407 too.
	// rcc_clock_setup_pll(&rcc_hse_8mhz_3v3[RCC_CLOCK_3V3_168MHZ] );  // stm32f407

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


  // TFT
  // SHOULD PUT ALL TFT stuff in header... or at least predeclare.
  // parallel tft / ssd1963
  rcc_periph_clock_enable(RCC_GPIOD);
  rcc_periph_clock_enable(RCC_GPIOE);

  rcc_periph_clock_enable(RCC_FSMC);


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
  // systick_setup(168000);
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
  



  fsmc_gpio_setup();

  // fsmc_setup(12);
  // tft_reset();

  LCD_Init(); 



  usart_printf("\n--------");
  usart_printf("\nstarting\n");

  // 0x0A is the power mode?????   - we could test if power
  // need bit presentation

  char buf[100];
  // uint16_t reg = 0x0A; // get_power_mode.   successive read can change 4th bit.
  uint16_t reg = 0x0B;  // get_address_mode.  nothing changes????
  volatile uint16_t x;


#if 1
  while(1) {
    /*
    // read_ddb. a lot of serial stuff. 
    reg 161 (a1)  r
      001  0000000000000001
      087  0000000001010111
      097  0000000001100001
      001  0000000000000001
      255  0000000011111111
    */

    // reg = 0x0A;   // == 1000
    reg = 0xA1;   // read_ddb,    5 parameter register.

    LCD_SetAddr(reg );

    uint16_t x1 = LCD_ReadData();
    uint16_t x2 = LCD_ReadData();
    uint16_t x3 = LCD_ReadData();
    uint16_t x4 = LCD_ReadData();
    uint16_t x5 = LCD_ReadData();

    usart_printf("reg %u (%02x)  r\n", reg,  reg);
    usart_printf("%03u  %s\n", x1, format_bits(buf, 16, x1));
    usart_printf("%03u  %s\n", x2, format_bits(buf, 16, x2));
    usart_printf("%03u  %s\n", x3, format_bits(buf, 16, x3));
    usart_printf("%03u  %s\n", x4, format_bits(buf, 16, x4));
    usart_printf("%03u  %s\n", x5, format_bits(buf, 16, x5));

    msleep(1000);
  }
#endif





  LCD_SetAddr( 0x01 );
  LCD_SetAddr( 0x01 );
  LCD_SetAddr( 0x01 );
  LCD_SetAddr( 0x01 );
  msleep(50);

  while(1) {

    // also see read_ddb. a lot of serial stuff. 

    usart_printf("---------\n");

#if 1
    reg = 0x0A;   // == 1000
    LCD_SetAddr(reg );
    x = LCD_ReadData();
    usart_printf("reg %u (%02x)", reg,  reg);
    usart_printf("%03u  %s\n", x, format_bits(buf, 16, x)); // maybe the printf buf. is not being copied???? 
    // msleep(1000);

    reg = 0x0D;   // == 11
    LCD_SetAddr(reg );
    x = LCD_ReadData();
    usart_printf("reg %u (%02x)  r %u  %s\n", reg,  reg, x, format_bits(buf, 16, x));
    // msleep(1000);

    reg = 0x0A;   // == 1000
    LCD_SetAddr(reg );
    x = LCD_ReadData();
    usart_printf("reg %u (%02x)", reg,  reg);
    usart_printf("%03u  %s\n", x, format_bits(buf, 16, x)); // maybe the printf buf. is not being copied???? 


    msleep(200);



#endif

    /* this reg is weird.
    reg = 0x26;   
    LCD_SetAddr(reg );
    // x = LCD_ReadReg( reg );
    x = LCD_ReadData();
    usart_printf("reg %u (%02x)  r %u  %s\n", reg,  reg, x, format_bits(buf, 16, x));
    msleep(1000);
    */



  }



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



