

#include <libopencm3/stm32/spi.h>


#include "util.h"
#include "assert.h"
#include "streams.h" // usart_printf

// #include "mux.h"
// #include "spi-ice40.h"
// #include "reg.h"

#include "spi.h"

#include "4094.h"
#include "dac8734.h"

/*
  OK. dac can be initialized and registers written and read.
  without having analog power
*/

// TODO change return value to uint32_t?
int voltage_to_dac( float x)
{
  /* this uses 50k points of 65535.
    ie. vset=10V,  10/2*10k = 50000
  */
  return x / 2.0 * 10000;
/*
  For unipolar mode: AVDD ≥ Gain × VREF + 1V, and AVSS ≤ –2 × VREF – 1V.
  2 * 8.192 + 1 => rails = +-17.3V.

  15V    get 4.44V.
  16V    get 4.73V      (needs *both* +- rails )
*/
  // ie. vset=10V => 10/2*8k = 40000.
  // return x / 2.f * 8000;
}




void spi_dac_setup( uint32_t spi)
{
  // rcc_periph_clock_enable(RCC_SPI1);
  spi_init_master(
    spi,
    // SPI_CR1_BAUDRATE_FPCLK_DIV_4,     // at 168MHz. too fast. even with nop loop between cs.
    SPI_CR1_BAUDRATE_FPCLK_DIV_8,     // ok 168MHz. with nop loop on cs
    SPI_CR1_CPOL_CLK_TO_0_WHEN_IDLE,
    SPI_CR1_CPHA_CLK_TRANSITION_2,    // 2 == falling edge (from dac8734 doc.
    SPI_CR1_DFF_8BIT,
    SPI_CR1_MSBFIRST
  );

  spi_disable_software_slave_management(spi);
  spi_enable_ss_output(spi);

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

  /* this is probably being heavily inlined. and the time between cs assert is too short to be distinct.
    about 200ns. on scope. with other code
    - alternatively could probably move to separate file to avoid inlining
  */
  for(uint32_t i = 0; i < 10; ++i)
     __asm__("nop");


  // assert(spi == SPI1);
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
  UNUSED(reg);
  usart_printf("------------------\n");
  usart_printf("dac8734 init\n");


  /*
    dac digital initialization. works even if without analog power
    fail early if something goes wrong
  */

#if 0
  mux_ice40(spi);

  // keep latch low, and unused, unless chaining
  ice40_reg_clear(spi, reg, DAC_LDAC);

  // unipolar output on a
  ice40_reg_set(spi, reg, DAC_UNI_BIP_A /*| DAC_UNIBIPB */);
#endif


  // JA
  spi1_port_cs2_setup();
  spi_4094_setup(spi);
  spi_4094_reg_write(spi, REG_DAC_RST | REG_DAC_UNI_BIP_A);


  //////////////
  /*
  To set the gain = 4, connect RFB1-x to DAC-x with RFB2-x left open, and set the gain bit for that channel to '1' in the Command Register.
  To set the gain = 2, connect both RFB1-x and RFB2-x to DAC-x, and set the gain bit for that channel to '0'.
  The gain bits in the Command Register are set to '1' by default at power-on or reset, and must be cleared to '0' for gain = 2.

    we set gain when write 0 into main reg.
  */

#if 0
  // toggle reset pin
  usart_printf("doing dac reset\n");
  ice40_reg_clear(spi, reg, DAC_RST);
  msleep(20);
  ice40_reg_set( spi, reg, DAC_RST);
  msleep(20);
#endif

  // JA
  spi_4094_reg_write(spi, REG_DAC_UNI_BIP_A);
  msleep(1);
  spi_4094_reg_write(spi, REG_DAC_RST | REG_DAC_UNI_BIP_A);
  msleep(1);



  spi1_port_cs1_setup(); // with CS.
  spi_dac_setup( spi);

  // DAC_CMD_REGISTER

  // see if we can toggle the dac gpio0 output
  // mux_dac(spi);
  uint32_t u1 = spi_dac_read_register(spi, DAC_CMD_REGISTER);

  // default value
  usart_printf("u1 %x\n", u1);

  // test default values...
  assert(u1 == 0x80033c);

  usart_printf("gpio gain out0 %d,  out1 %d\n", (u1 & DAC_GAIN_BIT0) != 0, (u1 & DAC_GAIN_BIT1) != 0);
  assert(u1 & DAC_GAIN_BIT0);
  assert(u1 & DAC_GAIN_BIT1);

  usart_printf("gpio test set %d %d\n", (u1 & DAC_GPIO1) != 0, (u1 & DAC_GPIO1) != 0);

  /*
    clear main reg.
    IMPORTANT....
    this also clears the gain regsiters. needed for x2.
    but should really be explicit and use masks
  */
  spi_dac_write_register(spi, DAC_CMD_REGISTER, 0);


  uint32_t u2 = spi_dac_read_register(spi, DAC_CMD_REGISTER);
  usart_printf("gpio test set %d %d\n", (u2 & DAC_GPIO1) != 0, (u2 & DAC_GPIO1) != 0);

  /* OK. to read gpio0 and gpio1 hi vals. we must have pullups.
     note. also means they can effectively be used bi-directionally.
  */
  if(u1 == u2) {
    // toggle not ok,
    usart_printf("dac test toggle gpio not ok\n" );
    return -1;
  } else {
    usart_printf("dac test toggle ok\n" );
  }


  // mux_dac(spi);

  // check can write register also
  spi_dac_write_register(spi, DAC_DAC0_REGISTER, 12345);
  msleep( 1);
  uint32_t u = spi_dac_read_register(spi, DAC_DAC0_REGISTER) ;
  // usart_printf("u is %d\n", u );
  // usart_printf("v set register val %d\n", u & 0xffff );
  // usart_printf("v set register is %d\n", (u >> 16) & 0x7f);

  if( (u & 0xffff) != 12345) {
    usart_printf("dac test set reg and read not ok\n" );
    return -1;
  } else {
    usart_printf("dac test set reg and read ok\n" );
  }

  // should go to failure... and return exit...
  // usart_printf("write vset ok\n");

  // clear register
  spi_dac_write_register(spi, DAC_DAC0_REGISTER, 0);

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

