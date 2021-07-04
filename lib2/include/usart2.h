

typedef struct CBuf CBuf;


void usart_setup_gpio_portA(void);
void usart_setup_gpio_portB(void);


/*
  TODO
  use separate functions to set the buffers/queues
*/
void usart_setup(CBuf *input, CBuf *output);


/*
  use only for printf()/puts()  etc
  to re-enable tx interupt
*/
void usart_output_update(void);


