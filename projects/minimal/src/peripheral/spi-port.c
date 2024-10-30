/*
   consider file rename spi1-port.c
    because it is specific.

  ---
  - this code is just about generic enough. to move to lib2
  - perhaps should make generic for any spi.  and/or  prefix filename spi1-port.c  etc.

  ---
  anything through a 6 pin adum/cap isolator

  Code is shared for smu and adc
  move to shared library?

  -----------

  EXTR. dec 18 2022.
    at 100MHz (instead of 50MHz) there are flip issues, under stress test. with iso7762

  could use an and gate ic with gpio. but dificulty of synchronizing signals

*/

#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/spi.h>


#include <libopencm3/cm3/nvic.h>
#include <libopencm3/stm32/exti.h>


#include <stdio.h>
#include <stddef.h>   // NULL
#include <assert.h>

#include <peripheral/spi-port.h>


//////////////////////////

// #define UNUSED(x) (void)(x)


// TODO - no reason to have the ICE40  prefix here.
// should just name to SPI1c

/*
  jan 2024.
  - if put all these in a data structure - then the enable/disable could be made more generic. for spi1,or spi2 etc.
  - we would'nt have to carry around the spi integer. in app.
  - not sure the callback works.
  ----
  - rather than having to prefix with spi1... everywhere.

*/

// For dsn257.

#define SPI1_PORT       GPIOA
#define SPI1_CS1        GPIO4     // PA4
#define SPI1_CS2        GPIO15     // gerber157.
#define SPI1_INTERUPT   GPIO3     // PA3

// #define SPI1_CS2       GPIO10    // PA10 july 2024. nss moved.

// clk,PA5   mosi, PB5.  miso PA6


/*
  feb 2024. splitting spi configuration over two ports (mosi on PB5) . is messy. but needed to free PA7 pin for ethernet.
  keep the cs1 and cs2 gpio config separate


  see hal.h  and PIN() macro for cleaner example looping an array.
    vim ~/devel/mongoose/examples/stm32/nucleo-f429zi-make-baremetal-builtin/hal.
*/










void spi1_port_setup(void)
{
  printf("spi1 port setup\n");

  /* port, not driver.
    performed once only.
  */

	// hold cs lines lo - to put fpga in reset, avoid isolator/fpga contention, because fpga wants to become master and drive spi lines.
  // probably ok, if just SS held lo.
	spi_port_cs2_enable( SPI1);
  spi_port_cs1_enable( SPI1);

  /*
  oct 2024.

  If/once fpga enters contention/latchup condition. we cannot get out of it. without re-power.
  even if set all pins lo / including RST . eg. if analog side, loses power and is repowered.
  There is no general way to avoid this. without having a power sense/monitor to report back to digital side.

  if cs hi, we get contention/fight - regardless of state of cs2/gpio pin. probably due on the cs pin itself.  or perhaps due to clk/mosi pin.
  if cs is lo.  then ok.   regardless of gpio.

  EXTR. this is because SS is sampled at POR, to determine if fpga should orient/act as master or peripheral, see DS, p11. configuration chart.
  But all this makes spi1 not a generalizable service that the fpga hangs off.
  the solution is move the control of/and start deefault value of SS off the cs1/cs2 assert condition.

  Actually RST is a gate in the sequence before SS is sampled to determine master/slave orientation.
  So just holding in RST. is also sufficient.

  We get correct behavior at the moment by using 'F' version of isolator that emits lo. at start if mcu/digital side unpowered.
  and presumably when mcu is in reset, emitting high-z on gpio.


	// locks up.
	spi_port_cs2_disable(SPI1 );      // both gpio and cs hi. worst case. both fight.   both cs hi.  CC 2.9V / 29mA.  eg.
  spi_port_cs1_disable(SPI1 );

  // locks up
	spi_port_cs2_enable(SPI1 );			  // gpio pin lo.  but cs is hi. and fights.   but less worse CC 3.28V/29mA .
  spi_port_cs1_disable(SPI1 );

	// OK. works.
	spi_port_cs2_disable(SPI1 );			// gpio hi, and cs lo.   ok. doesn't fight. because gpio is high-z before bitstream configuration.
  spi_port_cs1_enable(SPI1 );       // this is the same state, as when, we are trying to configure the bitstream.

  */



  // perhaps simpler with array loop ia[] = { GPIOA ,  GPIO5 } etc.

  // clk, miso.
  gpio_mode_setup(GPIOA, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO5  | GPIO6);
  gpio_set_af(GPIOA, GPIO_AF5, GPIO5  | GPIO6);       // af 5
  gpio_set_output_options(GPIOA, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, GPIO5 );   // 100MHZ ??


  // mosi. on PB5.
  gpio_mode_setup(GPIOB, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO5 );
  gpio_set_af(GPIOB, GPIO_AF5, GPIO5 );       // af 5
  gpio_set_output_options(GPIOB, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, GPIO5 );   // 100MHZ ??


  // CS1 to manual external gpio output
  gpio_mode_setup(SPI1_PORT, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, SPI1_CS1 | SPI1_CS2);
  gpio_set_output_options(SPI1_PORT, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, SPI1_CS1 | SPI1_CS2);

}


/*
  - almost all spi is active lo.
  - only fpga config . should change name to enable/disable

  - 4094 is inverted, by fpga or 74lvc1g.

  only expose enable/disable here.  whether active hi/lo is detail.

  ---
  whether active high/lo depends on the peripheral device. and fpga may invert
  so use  _enable(), disable() . instead of clear
  following normal CS convention. active low

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


static inline void spi_wait_ready(uint32_t spi )
{
/*
  see example, that also uses a loop.
  https://github.com/libopencm3/libopencm3-examples/blob/master/examples/stm32/f4/stm32f429i-discovery/lcd-dma/lcd-spi.c
*/
  // so we actually need both of these,

  spi_wait_until_not_busy (spi);

  spi_wait_for_transfer_finish( spi);   // may not need. if check not_busy, on both enable and disable.

}


  //////////////////////

// we can factor this common stuff, later

void spi_port_cs1_enable(uint32_t spi)
{
  spi_wait_ready( spi);

  if(spi == SPI1)
    gpio_clear(SPI1_PORT, SPI1_CS1);
  else
    assert(0);

  // printf("spi1 cs1 lo\n");

}


void spi_port_cs1_disable(uint32_t spi)
{
  spi_wait_ready( spi);

  if(spi == SPI1)
    gpio_set(SPI1_PORT, SPI1_CS1);
  else
    assert(0);

  // printf("spi1 cs1 hi\n");
}




void spi_port_cs2_enable(uint32_t spi)
{
  spi_wait_ready( spi);

  // active lo
  if(spi == SPI1)
    gpio_clear(SPI1_PORT, SPI1_CS2);
  else
    assert(0);

  // printf("spi1 cs2 lo\n");
}


void spi_port_cs2_disable(uint32_t spi)
{
  spi_wait_ready( spi);

  // We need

  if(spi == SPI1)
    gpio_set(SPI1_PORT, SPI1_CS2);
  else
    assert(0);

  // printf("spi1 cs2 hi\n");
}



bool spi_port_cdone_get(void)
{
   return gpio_get(SPI1_PORT, SPI1_INTERUPT)  != 0;

}

//////////////////


/*
  this interupt can be for any kind of status change notice
  so, it's better to make a generalized func, rather than push this
  code into the ads131 code.
*/

static void (*spi1_interupt)(void *ctx) = NULL;
static void *spi1_ctx = NULL;

void spi1_port_interupt_handler_set( void (*f)(void *), void *ctx)
{
  spi1_interupt = f;
  spi1_ctx = ctx;
}


/*
  mar. 2024.  interuupt moved from PA2 to PA3.

*/

void exti3_isr(void) // called by runtime
{
  /*
    OK. bizarre. resetting immediately, prevents being called a second time
  */
  exti_reset_request(EXTI3);

  if(spi1_interupt) {
    spi1_interupt(spi1_ctx);
  }
}





void spi1_port_interupt_setup()
{
  gpio_mode_setup(SPI1_PORT, GPIO_MODE_INPUT, GPIO_PUPD_NONE, SPI1_INTERUPT);

  // ie. use exti2 for pa2, exti3 for pa3
  nvic_enable_irq(NVIC_EXTI3_IRQ);
  // nvic_set_priority(NVIC_EXTI3_IRQ, 5 );

  exti_select_source(EXTI3, SPI1_PORT);
  exti_set_trigger(EXTI3 , EXTI_TRIGGER_RISING );         // JA. nov 1. 2023. to make consistent with _valid signal hi.
  exti_enable_request(EXTI3);
}




//////////////////////////////













