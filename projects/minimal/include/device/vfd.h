

#pragma once

/*

  low level device specific code. placed in header. for speed.

  feb 2026.

  low-level vfd instance specific state that touches the mcu hardward.
  eg. the FSMC_A18. and the PB8 for reset.

  for low-level funcs should be fast.
  avoid call indirection from using an opaque function pointer
*/




#include <assert.h>



/* keep in header
no reason to directly associate with fsmc.
*/
#define FMC_MY_BASE 0x60000000
#define FMC_A16 (1<<(16+1))
#define FMC_A17 (1<<(17+1))
#define FMC_A18 (1<<(18+1))
#define FMC_A19 (1<<(19+1))


// place in header for speed

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
  // because no control over dir of level shifter
  assert( 0);
  return *((volatile uint16_t *)  (FMC_MY_BASE |  FMC_A18 ));
}


void vfd_init_gpio( void );

void vfd_init(  volatile uint32_t *system_millis);




