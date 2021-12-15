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

/*
  800x480.
  https://www.ebay.com.au/itm/331825585247
    we need the timings to use.

  https://github.com/stm32f4/library/blob/master/SSD1963/GLCD.c#L230

  google search.

  DISP_HOR_RESOLUTION 800 ssd1963

    https://arm-stm.blogspot.com/2016/12/ssd1963-init-collection.html
    https://os.mbed.com/teams/TECHSTEP/code/Nucleo_display_ssd1963/file/2714fcd95190/SSD1963/SSD1963.h/

  actually think it's 4.3" 480x272
    xtal is 10MHz.
  search on gl043036c0-40
    matches.
    https://chfile.cn.gcimg.net/gcwthird/day_20170726/efe81a3b93u32f90950f5i86c0104575.pdf

    like this.
      https://www.ebay.com.au/itm/164555517828

  good code here. 480x272.
    https://community.nxp.com/t5/Kinetis-Microcontrollers/Kinetics-with-SSD1963-interface/m-p/782183

    LCDC_FPR  = 12MHz.  which good for spec.

  screen datasheet.
    GL043036C0-40
    see table 9. for timing characteristics
      https://chfile.cn.gcimg.net/gcwthird/day_20170726/efe81a3b93u32f90950f5i86c0104575.pdf
  search on

*/

/*********************************************************/
#if 0
static void LCD_Init(void)
{
  usart_printf("-----------\n");
  usart_printf("LCD_Init\n");

  // LCD_Configuration();
  // fsmc_gpio_setup();
  fsmc_setup(12);
  tft_reset();


  /* Set MN(multipliers) of PLL, VCO = crystal freq * (N+1) */
  /* PLL freq = VCO/M with 250MHz < VCO < 800MHz */
  /* The max PLL freq is around 120MHz. To obtain 120MHz as the PLL freq */
  LCD_WriteCommand(0xE2); /* Set PLL with OSC = 10MHz (hardware) */
  /* Multiplier N = 35, VCO (>250MHz)= OSC*(N+1), VCO = 360MHz */
  LCD_WriteData(0x23);
  LCD_WriteData(0x02); /* Divider M = 2, PLL = 360/(M+1) = 120MHz */
  LCD_WriteData(0x54); /* Validate M and N values */

  LCD_WriteCommand(0xE0); /* Start PLL command */
  LCD_WriteData(0x01); /* enable PLL */
  // JA delay_ms(10); /* wait stabilization */
  msleep(10);

  LCD_WriteCommand(0xE0); /* Start PLL command again */
  LCD_WriteData(0x03); /* now, use PLL output as system clock */

  // JA LCD_FSMCConfig(1); /* Set FSMC full speed now */
  fsmc_setup(1);

  /* once PLL locked (at 120MHz), the data hold time is shortened */
  LCD_WriteCommand(0x01); /* Soft reset */
  // JA delay_ms(10);
  msleep(10);



  /* Set LSHIFT freq, i.e. the DCLK with PLL freq 120MHz set previously */
  /* Typical DCLK for TYX350TFT320240 is 6.5MHz in 24 bit format */
  /* 6.5MHz = 120MHz*(LCDC_FPR+1)/2^20 */
  /* LCDC_FPR = 56796 (0x00DDDC) */
  LCD_WriteCommand(0xE6);
  LCD_WriteData(0x00);
  LCD_WriteData(0xDD);
  LCD_WriteData(0xDC);


     // for 4.3 inch lcd 12 MHz
    // Typical DCLK TRULY is max 12MHz
    // 12 = 100 * (LCDC_FPR + 1) / 2^20
    // LCD_FPR = 125828 (0x1EB84)
    (void)D4D_LLD_LCD_HW.D4DLCDHW_SendDataWord(0x01);
    (void)D4D_LLD_LCD_HW.D4DLCDHW_SendDataWord(0xeb);
    (void)D4D_LLD_LCD_HW.D4DLCDHW_SendDataWord(0x84);


  /* Set panel mode, varies from individual manufacturer */
  LCD_WriteCommand(0xB0);
  LCD_WriteData(0x20); /* Set 24-bit 3.5" TFT Panel */
  LCD_WriteData(0x00); /* set Hsync+Vsync mode */
  LCD_WriteData((DISP_HOR_RESOLUTION - 1) >> 8 & 0x07); /* Set panel size */
  LCD_WriteData((DISP_HOR_RESOLUTION - 1) & 0xff);
  LCD_WriteData((DISP_VER_RESOLUTION - 1) >> 8 & 0x07);
  LCD_WriteData((DISP_VER_RESOLUTION - 1) & 0xff);
  LCD_WriteData(0x00); /* RGB sequence */

  /* Set horizontal period */
  LCD_WriteCommand(0xB4);
}
#endif


static void LCD_Write_COM(uint16_t cmd)
{
 LCD_WriteCommand(cmd) ;
}

static void  LCD_Write_DATA(uint16_t data)
{
  LCD_WriteData(data) ;
}

#define SWAP(a, b) do { typeof(a) temp = a; a = b; b = temp; } while (0)
// #define swap(a,b) SWAP(a,b)

static void setXY(uint16_t x1,  uint16_t y1,uint16_t x2,  uint16_t y2 )
{
//case SSD1963_480:
//   swap(word, x1, y1);
//   swap(word, x2, y2);

  LCD_Write_COM(0x2a);  // JA set_column_address
    LCD_Write_DATA(x1>>8);
    LCD_Write_DATA(x1);
    LCD_Write_DATA(x2>>8);
    LCD_Write_DATA(x2);
  LCD_Write_COM(0x2b);  // JA set_page_address
    LCD_Write_DATA(y1>>8);
    LCD_Write_DATA(y1);
    LCD_Write_DATA(y2>>8);
    LCD_Write_DATA(y2);
  LCD_Write_COM(0x2c); // write_memory_start
  // break;
}

static void LCD_Init(void)
{

  usart_printf("-----------\n");
  usart_printf("LCD_Init\n");

  // LCD_Configuration();
  // fsmc_gpio_setup();
  fsmc_setup(12);   // slow.
  tft_reset();


  /* Set MN(multipliers) of PLL, VCO = crystal freq * (N+1) */
  /* PLL freq = VCO/M with 250MHz < VCO < 800MHz */
  /* The max PLL freq is around 120MHz. To obtain 120MHz as the PLL freq */
  LCD_WriteCommand(0xE2); /* Set PLL with OSC = 10MHz (hardware) */
  /* Multiplier N = 35, VCO (>250MHz)= OSC*(N+1), VCO = 360MHz */
  LCD_WriteData(0x23);
  LCD_WriteData(0x02); /* Divider M = 2, PLL = 360/(M+1) = 120MHz */
  LCD_WriteData(0x54); /* Validate M and N values */

  LCD_WriteCommand(0xE0); /* Start PLL command */
  LCD_WriteData(0x01); /* enable PLL */
  // JA delay_ms(10); /* wait stabilization */
  msleep(10);

  LCD_WriteCommand(0xE0); /* Start PLL command again */
  LCD_WriteData(0x03); /* now, use PLL output as system clock */

  // JA LCD_FSMCConfig(1); /* Set FSMC full speed now */
  fsmc_setup(1);
  msleep(10);

  LCD_Write_COM(0x01);    // software reset (JA requiredJ).
  msleep(100);

  // https://github.com/jscrane/UTFT-Energia/blob/master/tft_drivers/ssd1963/480/initlcd.h
// case SSD1963_480:

/*
  LCD_Write_COM(0xE2);    //PLL multiplier, set PLL clock to 120M
  LCD_Write_DATA(0x23);     //N=0x36 for 6.5M, 0x23 for 10M crystal
  LCD_Write_DATA(0x02);
  LCD_Write_DATA(0x54);

  LCD_Write_COM(0xE0);    // PLL enable
  LCD_Write_DATA(0x01);
  delay(10);
  LCD_Write_COM(0xE0);
  LCD_Write_DATA(0x03);
  delay(10);
  LCD_Write_COM(0x01);    // software reset
  delay(100);
*/


/*
                          // 73727
  LCD_Write_COM(0xE6);    //PLL setting for PCLK, depends on resolution
  LCD_Write_DATA(0x01);
  LCD_Write_DATA(0x1F);
  LCD_Write_DATA(0xFF);
*/



  // this works dclk is pin30 of the flex cable to the screen.
  // probed 12MHz on scope.
  // JA 105000 ==  19A28
  // 120 * (   105000 + 1 ) / Math.pow(2, 20)
  // ==  12.016410827636719 == 12MHz.
  LCD_Write_COM(0xE6);    //PLL setting for PCLK, depends on resolution
  LCD_Write_DATA(0x01);
  LCD_Write_DATA(0xA2);
  LCD_Write_DATA(0x28);



  LCD_Write_COM(0xB0);    //LCD SPECIFICATION
  LCD_Write_DATA(0x20);
  LCD_Write_DATA(0x00);
  LCD_Write_DATA(0x01);   //Set HDP 479
  LCD_Write_DATA(0xDF);
  LCD_Write_DATA(0x01);   //Set VDP 271
  LCD_Write_DATA(0x0F);
  LCD_Write_DATA(0x00);

  LCD_Write_COM(0xB4);    //HSYNC
  LCD_Write_DATA(0x02);   //Set HT  531
  LCD_Write_DATA(0x13);
  LCD_Write_DATA(0x00);   //Set HPS 8
  LCD_Write_DATA(0x08);
  LCD_Write_DATA(0x2B);   //Set HPW 43
  LCD_Write_DATA(0x00);   //Set LPS 2
  LCD_Write_DATA(0x02);
  LCD_Write_DATA(0x00);

  LCD_Write_COM(0xB6);    //VSYNC
  LCD_Write_DATA(0x01);   //Set VT  288
  LCD_Write_DATA(0x20);
  LCD_Write_DATA(0x00);   //Set VPS 4
  LCD_Write_DATA(0x04);
  LCD_Write_DATA(0x0c);   //Set VPW 12
  LCD_Write_DATA(0x00);   //Set FPS 2
  LCD_Write_DATA(0x02);

/*
  // TODO remove
  LCD_Write_COM(0xBA);  // JA set_gpio_value
  LCD_Write_DATA(0x0F);   //GPIO[3:0] out 1

  LCD_Write_COM(0xB8);  // JA set_gpio_conf
  LCD_Write_DATA(0x07);     //GPIO3=input, GPIO[2:0]=output
  LCD_Write_DATA(0x01);   //GPIO0 normal
*/

  LCD_Write_COM(0x36);    //rotation
  LCD_Write_DATA(0x22);


  LCD_Write_COM(0xF0);    //pixel data interface
  LCD_Write_DATA(0x03);       // 3 == 011 == 16bit 565  
  // LCD_Write_DATA(0b101 );       // JA 101 24-bit default.



  // delay(1);
  // delay(1);
  msleep(1);

  LCD_Write_COM(0x29);    //display on

/*
  LCD_Write_COM(0xBE);    //set PWM for B/L
  LCD_Write_DATA(0x06);
  LCD_Write_DATA(0xf0);
  LCD_Write_DATA(0x01);
  LCD_Write_DATA(0xf0);
  LCD_Write_DATA(0x00);
  LCD_Write_DATA(0x00);

  LCD_Write_COM(0xd0);   // JA set dynamic backlight configuration
  LCD_Write_DATA(0x0d);
*/



  usart_printf("writing some data\n");

  /*
    it's hard to see the change against background unless,


  */
  // setXY(20, 20, 479, 271);
  setXY(20, 20, 100, 200);
  // setXY(20, 20, 100, 100);
  // setXY(100, 100, 20, 20);

  // LCD_Write_COM(0x2C);    // JA write memory start
  for( int i  = 20 * 20 ; i < 100 * 200; ++i ) {

    // NO. should be 16bit values.... not 8 bit. eg. only registers use lower 8 bits of 16 bit bus. 
    // LCD_Write_DATA(0xff << 6 | 0x00 );
    // LCD_Write_DATA(0x0 << 8 | 0x00 );
    // LCD_Write_DATA(0xff << 8 | 0x00 );
    // LCD_Write_DATA(0x00); // hmmmmm
    // LCD_Write_DATA(0xff );
  
    // this is bright red
    // LCD_Write_DATA(  0x00 | 0xff >> 5);

    // 
    // 11111 = 1F
    // 111111 == 3F 

    // bright red
    // LCD_Write_DATA(   (0x1f ) ) ;

    // bright yellow
    // LCD_Write_DATA(   (0x1fu ) << 11) ;

    // 101 = 565 format
    // uint16_t r = 0xff, g = 0xff, b = 0xff; 
    uint16_t r = 0x00, g = 0x0, b = 0xff; 
    // white/ grety
    // LCD_Write_DATA(   0xffff   ) ;


  
    // rgb 565
    // rgb555
    LCD_Write_DATA(   (r & 0x1f) << 11 | (g & 0x3f << 5) | (b & 0x1f ) ) ;


  }


  // break;
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

  fsmc_setup(1);
  tft_reset();

  LCD_Init();



  usart_printf("\n--------");
  usart_printf("\nstarting\n");

  char buf[100];
  UNUSED(buf);

  // OK. looks like we have not successfully written any values....
  // but it appeared that
  // EXTR. Not sure. unless we do the init sequence. then setting divider == 1. means reads fail.


#if 0
  while(1) {

    // weird - attempting to read the register kills it ????
    // why?
    uint16_t reg = 0xE2;   //

    LCD_SetAddr(reg );
    uint16_t x1 = LCD_ReadData();
    uint16_t x2 = LCD_ReadData();
    uint16_t x3 = LCD_ReadData();
    usart_printf("reg %u (%02x)  r\n", reg,  reg);
    usart_printf("%03u  %s\n", x1, format_bits(buf, 16, x1));
    usart_printf("%03u  %s\n", x2, format_bits(buf, 16, x2));
    usart_printf("%03u  %s\n", x3, format_bits(buf, 16, x3));

    msleep(2000);
  }
#endif



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
    uint16_t reg = 0xA1;   // read_ddb,    5 parameter register.
    //uint16_t reg = 0xE2;   //

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

    msleep(2000);
  }
#endif






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



