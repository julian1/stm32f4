

#pragma once

#include <stddef.h> // size_t

/* we don't need a separate structure for 4094

  OE.  is handled  directly - with an fpga register.

  cs==strobe.  with inverting handled by mcu/fpga/discrete logic.
*/



typedef struct spi_t  spi_t ;


// rename spi_4094_write8()
uint8_t spi_4094_reg_write( spi_t *spi, uint8_t v);

uint32_t spi_4094_reg_write_n( spi_t *spi, const unsigned char *s, size_t n);








#if 0

typedef struct spi_4094_t  spi_4094_t ;
struct spi_4094_t
{


  spi_t  ;   // anonymous.  for composition.

  // magic, type, size.
  // uint32_t  spi;


  void (*config)(spi_4094_t *);      // call before use.

  //  pin assignment varies between instance. so give explitic functions
  void (*strobe)(spi_4094_t *, uint8_t );
  // void (*oe)(spi_4094_t *, uint8_t );  controlled by fpag. register.
} ;

#endif



// void spi_mux_4094(uint32_t spi );


