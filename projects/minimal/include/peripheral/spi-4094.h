

#pragma once

#include <stddef.h> // size_t

/* spi_t is sufficient structure for 4094,

  OE.  is handled  directly - with an fpga register.

  cs==strobe.  with invert handled by mcu/fpga/discrete logic.
  sequencing strobe handdled in write
*/



typedef struct spi_t  spi_t ;


uint32_t spi_4094_write_n( spi_t *spi, const unsigned char *s, size_t n);





