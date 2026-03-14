

#pragma once

/*
  peripheral.  an abstraction
*/




#include <assert.h>
#include <stdbool.h>


#define VFD_MAGIC 72972619

/*
    make sure support fast inlining and avoid virtual call overhead
*/


typedef struct vfd_t
{
  uint32_t  magic;

  uint32_t  fmc_addr;     // FMC_MY_BASE |  FMC_A18
  uint32_t  fmc_addr_cd;       // command/data FMC_A16


  // remmember the bits p
  // in pix
  uint32_t  width;
  uint32_t  height;   // hieght in bytes is / 8.

  //
  bool page;

} vfd_t;



static inline void vfd_write_cmd( vfd_t *vfd, uint8_t v)
{
  // higher byte is just ignored.
  *((volatile uint16_t *)  (vfd->fmc_addr | vfd->fmc_addr_cd)) = v ;
}

static inline void vfd_write_data( vfd_t *vfd, uint8_t v)
{
  *((volatile uint16_t *)  (vfd->fmc_addr )) = v ;
}

static inline uint8_t vfd_read_data( vfd_t *vfd)
{
  // not supported
  // without control over dir pin of the level shifter
  assert( 0);
  return *((volatile uint16_t *)  (vfd->fmc_addr ));
}



static inline void setx( vfd_t *vfd, uint8_t xpix )
{
  vfd_write_cmd( vfd, 0b01100100);          // write setx
  vfd_write_cmd( vfd, xpix );
}

static inline void sety( vfd_t *vfd, uint8_t ychar )
{
  vfd_write_cmd( vfd, 0b01100000  );          // data write sety
  vfd_write_cmd( vfd, ychar  );
}


static inline void setincx( vfd_t *vfd )
{

  vfd_write_cmd( vfd,  0b10000100 );          // set igx  to increment x,  bits are vertical. odd.
}











#if 0
// inline low-level funcs for speed
// A16 is command/data .
// A18 to select VFD.

static inline void vfd_write_cmd( uint8_t v)
{
  // higher byte is just ignored.
  *((volatile uint16_t *)  (FMC_MY_BASE |  FMC_A18 | FMC_A16)) = v ;
}

static inline void vfd_write_data( uint8_t v)
{
  *((volatile uint16_t *)  (FMC_MY_BASE |  FMC_A18 )) = v ;
}

static inline uint8_t vfd_read_data( void)
{
  // not supported
  // without control over dir pin of the level shifter
  assert( 0);
  return *((volatile uint16_t *)  (FMC_MY_BASE |  FMC_A18 ));
}
#endif


