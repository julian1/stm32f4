
#pragma once


#ifdef __cplusplus
extern "C" {
#endif



typedef struct CBuf CBuf;


void usart_setup_gpio_portA(void);
void usart_setup_gpio_portB(void);



//   TODO maybe use separate functions to set the buffers/queues
void usart_set_buffers(CBuf *input, CBuf *output);


/*
  non-blocking.
  use only for printf()/puts()  etc
  to re-enable tx interupt
*/
void usart_output_update(void);

void usart_flush(void);


#ifdef __cplusplus
}
#endif


