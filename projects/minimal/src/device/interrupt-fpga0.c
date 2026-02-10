

#include <libopencm3/stm32/gpio.h>
#include <libopencm3/cm3/nvic.h>
#include <libopencm3/stm32/exti.h>


// #include <stdio.h>
#include <stdlib.h> // malloc
#include <string.h> // memset
#include <assert.h>


#include <peripheral/interrupt.h>
#include <device/interrupt-fpga0.h>


#define UNUSED(x) ((void)(x))


#define INT_MAGIC   789




typedef struct interrupt2_t interrupt2_t;

struct interrupt2_t
{
  // this is  a lot of work - just to set the ctx, and callee.

  interrupt_t  ;   // anonymous.  for composition.

  interupt_handler_t  handler;
  void *ctx;
};


static interrupt2_t   *x = NULL;


void exti3_isr(void) // called by runtime
{
  /*
    context is not atomic
    OK. bizarre. resetting immediately, prevents being called a second time
  */
  exti_reset_request( EXTI3);


  assert(x);
  assert(x->magic == INT_MAGIC);

  if( x->handler) {

    // printf("x       is %p\n", x );
    // printf("x->ctx  is %p\n", x->ctx);
    // printf("f       is %p\n", x->handler);

    x->handler( x->ctx, NULL );
  }
}



static void setup( interrupt2_t *i)
{
  UNUSED(i);

  gpio_mode_setup(GPIOA, GPIO_MODE_INPUT, GPIO_PUPD_NONE, GPIO3);

  // ie. use exti2 for pa2, exti3 for pa3
  nvic_enable_irq(NVIC_EXTI3_IRQ);
  // nvic_set_priority(NVIC_EXTI3_IRQ, 5 );

  exti_select_source(EXTI3, GPIOA);
  exti_set_trigger(EXTI3 , EXTI_TRIGGER_RISING );         // JA. nov 1. 2023. to make consistent with _valid signal hi.
  exti_enable_request(EXTI3);
}



static void set_handler( interrupt2_t *i, void *ctx, interupt_handler_t handler)
{
  assert(i->magic == INT_MAGIC);
  i->ctx = ctx;
  i->handler = handler;
}




interrupt_t * fpga0_interrupt_create()
{
  interrupt2_t *i = malloc(sizeof(  interrupt2_t ));
  assert(i);
  memset(i, 0, sizeof(interrupt2_t));

  i->magic        = INT_MAGIC;
  i->setup        = (void (*)( interrupt_t *))  setup;
  i->set_handler  = (void (*)( interrupt_t *, void *, interupt_handler_t)) set_handler;

  x = i;

  return i;
}




/*
../../lib/libopencm3/include/libopencm3/stm32/common/exti_common_all.h:#define EXTI3                            (1 << 3)
  ../../lib/libopencm3/include/libopencm3/stm32/l4/nvic.h:#define NVIC_EXTI3_IRQ 9
*/


