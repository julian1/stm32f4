


#include "FreeRTOS.h"
#include "queue.h"


// TODO - consider if should be global, global struct, or returned from usart_setup()
// not sure if should be pointer?
extern QueueHandle_t uart_txq;


// think we have to use an interrupt - so we don't miss anything. eg. we take the character
// in the isr, before it is replaced
extern QueueHandle_t uart_rxq;



extern void usart_setup(void);


extern void uart_task(void *args __attribute__((unused))) ;


