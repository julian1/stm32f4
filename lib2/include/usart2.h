
typedef struct CBuf CBuf;



void usart_setup_gpio_portA(void);
void usart_setup_gpio_portB(void);


/*
  TODO Should have separate functions to set the buffers/queues

*/
void usart_setup(CBuf *input, CBuf *output);


/*
  TODO.
  we don't really need this...
  except as internal function for the printf puts confunctroutes....
  Whenever we call usart_printf(), then enable the tx interupt.
*/
void usart_output_update(void);


// void usart_sync_flush(void);


