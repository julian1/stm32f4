


#include <stdio.h>    // printf, scanf
#include <assert.h>
#include <string.h>

#include <libopencm3/stm32/gpio.h>

#include <lib2/util.h>      // UNUSED, ARRAY_SIZE

#include <peripheral/vfd.h>
#include <device/vfd0.h>




/*
  no reason to directly associate, or place with fsmc code.

  (0x60000000 | (1<<(16+1) )).toString(16);
  "60020000"

  (0x60000000 | (1<<(18+1) )).toString(16);
  "60080000"

starting
lcd->reg  0x6001fffe
lcd->ram  0x60020000


*/
#define FMC_MY_BASE 0x60000000
#define FMC_A16 (1<<(16+1))
#define FMC_A17 (1<<(17+1))
#define FMC_A18 (1<<(18+1))
#define FMC_A19 (1<<(19+1))


#define VFD0_MAGIC 1230237






static void vfd_gpio_setup( vfd_t *vfd)
{
  assert( vfd && vfd->magic == VFD0_MAGIC);

  // ikon rst. feb 2026
  // feb 2026.  device init.
  gpio_set( GPIOB, GPIO8);   // keep high - to avoid supirious seting.
  gpio_mode_setup(  GPIOB, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, GPIO8 );

  // need to do IRQ.

}


static void vfd_reset( vfd_t *vfd, bool val )
{
  assert( vfd && vfd->magic == VFD0_MAGIC);

  if( val)
    gpio_set( GPIOB, GPIO8);
  else
    gpio_clear ( GPIOB, GPIO8);
}


static bool vfd_getTear( vfd_t *vfd)
{
  assert( vfd && vfd->magic == VFD0_MAGIC);

  assert( 0);
  return false;
}




void vfd0_init( vfd_t *vfd)
{

  printf("vfd_init()\n");

  memset( vfd, 0, sizeof( vfd_t));

  vfd->magic        = VFD0_MAGIC;      // TODO change to use macro in this file

  vfd->fmc_addr     = FMC_MY_BASE | FMC_A18;
  vfd->fmc_cd       = FMC_A16;

  vfd->width        = 128;    // 16 bytes
  vfd->height_bytes = 8;     //


  vfd->vfd_gpio_setup = vfd_gpio_setup;
  vfd->vfd_getTear    = vfd_getTear;
  vfd->vfd_reset      = vfd_reset;
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





#if 0
static void vfd_clear( volatile uint32_t *system_millis)
{
  // display clear - it is part of sequence in s8. so may be required
  // may be more a reset function.
  // clears mem both pages, so not suitable to screen anyway.

  vfd_write_cmd( 0x5f);

  msleep( 1,  system_millis);

  for(unsigned i = 0; i < 8; ++i) {

    vfd_write_cmd( 0x62 );
    // vfd_write_cmd( 0x00 );   //
    vfd_write_cmd( i );   //
    vfd_write_data( 0xff );
  }
}


// should just have a bool page?
// to control...
// can alternate each draw. - good for test.

static void vfd_display_on_off( bool layer0, bool layer1, bool gram)
{
  /*  constrol which layer is shown
    could be both with operation
    bitfield struct might be easier.
    -----
    consider rename display_flip()  using a page arg.

  */

/*
  uint8_t l0 = 1 << 2;    // layer 0
  uint8_t l1 = 1 << 3;    // layer 1
  uint8_t gs = 1 << 6;    // gs area off/on

*/
    // display on/off command with layer specified
  uint8_t cmd[] = { 0b00100000, 0 };

  if(layer0)
    cmd[0] |= 1 << 2;    // layer 0
  if(layer1)
    cmd[0] |= 1 << 3;    // layer 1

  if(gram)
   cmd[1] |= 1 << 6;    // gs area off/on

  vfd_write_cmd( cmd[0] );
  vfd_write_cmd( cmd[1] );
}

#endif

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


