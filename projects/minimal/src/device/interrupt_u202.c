

/* could move to u202.c
  since u202 specific
*/

#include <libopencm3/stm32/gpio.h>
#include <libopencm3/cm3/nvic.h>
#include <libopencm3/stm32/exti.h>


#include <peripheral/interrupt.h>
#include <device/interrupt_u202.h>


#include <string.h> // memset


#define UNUSED(x) ((void)(x))



static interrupt_t   x;


void exti3_isr(void) // called by runtime
{
  /*
    OK. bizarre. resetting immediately, prevents being called a second time
  */
  exti_reset_request( EXTI3);


  if(x.handler) {
    x.handler( x.ctx, &x );
  }

}


static void setup( interrupt_t *ctx)
{
  UNUSED(ctx);

  gpio_mode_setup(GPIOA, GPIO_MODE_INPUT, GPIO_PUPD_NONE, GPIO3);

  // ie. use exti2 for pa2, exti3 for pa3
  nvic_enable_irq(NVIC_EXTI3_IRQ);
  // nvic_set_priority(NVIC_EXTI3_IRQ, 5 );

  exti_select_source(EXTI3, GPIOA);
  exti_set_trigger(EXTI3 , EXTI_TRIGGER_RISING );         // JA. nov 1. 2023. to make consistent with _valid signal hi.
  exti_enable_request(EXTI3);
}


interrupt_t * interrupt_u202_create()
{
  memset(&x, 0, sizeof( interrupt_t));

  x.setup = setup;
  return &x;
}




/*
../../lib/libopencm3/include/libopencm3/stm32/common/exti_common_all.h:#define EXTI3                            (1 << 3)
  ../../lib/libopencm3/include/libopencm3/stm32/l4/nvic.h:#define NVIC_EXTI3_IRQ 9
*/


