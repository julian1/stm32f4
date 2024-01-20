
#ifndef XPT2046_H
#define XPT2046_H

#ifdef __cplusplus
extern "C" {
#endif


void xpt2046_gpio_setup(void);



void xpt2046_spi_port_setup(void);
void xpt2046_spi_setup( uint32_t spi);

// uint32_t spi_xfer_16(uint32_t spi, uint16_t val);
// uint32_t spi_xfer_24(uint32_t spi, uint32_t val);



void xpt2046_read( void ) ;
void xpt2046_reset( uint32_t spi) ;

#define XPT2046_SPI  SPI2


#endif // XPT2046_H

#ifdef __cplusplus
}
#endif



