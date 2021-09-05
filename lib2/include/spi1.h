
///////////////
extern void spi1_port_setup(void);
extern void spi1_special_gpio_setup(void);




extern void spi_special_flag_set(uint32_t spi);
extern void spi_special_flag_clear(uint32_t spi);


void spi1_interupt_gpio_setup( void (*pfunc_)(void *),  void *ctx  );

