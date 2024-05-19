

#include <libopencm3/stm32/gpio.h>

#include <stdio.h>    // printf, scanf
#include <string.h>   // strcmp, memset
#include <assert.h>
#include <malloc.h> // malloc_stats()
#include <stdlib.h>   // abs()



#include <peripheral/vfd.h>

#include <lib2/util.h>      // msleep


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


// todo move to fsmc header.
#define FMC_MY_BASE 0x60000000
#define FMC_A16 (1<<(16+1))
#define FMC_A17 (1<<(17+1))
#define FMC_A18 (1<<(18+1))
#define FMC_A19 (1<<(19+1))


// A16 is command/data .
// A18 to select VFD.

void vfd_write_cmd( uint8_t v)
{
  // higher byte is just ignored.
  *((volatile uint16_t *)  (FMC_MY_BASE |  FMC_A18 | FMC_A16)) = v ;
}

void vfd_write_data( uint8_t v)
{
  *((volatile uint16_t *)  (FMC_MY_BASE |  FMC_A18 )) = v ;
}

uint8_t vfd_read_data( void)
{
  return *((volatile uint16_t *)  (FMC_MY_BASE |  FMC_A18 ));
}



void vfd_init_gpio( void )
{
  // Ikon reset is PD6

  // led is required....
  gpio_set( GPIOD, GPIO6);
  gpio_mode_setup(  GPIOD, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, GPIO6 );
  // msleep(20);

  // what is frp_out.  is output from vfd.

}

//////////////////////


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


static void vfd_init(  volatile uint32_t *system_millis)
{
  // see s8 manual.  everything must be initialized with gram

  // display clear
  vfd_write_cmd( 0x5f);

  msleep( 1,  system_millis);

  for(unsigned i = 0; i < 8; ++i) { 

    vfd_write_cmd( 0x62 );
    vfd_write_cmd( 0x00 );
    vfd_write_data( 0xff );
  }

  // need to turn display on.
}

/*
  https://github.com/rhalkyard/Noritake_GU800_GFX

  https://github.com/rhalkyard/Noritake_GU800_GFX/blob/master/src/GU800_GFX.cpp

*/

static void vfd_do_something(void)
{

    vfd_write_cmd( 0b01100100);          // data write setx
    vfd_write_data( 0x05 );

    vfd_write_cmd( 0b01100000  );          // data write sety
    vfd_write_data( 0x05 );

}




