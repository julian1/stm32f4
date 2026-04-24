
#pragma once


#ifdef __cplusplus
extern "C" {
#endif



typedef struct cbuf_t cbuf_t;

void usart1_configure_port_A(void);
void usart1_configure_port_B(void);



//  sets up interupt context
//


void usart_configure( uint32_t usart );
void usart1_set_buffers( cbuf_t *input, cbuf_t *output);



#ifdef __cplusplus
}
#endif


