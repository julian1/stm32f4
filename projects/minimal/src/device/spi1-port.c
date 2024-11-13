/*

  dec 18 2022.
    at 100MHz (instead of 50MHz) there are flip issues, under stress test. with iso7762


*/

#include <libopencm3/stm32/gpio.h>
// #include <libopencm3/stm32/spi.h>       // TODO REMOVE.
#include <libopencm3/cm3/nvic.h>
#include <libopencm3/stm32/exti.h>


#include <stdio.h>
#include <assert.h>

#include <device/spi-port.h>


#define SPI1_PORT       GPIOA
#define SPI1_CS1        GPIO4     // PA4
#define SPI1_CS2        GPIO15     // gerber157.
#define SPI1_INTERUPT   GPIO3     // PA3





void spi1_port_setup(void)
{
  printf("spi1 port setup\n");

  /* device
  */

	// hold cs lines lo - to put fpga in reset, avoid isolator/fpga contention, because fpga wants to become spi master and drive spi lines.
  // probably ok, if just SS held lo.
  // there's a delay of about 1ms. though before ice40 samples inputs.

  assert(0);  

  // these are no longer in scope.
  // but we can enforce order  in main.c
	// spi_port_cs2_enable( SPI1);
  // spi_port_cs1_enable( SPI1);

  /*
  oct 2024.

  If/once fpga enters contention/latchup condition. we cannot get out of it. without re-power.
  even if set all pins lo / including RST . eg. if analog side, loses power and is repowered.
  There is no general way to avoid this. without having a power sense/monitor to report back to digital side.

  EXTR. this is because SS is sampled at POR, to determine if fpga should orient/act as master or peripheral, see DS, p11. configuration chart.
  But all this makes spi1 not a generalizable service that the fpga hangs off.
  the solution is move the control of/and start deefault value of SS off the cs1/cs2 assert condition.

  Actually RST is a gate in the sequence before SS is sampled to determine master/slave orientation.
  So just holding in RST. is also sufficient.

  We get correct behavior at the moment by using 'F' version of isolator that emits lo. at start if mcu/digital side unpowered.
  and presumably when mcu is in reset, emitting high-z on gpio.

  ---
  we could move RST, SS to a hc175 type latch. that can start with outputs lo. using asynch CLR.

  ---
  eg.

	// locks up.
	spi_port_cs2_disable(SPI1 );      // both gpio and cs hi. worst case. both fight.   both cs hi.  CC 2.9V / 29mA.  eg.
  spi_port_cs1_disable(SPI1 );

  // locks up
	spi_port_cs2_enable(SPI1 );			  // gpio pin lo.  but cs is hi. and fights.   but less worse CC 3.28V/29mA .
  spi_port_cs1_disable(SPI1 );

	// OK. works.
	spi_port_cs2_disable(SPI1 );			// gpio hi, and cs lo.   ok. doesn't fight. because gpio is high-z before bitstream configuration.
  spi_port_cs1_enable(SPI1 );       // this is the same state, as when, we are trying to configure the bitstream.

  summary if cs hi, we get contention/fight - regardless of state of cs2/gpio pin. probably due on the cs pin itself.  or perhaps due to clk/mosi pin.
  if cs is lo.  then ok.   regardless of gpio.
  */



  // perhaps simpler with array loop ia[] = { GPIOA ,  GPIO5 } etc.

  /*
    The simpler way is to group. according to setup,af, output.

  */

  // clk, miso.
  gpio_mode_setup(GPIOA, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO5  | GPIO6);
  gpio_set_af(GPIOA, GPIO_AF5, GPIO5  | GPIO6);       // af 5
  gpio_set_output_options(GPIOA, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, GPIO5 );   // 100MHZ ??


  // mosi. on PB5.
  gpio_mode_setup(GPIOB, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO5 );
  gpio_set_af(GPIOB, GPIO_AF5, GPIO5 );       // af 5
  gpio_set_output_options(GPIOB, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, GPIO5 );   // 100MHZ ??


#if 0
  // CS1, CS2 to manual external gpio output
  gpio_mode_setup(SPI1_PORT, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, SPI1_CS1 | SPI1_CS2);
  gpio_set_output_options(SPI1_PORT, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, SPI1_CS1 | SPI1_CS2);
#endif

}




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


//////////////////////////



/*
  jan 2024.
  - if put all these in a data structure - then the enable/disable could be made more generic. for spi1,or spi2 etc.
  - we would'nt have to carry around the spi integer. in app.
  - not sure the callback works.
  ----
  - rather than having to prefix with spi1... everywhere.

*/

// For dsn257.


#if 0
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

#endif

//////////////////



/*
  feb 2024. splitting spi configuration over two ports (mosi on PB5) . is messy. but needed to free PA7 pin for ethernet.
  keep the cs1 and cs2 gpio config separate


  see hal.h  and PIN() macro for cleaner example looping an array.
    vim ~/devel/mongoose/examples/stm32/nucleo-f429zi-make-baremetal-builtin/hal.


   consider file rename spi1-port.c
    because it is specific.

  ---
  - this code is just about generic enough. to move to lib2
  - perhaps should make generic for any spi.  and/or  prefix filename spi1-port.c  etc.

  ---
  anything through a 6 pin adum/cap isolator

  Code is shared for smu and adc
  move to shared library?

 
*/
// actually this is probably wrongly named.
// should be gpio_cdone_get().
// bool spi_port_cdone_get(void);





///////

// abstracted over spi
// hardware ness with set_nss_high etc.
// doesn't work well with more than one cs.

/*
  we cannot abstract over these.
    cs1 will be common. but that functionality will be different
    ----------

  But consumers/users of this spi - should not have to know if its spi1 or spi2.
    eg. dac code, 4094 code etc.

  so something is not right.

*/

#if 0
void spi_port_cs1_enable(uint32_t );    // active lo
void spi_port_cs1_disable(uint32_t);


void spi_port_cs2_enable(uint32_t) ;
void spi_port_cs2_disable(uint32_t );
#endif



