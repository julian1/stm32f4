

// #include <stdint.h>  // uint32_t


// extern void spi_cs2_strobe_assert( uint32_t spi);
extern void spi1_port_cs2_disable(void);
extern void spi1_port_cs2_enable(void);
extern void spi1_port_cs1_disable(void);
extern void spi1_port_cs1_enable(void);




extern void spi1_port_cs1_setup(void);
extern void spi1_port_cs2_setup(void);
extern void spi1_port_cs2_gpio_setup(void);
extern void spi1_port_cs1_cs2_gpio_setup(void);


extern void spi1_port_interupt_gpio_setup( void (*pfunc_)(void *),  void *ctx  );

