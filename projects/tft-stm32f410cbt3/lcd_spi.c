/*
  we want a vertical scroll
  and organize the code - so it's checked in.

  https://www.eevblog.com/forum/projects/ili9341-tft-display-and-hardware-scrolling/

*/
/*
  Not sure, how useful using hardware NSS with an external PU resistor is.
  Since we must already toggle the command/data pin, which will require waiting for writes to complete etc.
*/
/*
  spi 9341  2.8 with touch

  -- OK. needs 5V - for backlight to be brighterjj
  -- i think.
    think the board LED pin controls mosfets that then turn the backlight on/off.

  just connecting led/BL to +3.3V Vcc and backlight turns on.
    probably a digital ctrl.  but maybe drawing current.
    so control with gpio later.

  looks like a couple of fets on the back of pcb. but maybe 5V power supplies.
  no tranceivers


  ------------

  - datasheet ILI9341
    https://www.displayfuture.com/Display/datasheet/controller/ILI9341.pdf


  libopencm3 example uses ILI9341  in spi mode.
    https://github.com/libopencm3/libopencm3-examples/blob/master/examples/stm32/f4/stm32f429i-discovery/lcd-dma/lcd-spi.c

  simple,
    https://github.com/adafruit/Adafruit_ILI9341

  primitives
    https://github.com/adafruit/Adafruit-GFX-Library/blob/master/Adafruit_SPILCD.cpp

  - gfx library - higher level
      https://github.com/adafruit/Adafruit-GFX-Library/blob/master/Adafruit_GFX.cpp

    https://controllerstech.com/interface-tft-display-with-stm32/

  -------
  agg antigrain.
    to use - would need buffer in local memory - because must know background in order to blend pixel.
    so even if use spi - to write the extents. still need backgroun buffer.
    need to modify font to handle - fix paths.

  8 bit parallel, has init sequence.
    https://github.com/sammyizimmy/ili9341/blob/master/ili9341.c

  optimised. 70 forks. uses spi.
    https://github.com/PaulStoffregen/ILI9341_t3
  --
*/



#include <libopencm3/stm32/spi.h>
// #include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>


#include "lcd_spi.h"

/*
  spi 1 AF5
    MOSI == PA7 == GPIO7    DAC SDI pin 4
    MISO == PA6 == GPIO6    DAC SDO pin 5
*/
// change name LCD_SPI_CS, LCD_SPI_CLK etc.
#define LCD_SPI       SPI1
#define LCD_SPI_PORT  GPIOA


#define LCD_SPI_AF    GPIO_AF5  // TODO USE this...

#define LCD_CS        GPIO4
#define LCD_CLK       GPIO5
#define LCD_MOSI      GPIO7
#define LCD_MISO      GPIO6


// mar25 2021.
#if 0
#define LCD_CTL_PORT  GPIOB
// PB2, is BOOT1
// PB3, is SDO
#define LCD_CTL_RST   GPIO4
#define LCD_CTL_DC    GPIO5
#define LCD_CTL_LED   GPIO6
#endif

// mar25 2021
#define LCD_CTL_PORT  GPIOA
#define LCD_CTL_RST   GPIO11
#define LCD_CTL_DC    GPIO12
// #define LCD_CTL_LED   GPIO6








void lcd_spi_setup( void )
{
  // uart_printf("dac setup spi\n\r");

  // TODO change GPIOA to LCD_SPI_PORT
  // albeit, should probabaly also do LCD_PORT_AF
  // spi alternate function 5
  gpio_mode_setup(LCD_SPI_PORT, GPIO_MODE_AF, GPIO_PUPD_NONE, LCD_CLK | LCD_MOSI | LCD_MISO | LCD_CS );

  // OK.. THIS MADE SPI WORK AGAIN....
  // need harder edges for signal integrity. or else different speed just helps suppress parasitic components
  // see, https://www.eevblog.com/forum/microcontrollers/libopencm3-stm32l100rc-discovery-and-spi-issues/

  
  // JA NSS
  // gpio_set_output_options(GPIOA, GPIO_OTYPE_PP, GPIO_OSPEED_25MHZ, LCD_CLK | LCD_MOSI | LCD_MISO );
  gpio_set_output_options(GPIOA, GPIO_OTYPE_PP, GPIO_OSPEED_25MHZ, LCD_CLK | LCD_MOSI | LCD_MISO | LCD_CS );

  // WARNING - CAREFULl THIS IS SPECFICIC to GPIOA....
  // JA NSS
  // gpio_set_af(GPIOA, GPIO_AF5, LCD_CLK | LCD_MOSI | LCD_MISO );
  gpio_set_af(GPIOA, GPIO_AF5, LCD_CLK | LCD_MOSI | LCD_MISO | LCD_CS );


  // rcc_periph_clock_enable(RCC_SPI1);
  spi_init_master(LCD_SPI,
    SPI_CR1_BAUDRATE_FPCLK_DIV_4,
    // SPI_CR1_BAUDRATE_FPCLK_DIV_256,
    SPI_CR1_CPOL_CLK_TO_0_WHEN_IDLE,
    // SPI_CR1_CPOL_CLK_TO_1_WHEN_IDLE , // possible we want clock high instead... no doesn't work
    // SPI_CR1_CPHA_CLK_TRANSITION_2,    // 1 == rising edge, 2 == falling edge.
    SPI_CR1_CPHA_CLK_TRANSITION_1,    // 1 == rising edge, 2 == falling edge.
    SPI_CR1_DFF_8BIT,
    SPI_CR1_MSBFIRST
    // SPI_CR1_LSBFIRST
  );

  
  // spi_enable_ss_output(LCD_SPI);
  // spi_enable(LCD_SPI);

  // JA NSS
  spi_disable_software_slave_management(LCD_SPI);
  spi_enable_ss_output(LCD_SPI);



  // make spi cs regular gpio
  // gpio_mode_setup(LCD_SPI_PORT, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, LCD_CS );


  // set up gpio
  gpio_mode_setup(LCD_CTL_PORT, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, LCD_CTL_RST | LCD_CTL_DC /*| LCD_CTL_LED */);

  // uart_printf("dac setup spi done\n\r");
}






/*
  read/write != command/data

  p33. p35.

  We use 4 wire. eg. we use D/CX
    If the D/CX bit is “low”, the transmission byte is interpreted as a command byte.
    If the D/CX bit is “high”, the transmission byte is stored as the
                display data RAM (Memory write command), or command register as parameter.

  The 4-line serial mode consists of the
    Data/Command selection input (D/CX),
    chip enable input (CSX),
    the serial clock input (SCL)
    and serial data Input/Output (SDA or SDI/SDO) for data transmission.

  Any instruction can be sent in any order to ILI9341 and the MSB is transmitted first.

  The serial interface is initialized when CSX is high status. In this state,
  SCL clock pulse and SDA data are no effect. A falling edge on CSX enables the
  serial interface and indicates the start of data transmission.

  Host processor drives the CSX pin to low and starts by setting the D/CX bit on
  SDA. The bit is read by ILI9341 on the first rising edge of SCL signal. On the
  next falling edge of SCL, the MSB data bit (D7) is set on SDA by the host. On
  the next falling edge of SCL, the next bit (D6) is set on SDA. If the optional
  D/CX signal is used, a byte is eight read cycle width.
  ---------

  read is defined on p38.
  lookks like supports an 8 bit read.

*/


static inline void spi_wait_until_not_busy(uint32_t spi)
{
  /*
    see,
    http://libopencm3.org/docs/latest/stm32f4/html/spi__common__all_8c_source.html#l00194
  */
  /* Wait until not busy */
  while (SPI_SR(spi) & SPI_SR_BSY);
}

static inline void spi_wait_for_transfer_finish(uint32_t spi)
{
   /* Wait for transfer finished. */
   while (!(SPI_SR(spi) & SPI_SR_TXE));


}


static inline void wait_for_transfer_finish(void)
{
/*
  see example, that also uses a loop.
  https://github.com/libopencm3/libopencm3-examples/blob/master/examples/stm32/f4/stm32f429i-discovery/lcd-dma/lcd-spi.c
*/
  // so we actually need both of these,

  spi_wait_for_transfer_finish(LCD_SPI);
  // nop_sleep(15);   // 9 doesn't work. 10 does... weird margin

  spi_wait_until_not_busy(LCD_SPI);

}



void lcd_spi_turn_on_backlight( void )
{
/*
  // turn led/backlight on.
  gpio_set( LCD_CTL_PORT, LCD_CTL_LED);    // high
*/
}

void lcd_spi_assert_rst(void)
{
  gpio_clear(LCD_CTL_PORT, LCD_CTL_RST);
}


void lcd_spi_deassert_rst(void)
{
  gpio_set( LCD_CTL_PORT, LCD_CTL_RST);
}



// ok. we're not really using this correctly
// should be disable on each major write.


// beginning stm32 uses, spi_enable()    for nss/cs.

void lcd_spi_enable(void)
{
  // assert chip select, with low
  // gpio_clear(LCD_SPI_PORT, LCD_CS);       // cs is spi port. this is hard.

  spi_enable(LCD_SPI);
}


void lcd_spi_disable(void)
{
  // gpio_set(LCD_SPI_PORT, LCD_CS);       // cs is spi port. this is hard.

  spi_disable(LCD_SPI);
}




//////////////////////////

static void lcd_spi_assert_command(void )
{
  //wait_for_transfer_finish();
  gpio_clear( LCD_CTL_PORT, LCD_CTL_DC);    // low == command
}



static void lcd_spi_assert_data(void )
{
  //wait_for_transfer_finish();
  gpio_set( LCD_CTL_PORT, LCD_CTL_DC);    // high == data
}


// probably should be inlined...
static void lcd_spi_send8( uint8_t x )
{
  spi_send( LCD_SPI, x );
}






/*
  EXTREME
    // OK. think issue may be that lcd_spi_send8 returns before it has finished sending bytes.
    // so we have to find another call to block until finished.
    // or check register in a loop ourselves.
    // not sure.
    // eg. it returns early. and only blocks if we try to write another byte while still sending spi data.
    // so if we try to fiddle with the command/data register  in the middle of sending it screws up.

*/


void lcd_send_command(uint8_t command, const uint8_t *dataBytes, uint32_t numDataBytes)
{

  wait_for_transfer_finish();
  lcd_spi_assert_command();
  lcd_spi_send8(command);

  wait_for_transfer_finish();
  lcd_spi_assert_data();
  for(unsigned i = 0; i < numDataBytes; ++i) {
    lcd_spi_send8(dataBytes[ i ]);
  }

}


void lcd_send_command_repeat(uint8_t command, uint16_t x, uint32_t n )
{
  // n is *not* bytes, but number of 16bit elements

  wait_for_transfer_finish();
  lcd_spi_assert_command();
  lcd_spi_send8(command);

  wait_for_transfer_finish();
  lcd_spi_assert_data();
  for(unsigned i = 0; i < n; ++i) {
    lcd_spi_send8( x >> 8 );
    lcd_spi_send8( x & 0xFF );
  }

}


