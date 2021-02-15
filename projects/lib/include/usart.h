


#include "FreeRTOS.h"
#include "queue.h"


// TODO - we need to expose these unfortunatley.... to separate out the serial task
extern QueueHandle_t usart_txq;
extern QueueHandle_t usart_rxq;



extern void usart_setup(void);


extern void usart_task(void *args __attribute__((unused))) ;


