
#include <stddef.h> // size_t

#include "buffer.h"






void usart_setup_gpio_portA(void);
void usart_setup_gpio_portB(void);

void usart_setup(CBuf *input, CBuf *output);

void usart_output_update(void);
void usart_input_update(void);
void usart_sync_flush(void);


