


#include <stdio.h>    // printf, scanf
#include <assert.h>
#include <string.h>


#include <peripheral/vfd.h>




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



void vfd_init( vfd_t *vfd, volatile uint32_t *system_millis)
{

  printf("vfd_init()\n");

  assert( vfd);


  printf("reset\n");

  vfd->vfd_reset( vfd, 0);
  msleep( 2,  system_millis);     // seems to be 3ms. not 2?
  vfd->vfd_reset( vfd, 1);
  msleep( 2,  system_millis);

  printf("done\n");

  //////////////////////////

  // display clear - it is part of sequence in s8. so may be required
  // see s8 manual.  everything must be initialized with gram

  vfd_write_cmd( vfd,  0x5f);

  msleep( 1,  system_millis);

  for(unsigned i = 0; i < 8; ++i) {

    vfd_write_cmd( vfd, 0x62 );
    // vfd_write_cmd( 0x00 );   //
    vfd_write_cmd( vfd,  i );   //
    vfd_write_data( vfd, 0xff );
  }


  /////////////////////////
  // turn display on.

  uint8_t l0 = 1 << 2;    // layer 0
  //   uint8_t l1 = 1 << 3;    // layer 1

  uint8_t gs = 1 << 6;    // gs area off/on
  // uint8_t grv = 1 << 4;   // reverse or normal   EXTR.  should flip this

//    uint8_t and_ = 1 << 3;
//    uint8_t exor = 1 << 2;


  // display on/off command with layer specified
  uint8_t cmd[] = { 0b00100000, 0 };

  cmd[0] |= l0;
  cmd[1] |= gs;

//    cmd[1] |= grv;      // inverse
                      // OK. inverse actually worked.
  vfd_write_cmd( vfd, cmd[0] );
  vfd_write_cmd( vfd, cmd[1] );

}

