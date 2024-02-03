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


#define SPI_SPI   SPI1

#define SPI1_PORT  GPIOA
#define SPI_CLK   GPIO5     // PA5
#define SPI_CS1   GPIO4     // PA4
#define SPI_MOSI  GPIO7     // PA7
#define SPI_MISO  GPIO6     // PA6
#define SPI_CS2   GPIO15    // PA15 nss 2. moved.


#define SPI_INTERUPT GPIO2   // PA2






void spi1_port_cs1_cs2_setup(void)
{

  // rcc_periph_clock_enable(RCC_GPIOA);
  // rcc_periph_clock_enable(RCC_SPI1);


  gpio_mode_setup(SPI1_PORT, GPIO_MODE_AF, GPIO_PUPD_NONE, SPI_CLK | SPI_MOSI | SPI_MISO);
  gpio_set_af(SPI1_PORT, GPIO_AF5, SPI_CLK | SPI_MOSI | SPI_MISO);       // af 5
  gpio_set_output_options(SPI1_PORT, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, SPI_CLK | SPI_MOSI );   // 100MHZ ??


  // CS1 to manual external gpio output
  gpio_mode_setup(SPI1_PORT, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, SPI_CS1 | SPI_CS2);
  gpio_set_output_options(SPI1_PORT, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, SPI_CS1 | SPI_CS2);

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

void spi_port_cs1_enable(uint32_t spi)  // enable
{
  spi_wait_ready( spi);

  if(spi == SPI_SPI) 
    gpio_clear(SPI1_PORT, SPI_CS1);
  else
    assert(0);
}


void spi_port_cs1_disable(uint32_t spi)  // disable
{
  spi_wait_ready( spi);

  if(spi == SPI_SPI)
    gpio_set(SPI1_PORT, SPI_CS1);
  else 
    assert(0);
}




void spi_port_cs2_enable(uint32_t spi)  // enable
{
  spi_wait_ready( spi);

  // active lo
  if(spi == SPI_SPI)
    gpio_clear(SPI1_PORT, SPI_CS2);
  else
    assert(0);
}


void spi_port_cs2_disable(uint32_t spi) // disable
{
  spi_wait_ready( spi);

  // We need

  if(spi == SPI_SPI)
    gpio_set(SPI1_PORT, SPI_CS2);
  else 
    assert(0);
}






//////////////////


/*
  this interupt can be for any kind of status change notice
  so, it's better to make a generalized func, rather than push this
  code into the ads131 code.
*/

static void (*spi1_interupt)(void *ctx) = NULL;
static void *spi1_ctx = NULL;


void exti2_isr(void)
{
  // interupt from ice40/fpga.

  /*
    EXTREME
    OK. bizarre. resetting immediately, prevents being called a second time
  */
  exti_reset_request(EXTI2);


  if(spi1_interupt) {
    spi1_interupt(spi1_ctx);
  }

}




/*
  ads131a04  DYDR Data ready; active low; host interrupt and synchronization for multi-devices
*/

void spi1_port_interupt_setup(void (*pfunc)(void *),  void *ctx)
{
  // TODO check non-null init args ...

  spi1_interupt = pfunc;
  spi1_ctx = ctx;

  gpio_mode_setup(SPI1_PORT, GPIO_MODE_INPUT, GPIO_PUPD_NONE, SPI_INTERUPT);

  // gpio_set_output_options(SPI1_PORT, GPIO_ITYPE, GPIO_ISPEED_50MHZ, SPI_SPECIAL);   is there a way to set the speed?
                                                                                                  // looks like GPIO_ITYPE is recognized.

  // ie. use exti2 for pa2
  nvic_enable_irq(NVIC_EXTI2_IRQ);
  // nvic_set_priority(NVIC_EXTI2_IRQ, 5 );

  exti_select_source(EXTI2, SPI1_PORT);
  // exti_set_trigger(EXTI2 , EXTI_TRIGGER_FALLING);
  exti_set_trigger(EXTI2 , EXTI_TRIGGER_RISING );         // JA. nov 1. 2023. to make consistent with _valid signal hi.
  exti_enable_request(EXTI2);
}











#if 0
static inline void wait_for_transfer_finished(uint32_t spi)
{
  /*
    see,


  [nix-shell:~/devel/stm32f4/projects/minimal]$ grep -r SPI_SR ../
  grep: ../dmm/main.elf: binary file matches
  ../old2/tft-stm32f410cbt3/src/lcd_spi.c:  while (SPI_SR(spi) & SPI_SR_BSY);
  ../old2/tft-stm32f410cbt3/src/lcd_spi.c:   while (!(SPI_SR(spi) & SPI_SR_TXE));
  ../old/smu07/old/dac8734.c:   while (!(SPI_SR(spi) & SPI_SR_TXE));
  ../old/smu01/src/dac8734.c:   while (!(SPI_SR(spi) & SPI_SR_TXE));
  ../minimal/src/peripheral/spi-port.c:  // while (!(SPI_SR(spi) & SPI_SR_RXNE));
  ../minimal/src/peripheral/spi-port.c:   while (!(SPI_SR(spi) & SPI_SR_TXE));

  */


  // return;
  /* Wait for transfer finished. */
  // while (!(SPI_SR(spi) & SPI_SR_RXNE));

   while (!(SPI_SR(spi) & SPI_SR_TXE));

}
#endif



#if 0

void spi_port_cs1_setup(void)
{
  // rcc_periph_clock_enable(RCC_SPI1);

  // setup spi with cs ...
  uint16_t out = SPI_CLK | SPI_CS1 | SPI_MOSI ; // not MISO
  uint16_t all = out | SPI_MISO;

  gpio_mode_setup(SPI1_PORT, GPIO_MODE_AF, GPIO_PUPD_NONE, all);
  gpio_set_af(SPI1_PORT, GPIO_AF5, all); // af 5
  gpio_set_output_options(SPI1_PORT, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, out);

  // we should be able to simplify this. to non configured or input.
  // http://libopencm3.org/docs/latest/gd32f1x0/html/group__gpio__mode.html

  /*

  ''During and just after reset, the alternate functions are not active and the I/O ports are configured in Input Floating mode (CNFx[1:0]=01b, MODEx[1:0]=00b).''
  */
  // set cs2 hi - with external pullup.
  gpio_mode_setup(SPI1_PORT, GPIO_MODE_INPUT, GPIO_PUPD_NONE, SPI_CS2);
}


void spi_port_cs2_setup(void)
{
  // rcc_periph_clock_enable(RCC_SPI1);

  // setup spi with cs ...
  uint16_t out = SPI_CLK | SPI_CS2 | SPI_MOSI ; // not MISO
  uint16_t all = out | SPI_MISO;

  gpio_mode_setup(SPI1_PORT, GPIO_MODE_AF, GPIO_PUPD_NONE, all);
  gpio_set_af(SPI1_PORT, GPIO_AF5, all); // af 5
  gpio_set_output_options(SPI1_PORT, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, out); // probably need to reset each time.

  // set cs1 hi - with external pullup.
  gpio_mode_setup(SPI1_PORT, GPIO_MODE_INPUT, GPIO_PUPD_NONE, SPI_CS1);
}



/////////////////////


void spi_port_cs1_gpio_setup(void)
{
  // for programming flash.
  // rcc_periph_clock_enable(RCC_SPI1);

  // setup spi with cs ...
  uint16_t out = SPI_CLK |  SPI_MOSI ; // not MISO
  uint16_t all = out | SPI_MISO;

  gpio_mode_setup(SPI1_PORT, GPIO_MODE_AF, GPIO_PUPD_NONE, all);
  gpio_set_af(SPI1_PORT, GPIO_AF5, all); // af 5
  gpio_set_output_options(SPI1_PORT, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, out); // probably need to reset each time.

  // set cs2 hi-z/hi - with external pullup.
  gpio_mode_setup(SPI1_PORT, GPIO_MODE_INPUT, GPIO_PUPD_NONE, SPI_CS2);

  // set CS1 to manual external gpio output
  gpio_mode_setup(SPI1_PORT, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, SPI_CS1);
  gpio_set_output_options(SPI1_PORT, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, SPI_CS1);
}




/////////////////////





void spi_port_cs2_gpio_setup(void)
{
  // for writing 4094.
  // rcc_periph_clock_enable(RCC_SPI1);

  // setup spi with cs ...
  uint16_t out = SPI_CLK |  SPI_MOSI ; // not MISO
  uint16_t all = out | SPI_MISO;

  gpio_mode_setup(SPI1_PORT, GPIO_MODE_AF, GPIO_PUPD_NONE, all);
  gpio_set_af(SPI1_PORT, GPIO_AF5, all); // af 5
  gpio_set_output_options(SPI1_PORT, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, out); // probably need to reset each time.

  // set cs1 hi-z/hi - with external pullup.
  gpio_mode_setup(SPI1_PORT, GPIO_MODE_INPUT, GPIO_PUPD_NONE, SPI_CS1);

  // set CS2 to manual external gpio output
  gpio_mode_setup(SPI1_PORT, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, SPI_CS2);
  gpio_set_output_options(SPI1_PORT, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, SPI_CS2);
}


#endif














#if 0

void spi1_port_cs1_cs2_gpio_setup(void)
{
  // control both cs1 and cs2 with gpio. for creset assert.

  gpio_mode_setup(SPI1_PORT, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, SPI_CS1 | SPI_CS2);
  gpio_set_output_options(SPI1_PORT, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, SPI_CS1 | SPI_CS2);

}
#endif

#if 0



void spi1_port_cs1_cs2_manual_setup(void)
{
  // configure
  // rcc_periph_clock_enable(RCC_SPI1);

  uint16_t out = SPI_CLK |  SPI_MOSI ; // not MISO
  uint16_t all = out | SPI_MISO;


  gpio_mode_setup(SPI1_PORT, GPIO_MODE_AF, GPIO_PUPD_NONE, all);
  gpio_set_af(SPI1_PORT, GPIO_AF5, all); // af 5
  gpio_set_output_options(SPI1_PORT, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, out); // probably need to reset each time.

  // set cs1 and cs2 as gpio
  // IMPORTANT - no longer open-drain.
  gpio_mode_setup(SPI1_PORT, GPIO_MODE_INPUT, GPIO_PUPD_NONE, SPI_CS1 | SPI_CS2);

  // set speed
  gpio_set_output_options(SPI1_PORT, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, SPI_CS1 | SPI_CS2);
}



#endif

