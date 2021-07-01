
#include <stdint.h> // uint32_t
#include <stddef.h> // size_t


/*
    TODO. fix/make consistent the naming.
    eg.

    adc_spi_setup
    adc_spi_init

    or spi_adc_ etc.
*/
extern void spi_adc_setup(uint32_t spi);

int adc_init(uint32_t spi, uint8_t reg);

int32_t spi_adc_do_read( uint32_t spi, float *ar, size_t n); // pass array 4 bytes...

