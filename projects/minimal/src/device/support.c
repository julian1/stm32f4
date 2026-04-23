
#include <stdio.h>

#include <libopencm3/stm32/gpio.h>    // bsrr
#include <libopencm3/stm32/spi.h>
#include <libopencm3/cm3/scb.h>  // reset()

#include <device/support.h>



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




static inline void spi_wait_until_not_busy(uint32_t spi)
{
  /*
    see,
    http://libopencm3.org/docs/latest/stm32f4/html/spi__common__all_8c_source.html#l00194
  */
  /* Wait until not busy */
  while (SPI_SR(spi) & SPI_SR_BSY);
}

static inline void spi_wait_for_transfer_finish(uint32_t spi)
{
   /* Wait for transfer finished. */
   while (!(SPI_SR(spi) & SPI_SR_TXE));


}


void spi_wait_ready(uint32_t spi )
{
/*
  see example, that also uses a loop.
  https://github.com/libopencm3/libopencm3-examples/blob/master/examples/stm32/f4/stm32f429i-discovery/lcd-dma/lcd-spi.c
*/
  // so we actually need both of these,

  spi_wait_until_not_busy (spi);

  spi_wait_for_transfer_finish( spi);   // may not need. if check not_busy, on both enable and disable.

}


void gpio_write_val(uint32_t gpioport, uint16_t gpios, bool val)
{
  // sets all the gpios to the val.  although we only use for single bit values
  // not very clear.
  // better place for this?
  // inline in include/gpio perhaps.

  /*
  // set/clear gpios bits, according to bool val
  // use CM3 style args
  // eg. gpio_write_val( GPIOA, GPIO9, 1);
  // eg.

  assert( GPIO9 == 1<< 9 );
  assert( GPIOA ==  (PERIPH_BASE_AHB1 + 0x0000) );

  */

  // BSRR == bit set/reset register.


  GPIO_BSRR( gpioport) |= gpios  << (val ? 0: 16);
}




void gpio_write_with_mask( uint32_t gpioport, uint32_t shift, uint32_t mask, uint32_t val)
{
  // better name? write consequtive bits into BSRR with shift and mask.

  GPIO_BSRR( gpioport) =
    // ((val << shift) & mask << shift)
    ((val & mask) << shift)                             // set bits
    | (((~val << shift) & (mask << shift) ) << 16);     // clear bits
}




void mcu_reset()
{
  // reset stm32f4
  // scb_reset_core()
  scb_reset_system();


}



#if 0
static void f()
{
  SCB->VTOR;
};
#endif

/*
  see lib2/util.c  for other approaches

*/


void print_stack_pointer()
{
  // https://stackoverflow.com/questions/20059673/print-out-value-of-stack-pointer
  // non-portable.
  void* p = NULL;

  printf("sp %p   %d\n", (void*)&p,  ( (unsigned)(void*)&p) - 0x20000000   );

  // return &p;
  // uint32_t x = _stack;
}



#if 0

/*
  we use a msleep() without app context,
  when initializing fpga and vfd
  could localize here if desired.
*/

void msleep(uint32_t delay, volatile uint32_t *system_millis)
{

  // works for system_millis integer wrap around
  // could be a do/while block.
  uint32_t start = *system_millis;
  while (true) {
    uint32_t elapsed = *system_millis - start;
    if(elapsed > delay)
      break;

    // yield()
  };
}


#endif

