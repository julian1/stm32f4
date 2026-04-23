

#pragma once

/*
  peripheral.  an abstraction
  This is actually general enough to supp
*/




#include <assert.h>
#include <stdbool.h>


#define VFD_MAGIC 1230237


/*
    make sure support fast inlining and avoid virtual call overhead
*/


typedef struct vfd_t vfd_t;

typedef struct vfd_t
{
  uint32_t  magic;

  uint32_t  fmc_addr;     // FMC_MY_BASE |  FMC_A18
  uint32_t  fmc_cd;       // command/data bit. FMC_A16.   change name vmc_cd_bit.  it is bit in an address not address

  //
  void (*vfd_gpio_port_configure)( vfd_t *);
  bool (*vfd_getTear)( vfd_t *);
  void (*vfd_reset)( vfd_t *, bool val );



  // remmember the bits p
  // in pix
  uint32_t  width;          // pix
  uint32_t  height_bytes;   // hieght in bytes is / 8.

  //
  bool page;

} vfd_t;






void vfd_init( vfd_t *vfd, volatile uint32_t *system_millis);



/*
  typed for 8 bit. bus.
  While tft. is usually 16 bit
*/

static inline void vfd_write_cmd( vfd_t *vfd, uint8_t v)
{
  // higher byte is just ignored.
  *((volatile uint16_t *)  (vfd->fmc_addr | vfd->fmc_cd)) = v ;
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


/*
  vfd specific, but not instance specific are ok here
*/

static inline void vfd_setx( vfd_t *vfd, uint8_t xpix )
{
  vfd_write_cmd( vfd, 0b01100100);          // write setx
  vfd_write_cmd( vfd, xpix );
}

static inline void vfd_sety( vfd_t *vfd, uint8_t ychar )
{
  vfd_write_cmd( vfd, 0b01100000  );          // data write sety
  vfd_write_cmd( vfd, ychar  );
}


static inline void vfd_setincx( vfd_t *vfd )
{
  // set igx  to increment x,  bits are vertical. odd.
  vfd_write_cmd( vfd,  0b10000100 );
}







