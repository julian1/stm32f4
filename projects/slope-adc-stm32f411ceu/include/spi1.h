
///////////////

#if 1

extern void spi1_port_cs1_setup(void);
extern void spi1_port_cs2_setup(void);

void spi1_interupt_port_setup();
void spi1_interupt_handler_set(void (*pfunc)(void *),  void *ctx);

#endif
