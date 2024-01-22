

// #include <stdint.h>  // uint32_t




extern void spi1_port_cs1_setup(void);
extern void spi1_port_cs2_setup(void);
// extern void spi1_port_cs2_gpio_setup(void);
// extern void spi1_port_cs1_cs2_gpio_setup(void);


// not sure if really needed - 
// void spi1_port_cs1_cs2_manual_setup(void);


/*
  should use these functions instead.

   void   spi_set_nss_high (uint32_t spi)
  SPI Set the Software NSS Signal High. More...
 
void  spi_set_nss_low (uint32_t spi)
  SPI Set the Software NSS Signal Low. More...

*/




void spi1_port_cs1_gpio_setup(void);

// manual toggle cs pins.  do we still use this?
extern void spi1_port_cs1_clear(void);
extern void spi1_port_cs1_set(void);


// extern void spi1_port_cs2_clear(void);
// extern void spi1_port_cs2_set(void);


extern void spi1_port_interupt_setup( void (*pfunc_)(void *),  void *ctx  );

