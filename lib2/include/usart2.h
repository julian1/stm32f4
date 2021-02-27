
#include <stddef.h> // size_t

#include "buffer.h"






void usart_setup_gpio(void);
void usart_setup_gpio_portB(void);

void usart_setup(CBuf *output, CBuf *input);

void usart_output_update(void);
void usart_input_update(void);
void usart_sync_flush(void);


