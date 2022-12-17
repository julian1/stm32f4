
///////////////

#include <stdint.h>  // uint32_t

extern void spi_strobe_assert( uint32_t spi);

extern void spi1_port_setup(void);
extern void spi1_port_setup2(void);


void spi1_interupt_gpio_setup( void (*pfunc_)(void *),  void *ctx  );

