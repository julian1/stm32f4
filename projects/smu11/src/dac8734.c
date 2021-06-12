

#include <libopencm3/stm32/spi.h>


#include "core.h"
#include "util.h" // usart_printf


#include "dac8734.h"

/*
  OK. dac can be initialized and registers written and read.
  without having analog power
*/

// TODO change to uint32_t
int voltage_to_dac( float x)
{
  // return x / (6.5769 * 2) * 65535;
  // eg.
  return x / 2.0 * 10000;  // need to add ptf56 60ohm, and then use dac trim registers.
}




void spi_dac_setup( uint32_t spi)
{
  // rcc_periph_clock_enable(RCC_SPI1);
  spi_init_master(
    spi,
    SPI_CR1_BAUDRATE_FPCLK_DIV_4,     // reasonably fast
    SPI_CR1_CPOL_CLK_TO_0_WHEN_IDLE,
    SPI_CR1_CPHA_CLK_TRANSITION_2,    // 2 == falling edge (from dac8734 doc.
    SPI_CR1_DFF_8BIT,
    SPI_CR1_MSBFIRST                  
  );
}


static uint32_t spi_dac_xfer_24(uint32_t spi, uint32_t r)
{
  uint8_t a = spi_xfer( spi, (r >> 16) & 0xff );
  uint8_t b = spi_xfer( spi, (r >> 8) & 0xff  );
  uint8_t c = spi_xfer( spi, r & 0xff  );

  // fixed this.
  return (a << 16) + (b << 8) + c;        // this is better. needs no on reading value .
}


static uint32_t spi_dac_xfer_register(uint32_t spi, uint32_t r)
{
  spi_enable( spi);
  uint32_t ret = spi_dac_xfer_24(spi, r );     // write
  spi_disable( spi);

  return ret;
}




void spi_dac_write_register(uint32_t spi, uint8_t r, uint16_t v)
{
  spi_dac_xfer_register( spi, r << 16 | v );
}



uint32_t spi_dac_read_register(uint32_t spi, uint8_t r)
{
  // write, with read flag (1 << 23) set, to read r.
  spi_dac_xfer_register(spi, 1 << 23 | r << 16 );

  // do a dummy read, while clocking out data, for previous read
  // simple although inefficient
  return spi_dac_xfer_register(spi, 1 << 23);

  // TODO IMPORTANT - we could chop off the high bit here.
}




// ok. peripherals have to be able to mux fo their IO.

int dac_init(uint32_t spi, uint8_t reg)  // bad name?
{
  usart_printf("------------------\n");
  usart_printf("dac8734 init\n");


  /*
    dac digital initialization. works even without analog power
    fail early if something goes wrong
  */

  mux_io(spi);

  // keep latch low, and unused, unless chaining
  io_clear(spi, reg, DAC_LDAC);

  // unipolar output on a
  io_set(spi, reg, DAC_UNI_BIP_A /*| DAC_UNIBIPB */);


  // toggle reset pin
  usart_printf("doing dac reset\n");
  io_clear(spi, reg, DAC_RST);
  msleep(20);
  io_set( spi, reg, DAC_RST);
  msleep(20);


  // see if we can toggle the dac gpio0 output
  mux_dac(spi);
  uint32_t u1 = spi_dac_read_register(spi, 0);
  // usart_printf("gpio0 set %d \n", (u1 & DAC_GPIO0) != 0 ); // TODO use macro for GPIO0 and GPIO1 // don't need == here
  // usart_printf("gpio1 set %d \n", (u1 & DAC_GPIO1) != 0);

  usart_printf("gpio set %d %d\n", (u1 & DAC_GPIO1) != 0, (u1 & DAC_GPIO1) != 0);


  // startup has the gpio bits set.
  // spi_dac_write_register(spi, 0, DAC_GPIO0 | DAC_GPIO1); // measure 0.1V. eg. high-Z without pu.
  spi_dac_write_register(spi, 0, 0 );                 // measure 0V

  uint32_t u2 = spi_dac_read_register(spi, 0);
  // usart_printf("read %d \n", u2 );
  // usart_printf("gpio0 set %d \n", (u2 & DAC_GPIO0) != 0);
  // usart_printf("gpio1 set %d \n", (u2 & DAC_GPIO1) != 0);

  usart_printf("gpio set %d %d\n", (u2 & DAC_GPIO1) != 0, (u2 & DAC_GPIO1) != 0);

  /* OK. to read gpio0 and gpio1 hi vals. we must have pullups.
     note. also means they can effectively be used bi-directionally.
  */
  if(u1 == u2) {
    // toggle not ok,
    usart_printf("dac toggle gpio not ok\n" );
    return -1;
  }


  mux_dac(spi);

  // check can write register also
  spi_dac_write_register(spi, DAC_VSET_REGISTER, 12345);
  msleep( 1);
  uint32_t u = spi_dac_read_register(spi, DAC_VSET_REGISTER) ;

  // usart_printf("u is %d\n", u );
  usart_printf("v set register val %d\n", u & 0xffff );
  usart_printf("v set register is %d\n", (u >> 16) & 0x7f);

  if( (u & 0xffff) != 12345) {
    usart_printf("dac setting reg not ok\n" );
    return -1;
  }

  // should go to failure... and return exit...
  usart_printf("write vset ok\n");

  // clear register
  spi_dac_write_register(spi, DAC_VSET_REGISTER, 0);

  // avoid turning on the refs. yet.
  return 0;
}




















#if 0

void dac_reset(void)
{
  // can / should do, before rails powerup.

  usart_printf("dac reset\n");

  /*
    code relies on task_sleep() for sequencing rst.
    therefore should only call in context of rtos thread.
    OTHERWISE - should just use a nop loop.
      and remove dependency on task_sleep. no still required for usart_print()
  */

  /* Load DAC latch control input(activelow). When LDAC is low, the DAC latch
  is transparent and the LDAC 56I contents of the Input Data Register are
  transferred to it.The DAC output changes to the corresponding level
  simultaneously when the DAClat */
  //
  gpio_clear(DAC_PORT, DAC_LDAC);   // keep latch low, and unused, unless chaining

  // ONLY needed FOR BIT-BASHING
  gpio_clear(DAC_PORT_SPI, DAC_CLK); // raise clock - should be set / high????


  /*
  Output mode selection of groupB (DAC-2 and DAC-3). When UNI/BIP-A is tied to
  IOVDD, group B is in unipolar output mode; when tied to DGND, group B is in
  bipolar output mode
  */
  // bipolar
  // gpio_clear(DAC_PORT, DAC_UNIBIPA | DAC_UNIBIPB);

  // unipolar
  /* gpio_set(DAC_PORT, DAC_UNIBIPA  | DAC_UNIBIPB); */



  /*
  Reset input (active low). Logic low on this pin resets the input registers
  and DACs to the values RST 67I defined by the UNI/BIP pins, and sets the Gain
  Register and Zero Register to default values.
  */
  // These times may need to be increased for true cold start...
  gpio_clear(DAC_PORT, DAC_RST);
  task_sleep(20);
  gpio_set(DAC_PORT, DAC_RST);
  task_sleep(20);


  // powered up with +-14V....

  /*
  Writing a '1' to the GPIO-0 bit puts the GPIO-1 pin into a Hi-Z state(default).
  DB8 GPIO-01 Writing a '0' to the GPIO-0 bit forces the GPIO-1 pin low

  p22 After a power-on reset or any forced hardware or software reset, all GPIO-n
  bits are set to '1', and the GPIO-n pin goes to a high-impedancestate.
  */
  /*
    THIS code is *NOT* or-ing with the current contents of the register.
    Instead it is overwriting. So do not use except as test.

    TODO - use a typedef struct defining all the default bit values we want, and use it as a mask.
  */
  usart_printf("mcu gpio read1 %d %d\n", gpio_get(DAC_PORT, DAC_GPIO0), gpio_get(DAC_PORT, DAC_GPIO1));

  // ok this appears to hang if dac is not populated
  usart_printf("dac clearing dac register\n");
  spi_dac_xfer_register( 0);
  task_sleep(1);
  usart_printf("mcu gpio read2 %d %d\n", gpio_get(DAC_PORT, DAC_GPIO0), gpio_get(DAC_PORT, DAC_GPIO1));



}

#endif

