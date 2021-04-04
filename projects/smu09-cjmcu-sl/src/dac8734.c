

#include <libopencm3/stm32/spi.h>

#include "dac8734.h"



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
    SPI_CR1_CPHA_CLK_TRANSITION_2,
    SPI_CR1_DFF_8BIT,
    SPI_CR1_MSBFIRST                  
  );
}


static uint32_t spi_dac_xfer_24(uint32_t spi, uint32_t r)
{
  // should be renamed. xfer. it can be read or write.

  uint8_t a = spi_xfer( spi, (r >> 16) & 0xff );
  uint8_t b = spi_xfer( spi, (r >> 8) & 0xff  );
  uint8_t c = spi_xfer( spi, r & 0xff  );


  // REVIEW
  return (a << 16) + (b << 8) + c;        // this is better. needs no on reading value .

  // return (c << 16) + (b << 8) + a;      // msb last... seems weird.
                                        // wrong...
                                          // eg. we are manually swapping bytes around elsewhere,
                                        /// usart_printf("bit set %d \n", (u1 & (1 << 8)) == (1 << 8));
                                        // Yes.

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
}



/*
    // can turn off - by dialing down voltage.
    // need to set the ldac etc.
    // should put this on a scope pin.

    spi_fpga_reg_clear(spi, DAC_REGISTER, DAC_RST);
    msleep(20);
    spi_fpga_reg_set( spi, DAC_REGISTER, DAC_RST);
    msleep(20);


    // spi_dac_write_register(uint32_t spi, uint8_t r, uint16_t v)

    mux_dac(spi);
    // spi_dac_xfer_register( spi, 0);

    spi_dac_write_register(spi, 0, 1 << 9 | 1 << 8);
*/


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

