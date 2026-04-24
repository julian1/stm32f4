

#include <libopencm3/stm32/gpio.h>
#include <libopencm3/cm3/nvic.h>
#include <libopencm3/stm32/exti.h>


#include <string.h> // memset
#include <assert.h>


#include <peripheral/interrupt.h>
#include <device/interrupt-fpga0.h>


#define UNUSED(x) ((void)(x))

#define FPGA0_MAGIC   78912433



/*
  consider a better way to context pointers for callbacks/handlers.
  a single global table.

    void *glb_nvic_ctx_table[ NVIC_IRQ_COUNT ];

  to register,

    glb_nvic_ctx_table[  NVIC_EXTI3_IRQ ] = (void *) this;

  and index lookup,

    void *ctx = glb_nvic_ctx_tabl[ NVIC_EXTI3_IRQ ];


  But note, negative value/index for systick.
  So would need something different.

  #define NVIC_SYSTICK_IRQ    -1

    The NVIC_SYSTICK_IRQ is defined as a negative number because it represents a
    processor core exception (internal interrupt) rather than a device-specific
    (external) interrupt, according to ARM CMSIS standards.
*/

static interrupt_t   *x = NULL;

void exti3_isr(void) // called by runtime
{
  /*
    context is not atomic
    OK. bizarre. resetting immediately, prevents being called a second time
  */
  exti_reset_request( EXTI3);


  // void *ctx = glb_nvic_ctx_table[ NVIC_EXTI3_IRQ ];

  assert(x);
  assert(x->magic == FPGA0_MAGIC);

  if( x->handler) {

    // printf("x       is %p\n", x );
    // printf("x->ctx  is %p\n", x->ctx);
    // printf("f       is %p\n", x->handler);

    x->handler( x->ctx, NULL );
  }
}


static void port_configure( interrupt_t *i)
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


void interrupt_fpga0_init( interrupt_t *i /* nvic_ctx_table * */ )
{
  assert(i);
  memset(i, 0, sizeof(interrupt_t));

  i->magic        = FPGA0_MAGIC;
  i->port_configure = port_configure;

  x = i;
}




/*
../../lib/libopencm3/include/libopencm3/stm32/common/exti_common_all.h:#define EXTI3                            (1 << 3)
  ../../lib/libopencm3/include/libopencm3/stm32/l4/nvic.h:#define NVIC_EXTI3_IRQ 9
*/


