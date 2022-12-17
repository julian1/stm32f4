
///////////////

void spi1_cs2_set(void);
void spi1_cs2_clear(void);

extern void spi1_port_setup(void);
extern void spi1_port_setup2(void);


void spi1_interupt_gpio_setup( void (*pfunc_)(void *),  void *ctx  );

