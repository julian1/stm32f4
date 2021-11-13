
/*
  TODO try to use the NSS. now have 10k pull up available.

  need to get rid of the 1ms task sleeps.
*/

#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/spi.h>


#include "sleep.h"
// #include "usart.h"
#include "serial.h"
#include "dac8734.h"


/*
  TODO SHOULD RENAME THESE

  DAC_SPI_PORT
  DAC_MOSI becomes DAC_SPI_MOSI

  DAC_CTL_PORT
  DAC_CTL_UNIBIPB
  etc.
  To make it clear which port is associated with which gpio/af more easily
*/

/*
  spi 1 AF5
    MOSI == PA7 == GPIO7    DAC SDI pin 4
    MISO == PA6 == GPIO6    DAC SDO pin 5
*/
#define DAC_SPI       SPI1
#define DAC_PORT_SPI  GPIOA

#define DAC_CS        GPIO4
#define DAC_CLK       GPIO5
#define DAC_MOSI      GPIO7
#define DAC_MISO      GPIO6



// OK. looks like we remapped everything. to GPIOB
#define DAC_PORT      GPIOB
#define DAC_LDAC      GPIO0
#define DAC_RST       GPIO1
// PB2, is BOOT1
// PB3, is SDO
// #define DAC_UNIBIPA   GPIO4
// #define DAC_UNIBIPB   GPIO5
#define DAC_GPIO0     GPIO6
#define DAC_GPIO1     GPIO7


// change name dac_write_register24
// pass the spi version?

// TODO . pass the spi as argument,
// much simpler.

static uint32_t dac_write_register_spi(uint32_t r)
{
  uint8_t a = spi_xfer( DAC_SPI, (r >> 16) & 0xff );
  uint8_t b = spi_xfer( DAC_SPI, (r >> 8) & 0xff  );
  uint8_t c = spi_xfer( DAC_SPI, r & 0xff  );


  // REVIEW
  // return (a << 16) + (b << 8) + c;
  return (c << 16) + (b << 8) + a;      // msb last... seems weird.
                                        // wrong...
                                          // eg. we are manually swapping bytes around elsewhere,

                                        /// usart_printf("bit set %d \n", (u1 & (1 << 8)) == (1 << 8));

                                        // Yes.
    
}



static uint32_t dac_write_register1(uint32_t r)
{
  spi_enable( DAC_SPI );
  uint32_t ret = dac_write_register_spi( r );     // write
  spi_disable( DAC_SPI );

  return ret;
}


// RENAME THIS....
void dac_write_register(uint8_t r, uint16_t v)
{
  // eg. like reg, value
  dac_write_register1( r << 16 | v );
}




uint32_t dac_read_register(uint8_t r)
{
  // write, with read flag (1 << 23) set, to read r.
  dac_write_register1(1 << 23 | r << 16 );

  // do a dummy read, while clocking out data, for previous read
  // simple although inefficient
  return dac_write_register1(1 << 23);
}




void dac_setup_spi( void )
{
  // TODO change name. just dac_setup...  spi is given.

    error. miso should not have output option set
    // TODO - FIXME. WRRONG
  uint32_t all = DAC_CLK | DAC_MOSI | DAC_MISO | DAC_CS;

  usart_printf("dac setup spi\n");

  // spi alternate function

  // TODO // don't refer to GPIOA

  gpio_mode_setup(GPIOA, GPIO_MODE_AF, GPIO_PUPD_NONE, all);

  // OK.. THIS MADE SPI WORK AGAIN....
  // note, need harder edges for signal integrity. or else different speed just helps suppress parasitic components
  // see, https://www.eevblog.com/forum/microcontrollers/libopencm3-stm32l100rc-discovery-and-spi-issues/
  gpio_set_output_options(GPIOA, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, all);

  // af 5
  gpio_set_af(GPIOA, GPIO_AF5, all);

  // rcc_periph_clock_enable(RCC_SPI1);
  spi_init_master(
    DAC_SPI,
    SPI_CR1_BAUDRATE_FPCLK_DIV_4,     // SPI_CR1_BAUDRATE_FPCLK_DIV_256,
    SPI_CR1_CPOL_CLK_TO_0_WHEN_IDLE,  // SPI_CR1_CPOL_CLK_TO_1_WHEN_IDLE ,
    SPI_CR1_CPHA_CLK_TRANSITION_2,    // 2 == falling edge (from dac8734 doc.
    SPI_CR1_DFF_8BIT,
    SPI_CR1_MSBFIRST                  // SPI_CR1_LSBFIRST
  );

  spi_disable_software_slave_management( DAC_SPI );
  spi_enable_ss_output(DAC_SPI);

  // ************
  // TODO remove. this should be enabled in use... m
  spi_enable(DAC_SPI);

  ///////////////

  // should set a reasonable state before enabling...

  // internal pu, doesn't change anything - because its powered off, and starts up high-Z.
  gpio_mode_setup(DAC_PORT, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, DAC_RST | DAC_LDAC /*| DAC_UNIBIPA | DAC_UNIBIPB */);
  gpio_mode_setup(DAC_PORT, GPIO_MODE_INPUT, GPIO_PUPD_PULLUP, DAC_GPIO0 | DAC_GPIO1 ); // these are open-drain as inputs

  usart_printf("dac setup spi done\n");
}


// think we have to look at it on a scope.




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
  dac_write_register1( 0);
  task_sleep(1);
  usart_printf("mcu gpio read2 %d %d\n", gpio_get(DAC_PORT, DAC_GPIO0), gpio_get(DAC_PORT, DAC_GPIO1));

  usart_printf("dac set\n");

  // doing two writes fills in the register?
  dac_write_register(0, 1 << 9 | 1 << 8);
  task_sleep(1);
  usart_printf("mcu gpio read3 %d %d\n", gpio_get(DAC_PORT, DAC_GPIO0), gpio_get(DAC_PORT, DAC_GPIO1));


  // R/W = '1' sets a read-back operation. For read operation, bits A3 to A0 select the register to be read.
  usart_printf("----------\n");



  ////////////////////
  // write, then read the configuration register 0
  // 128 (0x80) seems be hard-set as default...
  dac_write_register(0,  1 << 8);       // gpio bit,  640  == 512 + 128
  uint32_t u1 = dac_read_register(0);
  usart_printf("read %d \n", u1 );
  usart_printf("bit set %d \n", (u1 & (1 << 8)) == (1 << 8));
  task_sleep(1);


  usart_printf("----------\n");

  ////////////////
  // writing then read an output register
  // output register is 16 bit, two bytes only. not 3 bytes == 24.

  dac_write_register(DAC_VSET_REGISTER, 12345);
  task_sleep(1);
  uint32_t u = dac_read_register(DAC_VSET_REGISTER);
  usart_printf("read %d \n",  (u & 0x00ff00) | (u >> 16));     // 12345 works...




  /*
  The DAC8734 updates the DAC latch only if it has been accessed since the last
  time the LDAC pin was brought low or the LD bit in the CommandRegister was set
  to'1', there by eliminating any unnecessary glitch.
  */
  // dac_write_register1( 0 << 16 | 1 << 14); // LD bit


  /*
   To set the DAC-x gain=2, connect RFB1-x and RFB2-x to VOUT-x, and set the
  corresponding GAIN bit in the CommandRegister to'0'. To set the DAC-xgain=4,
  connect RFB1-x to VOUT-x, keep RFB2-x open, and set the corresponding GAIN bit
  in the CommandRegister to '1'. After power-on reset or userreset, the GAIN bits
  are set to'1' by default; forgain=2, the gain bits must be cleared to'0'.
  */
  // *NOT* or-ing.
  // sets all bits.
  dac_write_register1( 0);  // set gain bit to 0. also sets LD bit to 0


  /*
  34,the digital supplies (DVDD and IOVDD) and logic inputs (UNI/BIP-x) must be
  applied before AVSS and AVDD. Additionally, AVSS must be applied before AVDD
  unless both can ramp up at the same time. REF-x should be applied after AVDD
  comes up in order to make sure the ESD protection circuitry does not turn on.
  */

  usart_printf("dac reset done\n");
}



