
#pragma once

/*

  peripheral interface is abstract, and not associated with any device instance.

*/


/*
../../lib/libopencm3/include/libopencm3/stm32/common/exti_common_all.h:#define EXTI3                            (1 << 3)
  ../../lib/libopencm3/include/libopencm3/stm32/l4/nvic.h:#define NVIC_EXTI3_IRQ 9
*/


/*


  note. the NVIC interrupt number (often referred to as IRQn in ARM Cortex-M
  microcontrollers) corresponds directly with the vector table

  just pass array the init/constructor.
  use separate vectors - for NVIC versus internal system/core exceptions that are negatively indexed.

  No. need for a struct to manage both - since we already know ahead of time -
  for each interrupt source (normal or core) which array is needed.

  like this,

  void isr_x( void)
  {
    void *ctx = glb_ctx_irq_vector[ NVIC_EXTI_X_IRQ];
    cb_t *cb  = glb_cb_irq_vector[ NVIC_EXTI_X_IRQ] ;

    if( ctx && cb)
      cb( ctx, NULL );

  }

  still not that clean - since it need globals for ctx. and the handler function.

*/





typedef struct interrupt_t  interrupt_t;

// TODO consider change typename to something simpler eg.   cb_t
typedef void (*interrupt_handler_t)( void *ctx, void *arg);




struct interrupt_t
{
  uint32_t magic;

  /*
    remove these fields if use array/table
  */
  interrupt_handler_t  handler;
  void *ctx;


  /*
      port_configure is correct name, since exti will configure input port,pin
  */
  void (*port_configure)( interrupt_t *);
  void (*handler_set)( interrupt_t *, void *ctx, interrupt_handler_t);
};




static inline void interrupt_port_configure( interrupt_t *i)
{
  assert( i && i->port_configure);
  i->port_configure( i);
}


static inline  void interrupt_handler_set( interrupt_t *i, void *ctx, interrupt_handler_t h)
{
  assert( i && i->handler_set);
  i->handler_set( i, ctx, h);
}






#if 0

    OLD.

    could use a struct with two arrays - for nvic, and core interrupts/exceptions

cleaner way to manage context pointers for callbacks/handlers.
  is single global table.

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
  ----

  struct ctx_table_t {

    void *nvic[  NVIC_IRQ_COUNT ] ;
    void *core_irq[  ];       // ie. positively indexed
  } ;
  actually just use separate array - for the negative internale system interrupts.
  -----------------------


#endif


