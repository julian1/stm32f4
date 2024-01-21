

// #include <stdint.h>  // uint32_t


// manual toggle cs pins.  do we still use this?
extern void spi1_port_cs2_disable(void);
extern void spi1_port_cs2_enable(void);
extern void spi1_port_cs1_disable(void);
extern void spi1_port_cs1_enable(void);




extern void spi1_port_cs1_setup(void);
extern void spi1_port_cs2_setup(void);
extern void spi1_port_cs2_gpio_setup(void);
extern void spi1_port_cs1_cs2_gpio_setup(void);


extern void spi1_port_interupt_setup( void (*pfunc_)(void *),  void *ctx  );

