


#include <stdarg.h>
#include "usart.h"
#include "task.h"

#include "serial.h"
#include "miniprintf.h" // FIXME.... should not be linked through library, with different compilation parameters...


//////////////////
///////////////////////
// these are higher level functions....
// except requires access to the queue.


void uart_putc_from_isr(char ch) 
// void usart_enqueue_tx_test(uint8_t data ) 
{
  xQueueSendFromISR(uart_txq, &ch, NULL );
}

static void uart_putc(char ch) {
  xQueueSend(uart_txq, &ch, portMAX_DELAY); /* blocks when queue is full */
}


int uart_printf(const char *format,...) {
  // very nice. writes to the uart_putc()/queue so no buffer to overflow
  // COOKED means that a CR is sent after every LF is sent out
  va_list args;
  int rc;

  va_start(args,format);
  rc = mini_vprintf_cooked(uart_putc,format,args);
  va_end(args);
  return rc;
}

// add motor control again - b.  with external power, chose
// needs to be real time - unless on interupt.
// control motor with rotary encoder.
// control motor with prompt.

// ok - motor is a loop. but i think there's an issue..
// cannot just have it running at some subdivision of 1ms. eg. if tick time.
// 1ms tick time is twice as fast as 2ms.

// or we multiplex the gpio signals with external ic logic?

// or we just have a high-priority interupt timer - that works independently of any other tasks?
// if the gpio update is running in an isr. then cannot see a problem.
// we need to code this. though independenty.
// so setting the timer - will set the speed...
// actually timer just has to call the isr.

// OK - EXTREME - we can use the setting of the OC value - to determine the speed.
// eg. in the interupt - we set the next oc value - for the next interrupt.  eg. would just add a delay.
// eg. the pwm-with-interrupts approach.

// actually don't need the delay - just use regular 50% pulse.


char *uart_gets( char *buf, size_t len) {
  // will truncate if overflows...
  char ch;
	char *p = buf;

  for (;;) {
    // Receive char to be TX
    // if( xQueueReceive(uart_rxq,&ch,1) == pdPASS ) {
    if( xQueueReceive(uart_rxq,&ch,500) == pdPASS ) {

      // should we continue consuming - if past buf size...
      // or return immediately without having received '\r' ?

      // screen only ever gives us a '\r'... i think and not a '\n'
      // don't return the \r in the return string...
      if(ch == '\r') {
        // need to decide - if should be returning/including the '\r' and '\n' or not?
        // *p++ = ch;
        *p = 0;
        return buf;
      }

      // OK. this line kills it... but why? because it blows the stack...
      if((size_t) (p - buf) < (len - 1))
        *p++ = ch;


    } else {
     taskYIELD();
    }
  }
  // should never arrive here...
}



#if 0
static void demo_task1(void *args __attribute__((unused))) {

  for (;;) {
    uart_puts("Now this is a message..\n\r");
    uart_puts("  sent via FreeRTOS queues.\n\n\r");
    vTaskDelay(pdMS_TO_TICKS(1000));
  }
}
#endif

/*
  prompt task should perhaps be moved outside of elsewhere...
*/


static char buf[100];

void serial_prompt_task(void *args __attribute__((unused))) {

  // buf size of 10 - seems ok
  // buf size of 50 - ok.
  // OK. buf size of 100. fails and stack exception condition caught - led blinks fast.
  // so we have a margin of somewhere between 50 - 100 bytes or so...
  // ok - at 70 - short input strings are ok - but will stack overflow on longer input strings..
  // char buf[70];

  for (;;) {
    // uart_printf is cooked ... so it should already be giving us stuff...
    uart_printf("\n\r> ");
    uart_gets( buf, 100 );                    // ie. block...
    uart_printf("\n\ryou said '%s'", buf );   // there looks like a bug in the formatting...
                                              // no it's just returning the \n but not the \r...
  }
}


