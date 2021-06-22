
#include <stdint.h> // uint32_t
#include <stddef.h> // size_t

struct FBuf;

/*
  prefixing all funcs with ads131a04 is too messy.
  could use a struct, albeit its only two vars we need to deal with.

  struct {
    uint32_t  spi;
    uint8_t   reg;
  }
*/


/*
    TODO. fix the naming.
    eg.

    adc_spi_setup
    adc_spi_init
    or spi_adc_ etc.

*/
extern void spi_adc_setup(uint32_t spi);

int adc_init( uint32_t spi, uint8_t reg);


// OK. this thing is mcu? interrupt handling
// void adc_exti_setup(FBuf *buffer1);

// change spi_adc_do_read()?
// float spi_adc_do_read( uint32_t spi/*, uint8_t reg */);

int32_t spi_adc_do_read( uint32_t spi, float *ar, size_t n); // pass array 4 bytes...

