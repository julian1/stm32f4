
#include <stdint.h> // uint32_t

struct FBuf;

/*
  Actually. I think we should just about do this.
  likewise for dac.
  saves having to pull global defines from everywhere.
  //////////
  prefixing all the calls with ads131a04 becomes a bit messy.

  its actually only two vars.

  struct {
    uint32_t  spi;
    uint8_t   reg;
  }
 
  io_set(a->reg, ADC_RST); 
  io_clear(a->reg, ADC_RST); 

  could also just pass to the call.
 
  same for dac etc.
  simpler functions would be good.
*/
extern void spi_adc_setup(uint32_t spi /*, uint8_t reg */);

unsigned adc_init( uint32_t spi);

// void adc_setup_spi( void );

// void adc_exti_setup(FBuf *buffer1);

