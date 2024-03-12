
DEPRECATED. use usart.h isntead.

#pragma once

#ifdef __cplusplus
extern "C" {
#endif



typedef struct CBuf CBuf;


void usart1_setup_portA(void);
void usart1_setup_portB(void);



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


