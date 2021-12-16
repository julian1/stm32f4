/*
  We put this code out of the fsmc code.
  because having it inlined causes problems with aggressive optimisation. of the bus addresses.
*/

#include <stdint.h>

#include "fsmc.h"
#include "ssd1963.h"

#include "util.h"   // printf, msleep
#include "str.h"  //format_bits. should rename




/*
  datasheet
  https://www.seacomp.com/sites/default/files/references/Solomon-Systech-SSD1963.pdf

  good video on front porch/back porch etc.
    https://www.youtube.com/watch?v=SgtP4MZ9Hys

  actually think it's 4.3" 480x272
    xtal is 10MHz.
  search on gl043036c0-40
    matches.
    https://chfile.cn.gcimg.net/gcwthird/day_20170726/efe81a3b93u32f90950f5i86c0104575.pdf

    like this.
      https://www.ebay.com.au/itm/164555517828

  good code here. 480x272.
    https://community.nxp.com/t5/Kinetis-Microcontrollers/Kinetics-with-SSD1963-interface/m-p/782183

  http://www.ampdisplay.com/documents/pdf/SSD1963%20800480S.txt

    LCDC_FPR  = 12MHz.  which good for spec.

  screen datasheet.
    GL043036C0-40
    see table 9. for timing characteristics
      https://chfile.cn.gcimg.net/gcwthird/day_20170726/efe81a3b93u32f90950f5i86c0104575.pdf

  -------------

  - tear sync
    7.1.5  explains tear.
    feedback from lcd to mcu. with timing information.
    can also get it by reading the get_tear_effect_status
    if cpu > lcd
    gpio interupt.  but poll good in a loop() waiting for redraw.

  - paging
    https://www.microchip.com/forums/m633574.aspx
    https://www.avrfreaks.net/forum/ssd1963-tft-paging

      Yes, this is possible. I recently started experimenting with the SSD1963 and a 480x272 display and was able to accomplish this.

      The frame buffer appears as several pages stacked on top of each other.
      Writing to lines between 0 and 271 are the 1st page.
      Writing to lines between 272 and 543 are the 2nd page.
      Writing to lines between 544 and 815 are the 3rd page.

      Then you set the value in Set Scroll Start to the line in the frame buffer where you want to start displaying from.
      Switching between pages is very quick, no noticeable flashing.

      ---
      note that - this could be done in the actual render_bu. doing the copy_hline operations etc.

  - timing
    we can just use the systick timer
      - to determine how long something takes to draw. eg. down to the ms.
      - to write the blit/clear the whole screen.
      awesome.

  - IMPR
      - change the 5 multipler for the setup time. and see if still workds. and if there's a timing difference.


*/

// #  - IMPR
 //     - change the 5 multipler for the setup time. and see if still workds. and if there's a timing difference.



/*********************************************************/


static void LCD_Write_COM(uint16_t cmd)
{
 LCD_WriteCommand(cmd) ;
}

static void  LCD_Write_DATA(uint16_t data)
{
  LCD_WriteData(data) ;
}




void LCD_Init(void)
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
  // LCD_Write_DATA(0x22);  // 0x22 == 10110 origin is top-right
  LCD_Write_DATA(0x00);     // 0x00 origin is top-left
  // LCD_Write_DATA(0b0011  );
  // LCD_Write_DATA(0b0010  );     // bottom right?
  // LCD_Write_DATA(0b0001  );     // origin bottom left


  LCD_Write_COM(0xF0);    //pixel data interface
  LCD_Write_DATA(0x03);       // 3 == 011 == 16bit 565
  // LCD_Write_DATA(0b101 );       // JA 101 24-bit default.


  // LCD_Write_COM(0x35);     // set tear on
  // LCD_Write_DATA(0x00);   // vblanking only   



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



  // break;
}





void setScrollStart(uint16_t y)
{
  /*
  This command sets the start of the vertical scrolling area in the frame buffer. The vertical scrolling area is fully defined
  when this command is used with the set_scroll_area (0x33).
  */

  LCD_Write_COM(0x37);  
    LCD_Write_DATA(y>>8);
    LCD_Write_DATA(y);
}





void setXY(uint16_t x1,  uint16_t y1,uint16_t x2,  uint16_t y2 )
{
/*
  passing in a scroll offset context - would be very useful for paging/double buffering
*/

// https://github.com/jscrane/UTFT-Energia/blob/master/tft_drivers/ssd1963/480/setxy.h
// https://github.com/stm32f4/library/blob/master/SSD1963/GLCD.c

//case SSD1963_480:
//   swap(word, x1, y1);
//   swap(word, x2, y2);

  LCD_Write_COM(0x2a);  // JA set_column_address
    LCD_Write_DATA(x1>>8);
    LCD_Write_DATA(x1);
    LCD_Write_DATA((x2-1)>>8);
    LCD_Write_DATA(x2-1);
  LCD_Write_COM(0x2b);  // JA set_page_address
    LCD_Write_DATA(y1>>8);
    LCD_Write_DATA(y1);
    LCD_Write_DATA((y2 - 1)>>8);
    LCD_Write_DATA(y2-1);
  LCD_Write_COM(0x2c); // write_memory_start
  // break;
}



void setOriginTopLeft()
{
  LCD_Write_COM(0x36);    //rotation
  LCD_Write_DATA(0x00);     // 0x00 origin is top-left
}

void setOriginBottomLeft()
{
  // better for truetype fonts...
  LCD_Write_COM(0x36);    //rotation
  LCD_Write_DATA(0b0001  );     // origin bottom left
}




/*
In the non-display period, the TE signal will go high.
However, if the
MCU is faster than the LCD controller, it should start updating the display content in the vertical non-display
period (VNDP) to enable the LCD controller will always get the newly updated data.

0x0E get_tear_effect_status

0x34set_tear_off
0x35set_tear_on
0x45  get_scanline

*/

uint16_t getTearEffectStatus(void )
{
  LCD_Write_COM(0x0E);    //rotation
  uint16_t x1 = LCD_ReadData();
  return x1; 
}


void LCD_SetTearOn(void )
{
  // this doesn't appear to work. 
  // cannot read anything with getTearEffectSttaus or probe with scope 
  LCD_Write_COM(0x35);    
  LCD_Write_DATA(0x00);   // vblanking only   
}





/*
// - Color RGB R5 G6 B5 -------------------------------------------------------
uint16_t SSD1963::Color565(uint8_t r, uint8_t g, uint8_t b) {
  uint16_t c;
  c = r >> 3;
  c <<= 6;
  c |= g >> 2;
  c <<= 5;
  c |= b >> 3;
  return c;
}
*/


uint16_t packRGB565( uint16_t r, uint16_t g, uint16_t b)
{

  return (r & 0x1f ) << 11 | (g & 0x3f) << 5 | (b & 0x1f) ;

}

void LCD_fillRect(uint16_t x1,  uint16_t y1,uint16_t x2,  uint16_t y2, uint16_t c )
{

  setXY(x1, y1, x2, y2);

  int len =  (x2 - x1) * (y2 - y1 );

  for( int i  = 0; i < len ; ++i ) {
    LCD_Write_DATA(   c  ) ;
  }

}


void LCD_Read_DDB()
{

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
  char buf[100];
  uint16_t reg = 0xA1;   // read_ddb,    5 parameter register.
  //uint16_t reg = 0xE2;   //

  //  LCD_SetAddr(reg );
  LCD_WriteCommand( reg ) ;


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

}






void  LCD_TestFill()
{
  // avoid filling corners - so can see that works.

  // put in function. LCD_testFill.

  LCD_fillRect(1, 1, 480 -1, 10 , packRGB565( 0xff , 0xff, 0xff));

  LCD_fillRect(1, 20, 480 -1, 30 , packRGB565( 0xff , 0xff, 0xff));


  // LCD_fillRect(1, 50, 480 -1, 50 , packRGB565( 0xff , 0xff, 0xff)); // height of 0. draws nothing
  LCD_fillRect(1, 50, 480 -1, 51 , packRGB565( 0xff , 0xff, 0xff)); // height of 1. draws


  LCD_fillRect(5, 5, 50, 50, packRGB565( 0x0, 0x0, 0xff));


//   agg_test2();

}












/*

void SSD1963_WindowSet(u16 S_X,u16 S_Y,u16 E_X,u16 E_Y)
{

  LCD_RS0_WR( 0x2a);
  LCD_RS1_WR( (u8)((S_X>>8)) );
  LCD_RS1_WR( (u8)(S_X   )   );
  LCD_RS1_WR( (u8)((E_X-1)>>8) );
  LCD_RS1_WR( (u8)(E_X-1)    );

    //Set Y Address
  LCD_RS0_WR( 0x2b);
  LCD_RS1_WR( (u8)((S_Y>>8))   );
  LCD_RS1_WR( (u8)(S_Y   )   );
  LCD_RS1_WR( (u8)((E_Y-1)>>8) );
  LCD_RS1_WR( (u8)(E_Y-1)    );
}
*/






// #define SWAP(a, b) do { typeof(a) temp = a; a = b; b = temp; } while (0)

// #define SWAP(t, a, b) do { t temp = a; a = b; b = temp; } while (0)
// #define swap(a,b) SWAP(a,b)


/*
  int k, l;

for(k=y1;k<y2;k++)

   {
    for(l=x1;l<x2;l++)
     {

    LCD_Write_DATA(   c  ) ;
    // LCD_RS1_WR(color);
    }
   }
*/


#if 0
void LCD_fillRect(uint16_t x1,  uint16_t y1,uint16_t x2,  uint16_t y2, uint16_t c )
{

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
    // uint16_t r = 0x00, g = 0x0, b = 0x0;    // black
    // uint16_t r = 0xff, g = 0xff, b = 0xff;    // white
    // uint16_t r = 0x0, g = 0x0, b = 0xff;    // blue
/*
    UNUSED(r);
    UNUSED(g);
    UNUSED(b);
*/
    // rgb 565
    // LCD_Write_DATA(   (r & 0x1f ) << 11 /* | (g & 0x3f << 5) */  /*| (b & 0x1f )*/ ) ; // bright red. good.
    // LCD_Write_DATA(   0xffff  ) ; // works.

    // LCD_Write_DATA(    ((g & 0x3f) << 5)  ) ;   // bright green ???

    // LCD_Write_DATA(   (r & 0x1f ) << 11 | (g & 0x3f) << 5 | (b & 0x1f)  ) ;

    LCD_Write_DATA(   c  ) ;

  }
}
#endif

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

