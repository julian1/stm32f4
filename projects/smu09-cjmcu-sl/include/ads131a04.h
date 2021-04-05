
#include <stdint.h> // uint32_t

struct FBuf;

/*
  Actually. I think we should just about do this.
  likewise for dac.
  saves having to pull global defines from everywhere.

  prefixing all the calls with ads131a04 becomes a bit messy.

  note, that if have more than one. 
  then use a larger struct
  struct {
    uint32_t  spi;
    adc_register
    adc_m0
    adc_m1
    reg_m2
    etc.
  }
  
  same for dac etc.
*/
extern void spi_adc_setup(uint32_t spi);

unsigned adc_reset( uint32_t spi);

// void adc_setup_spi( void );

// void adc_exti_setup(FBuf *buffer1);

