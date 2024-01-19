
#pragma once


#ifdef __cplusplus
extern "C" {
#endif



typedef struct CBuf CBuf;

// TODO change name usart1_portA_setup()
void usart1_setup_gpio_portA(void);
void usart1_setup_gpio_portB(void);



//   TODO maybe use separate functions to set the buffers/queues
void usart1_set_buffers(CBuf *input, CBuf *output);


/*
  non-blocking.
  use only for printf()/puts()  etc
  to re-enable tx interupt
*/
void usart1_enable_output_interupt(void);

void usart1_flush(void);


#ifdef __cplusplus
}
#endif


