


#include <stdio.h>    // printf, scanf
#include <assert.h>

#include <libopencm3/stm32/gpio.h>

#include <lib2/util.h>      // UNUSED, ARRAY_SIZE

#include <device/vfd.h>








static void msleep( uint32_t delay, volatile uint32_t *system_millis)
{

  // works for system_millis integer wrap around
  // could be a do/while block.
  uint32_t start = *system_millis;
  while (true) {
    uint32_t elapsed = *system_millis - start;
    if(elapsed > delay)
      break;

    // yield()
  };

}



void vfd_dev_gpio_init( void )
{
    // TODO rename vfd_gpio_init()
  printf("vfd_dev_gpio_init()\n");

  // ikon rst. feb 2026
  // feb 2026.  device init.
  gpio_set( GPIOB, GPIO8);   // keep high - to avoid supirious seting.
  gpio_mode_setup(  GPIOB, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, GPIO8 );

}


/*
  - read/write  are for reading writing.
  - CD command/data - are whether the operation is a command. or if its data.  this is orthogonal. to read/write.
      p16/ p15.

  5f   01011111     to clear.   yes. top 4 bytes match clear in command function.

  62H  01100010     in loop.  then n [ 0 - 8] as second byte.  and ff as data.
                    it's the 3 byte -  display area set.
                    first byte matches 01100010
                    second byte 0000 and n.
                    third byte  is 0xff.
*/


void vfd_dev_init(  volatile uint32_t *system_millis)
{
  // see s8 manual.  everything must be initialized with gram

  printf("vfd_init()\n");

  // feb 2026.  delegate to device.
  // perform reset hold reset pin lo for 2ms.
  gpio_clear ( GPIOB, GPIO8);
  msleep( 2,  system_millis);     // seems to be 3ms. not 2?
  gpio_set( GPIOB, GPIO8);


  // display clear - it is part of sequence in s8. so may be required

  vfd_write_cmd( 0x5f);

  msleep( 1,  system_millis);

  for(unsigned i = 0; i < 8; ++i) {

    vfd_write_cmd( 0x62 );
    // vfd_write_cmd( 0x00 );   //
    vfd_write_cmd( i );   //
    vfd_write_data( 0xff );
  }

  // need to turn display on.
  // EXTR.  quite easy to turn on the brightness.


    uint8_t l0 = 1 << 2;    // layer 0
 //   uint8_t l1 = 1 << 3;    // layer 1

    uint8_t gs = 1 << 6;    // gs area off/on
     uint8_t grv = 1 << 4;   // reverse or normal   EXTR.  should flip this
    UNUSED(grv);

//    uint8_t and_ = 1 << 3;
//    uint8_t exor = 1 << 2;

  uint8_t cmd[] = { 0b00100000, 0 };

   cmd[0] |= l0;
   cmd[1] |= gs;

//    cmd[1] |= grv;      // inverse
                      // OK. inverse actually worked.


    // if (layer0) cmd[0] |= l0;
    // if (layer1) cmd[0] |= l1;

    // if (on) cmd[1] |= gs;
    // if (inverse) cmd[1] |= grv;


    vfd_write_cmd( cmd[0] );
    vfd_write_cmd( cmd[1] );


  // this seems to turn on two dots in top-left?
}





/*
REFS
  https://github.com/rhalkyard/Noritake_GU800_GFX

  https://github.com/rhalkyard/Noritake_GU800_GFX/blob/master/src/GU800_GFX.cpp

  Fonts used by another noritake project.
    https://github.com/ryomuk/gu3000/tree/main/src/fonts

  see begin . // Hold /RESET low for 2ms to initialise display (only strictly required on
      // a cold poweron)
      digitalWrite(resetPin, LOW);
      delay(2);
      digitalWrite(resetPin, HIGH);

  see the void GU800::display() {
  for copying buffer to the display.
  it
    this->addrMode(true, false);    // Autoincrement X address, hold Y address

  loop the y rows.
    set x to 0.
    set y to y.
    write spi
    then flip the page.

    Ok. so it just draws horizontal or vertical lines.


#define GU800_WIDTH 128
#define GU800_HEIGHT 64

#define GU800_HEIGHTBYTES (GU800_HEIGHT / 8)

*/



/*
javascript.
(1<<16).toString(2)
"10000000000000000"

0x60000000.toString(2)
"1100000000000000000000000000000"

(0xffffffff ).toString(2)
"11111111111111111111111111111111"

----------

https://www.eevblog.com/forum/microcontrollers/stm3f407-ili9341-fsmc/

#define FMC_REGION ((uint32_t)0x60000000) // Bank1 FMC NOR/PSRAM

#define CommandAccess FMC_REGION //write to this address as a command
//#define DataAccess (FMC_REGION + 0x20000) // FSMC_A16
#define DataAccess (FMC_REGION + 0x40000) // FSMC_A17 //write to this address to data



So it matches.  but not quite sure.
  (1<<(16 +1)).toString(16) == "20000"


/home/me/devel/stm32f4//lib/libopencm3/include/libopencm3/stm32/f4/memorymap.h:152: note: this is the location of the previous definition
  152 | #define FMC_BASE                        (PERIPH_BASE_AHB3 + 0x40000000U)
      |
     |
/home/me/devel/stm32f4//lib/libopencm3/include/libopencm3/stm32/f4/memorymap.h:34: note: this is the location of the previous definition
   34 | #define PERIPH_BASE_AHB3                0x60000000U


  0x60000000 + 0x40000000

  ( 0x60000000 + 0x40000000 ).toString(16)
  "a0000000"
*/


/*
  FMC_BASE is defined as ( 0x60000000 + 0x40000000 ).toString(16)
  "a0000000"

  eg.
  152 | #define FMC_BASE                        (PERIPH_BASE_AHB3 + 0x40000000U)
   34 | #define PERIPH_BASE_AHB3                0x60000000U
*/


  /*

    For VFD data is read on the rising edge of the WR.

    with divider == 1. is is easier to see the address is already well asserted on WR rising edge. before CS.

    actually address is asserted before the CS. which is *VERY* nice. means
    logic should work.


  */


#if 0
  {
  /*
    *((volatile uint16_t *)  (FMC_MY_BASE |  FMC_A16)) = 1 ;      // assert A16, A17 lo.
    *((volatile uint16_t *)  (FMC_MY_BASE |  FMC_A17)) = 0 ;      // assert A17, A16 lo.
  */

    // use FMC_A18 to write VFD.
    *((volatile uint16_t *)  (FMC_MY_BASE |  FMC_A18)) = 0b10101010;
    *((volatile uint16_t *)  (FMC_MY_BASE |  FMC_A18)) = 0b01010101;


  }

#endif


