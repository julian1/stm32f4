
#pragma once

/*

  peripheral interface is abstract, and not associated with any device instance.

*/


/*
  can pass the cb table in the init/constructor.
  use separate tables - for NVIC versus internal system/core exceptions that are negatively indexed.

  struct cb_table_t {
    void                *ctx;
    interrupt_handler_t *handler;
    void                (*isr)( void);   // used as check.
  }


  void isr_x( void)
  {
    cb_table_t *cb = cb_a [ NVIC_EXTI3_IRQ ];

    assert( cb->isr == exti3_isr);

    if( cb->ctx && cb->handler)
      cb->handler( cb->ctx, NULL );

  }

  can then remove the ctx, and func. from the specific handler

*/





typedef struct interrupt_t  interrupt_t;

typedef void (*interrupt_handler_t)( void *ctx, void *arg);



typedef struct cb_table_t
{

  void                *ctx;
  interrupt_handler_t *handler;
  void                (*isr)( void);   // used as check.
} cb_table_t;



struct interrupt_t
{
  uint32_t magic;

  // cb_table_t  *cb_table;
  // size_t       cb_table_sz;


  /*
    remove these fields if use array/table
  */
  interrupt_handler_t  handler;
  void *ctx;


  /* rename to configure
  */
  void (*port_configure)( interrupt_t *);
  // void (*handler_set)( interrupt_t *, void *ctx, interrupt_handler_t);
};




static inline void interrupt_port_configure( interrupt_t *i)
{
  assert(i);
  i->port_configure( i);
}


static inline  void interrupt_handler_set( interrupt_t *i, void *ctx, interrupt_handler_t h)
{
  assert(i);

  // i->handler_set( i, ctx, h);

  i->ctx      = ctx;
  i->handler  = h;

  // cannot be generic.
}







/*

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


*/
