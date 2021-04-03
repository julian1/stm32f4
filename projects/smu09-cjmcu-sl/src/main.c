
// cjmcu  stm32f407.
// issue. is that board/stlink doesn't appear to reset cleanly. needs sleep.


#include <libopencm3/stm32/rcc.h>

//#include <libopencm3/stm32/gpio.h>
//#include <libopencm3/cm3/nvic.h>
//#include <libopencm3/cm3/systick.h>
//#include <libopencm3/stm32/exti.h>
//#include <libopencm3/stm32/timer.h>
//#include <libopencm3/cm3/scb.h>
#include <libopencm3/stm32/spi.h>


#include <stddef.h> // size_t
//#include <math.h> // nanf
//#include <stdio.h>


#include "buffer.h"
#include "miniprintf2.h"
#include "usart2.h"
#include "util.h"

#include "mcp3208.h"
#include "w25.h"
// #include "common.h" // GET RID OF THIS



/*

  GPIOx_BSRR
    This is GPIO Bit Set/Reset Register. When you want to set or reset the
    particular bit or pin, you can use this register.

  GPIOx_ODR
    This is the Output Data Register. [...] this register is used to set the value
    to the GPIO pin.
  https://embetronicx.com/tutorials/microcontrollers/stm32/stm32-gpio-tutorial/#GPIOx_BSRR


void gpio_set(uint32_t gpioport, uint16_t gpios)
 {
         GPIO_BSRR(gpioport) = gpios;
 }

void  gpio_clear(uint32_t gpioport, uint16_t gpios)
 {
         GPIO_BSRR(gpioport) = (gpios << 16);
 }

  So BSRR. doesn't need bit masks, or to read values.  to set individual values.
    instead it uses low bytes to set.
    and high bytes to clear

  #define GPIO_BSRR  (     port )     MMIO32((port) + 0x18)
  #define MMIO32  (     addr )     (*(volatile uint32_t *)(addr))

toggle does a read eg. stores in port ..

void gpio_toggle(uint32_t gpioport, uint16_t gpios)
 {
         uint32_t port = GPIO_ODR(gpioport);
         GPIO_BSRR(gpioport) = ((port & gpios) << 16) | (~port & gpios);
 }


  so for setting it's just    current | val
     and clear                ~(~current | val)

   0b01111
  ~0b10010
val     1
  |  10010
  ~  01101

  but then how are they combined?
    yes. apply set(), take output, then apply clear()
  -----------------------

  we want to get analog power - to test dac. i think. in priority.


void gpio_port_write(uint32_t gpioport, uint16_t data)
 {
         GPIO_ODR(gpioport) = data;
 }
  write uses a different register.
*/

static void spi_fpga_reg_setup(uint32_t spi)
{
  // the fpga as a spi slave.

  spi_init_master(
    spi,
    SPI_CR1_BAUDRATE_FPCLK_DIV_4,
    SPI_CR1_CPOL_CLK_TO_0_WHEN_IDLE,
    SPI_CR1_CPHA_CLK_TRANSITION_2,    // 2 == falling edge
    SPI_CR1_DFF_8BIT,
    SPI_CR1_MSBFIRST
  );

  spi_disable_software_slave_management( spi);
  spi_enable_ss_output(spi);
}


static uint32_t spi_xfer_register_16_here(uint32_t spi, uint32_t r)
{
  uint8_t a = spi_xfer( spi, (r >> 8) & 0xff  );
  uint8_t b = spi_xfer( spi, r & 0xff  );

  return (a << 8) + b; // msb first
}





static uint16_t spi_fpga_xfer(uint32_t spi, uint32_t r)
{
  spi_special_flag_clear(spi);
  spi_enable(spi);
  uint16_t ret = spi_xfer_register_16_here(spi, r );
  spi_disable(spi);
  spi_special_flag_set(spi);
  return ret;
}


// OK. we are using this to write the spi muxing register with 8 bits.
// if need more bits then it's problematic

static uint16_t spi_fpga_write( uint32_t spi, uint8_t r, uint8_t v)
{
  // change name to xfer also. I think.
  uint16_t ret = spi_fpga_xfer(spi, r << 8 | v );
  return ret;
}



static void spi_fpga_reg_set( uint32_t spi, uint8_t r, uint8_t v)
{
  spi_fpga_write(spi, r, (v & 0xF)); // SHOULD BE & 0xf
}

static void spi_fpga_reg_clear( uint32_t spi, uint8_t r, uint8_t v)
{
  spi_fpga_write(spi, r, v << 4);
}

// OK. don't think we need a separate hardware register...

static void spi_fpga_reg_write( uint32_t spi, uint8_t r, uint8_t v)
{

  uint8_t x =  (~v << 4) | (v & 0xF );
  // usart_printf("spi %d  r %d\n", spi, r);
  // usart_printf("reg_write value val %d  %8b\n", v, x );
  spi_fpga_write(spi, r, x );

}








//////////////////////////////////////////////

// REGISTER_DAC?

// what the hell is happening...

// RENAME OR = output register and SRR set reset register

// OK. Now is there a way to do both...

#define LED_REGISTER  7
#define LED1 (1<<0)    // D38
#define LED2 (1<<1)    // D37
#define LED3 (1<<2)    // D37
#define LED4 (1<<3)    // D37


#define SPI_MUX_REGISTER  8
#define SPI_MUX_ADC03     (1<<0)
#define SPI_MUX_DAC       (1<<1)
#define SPI_MUX_FLASH     (1<<2)

#define DAC_REGISTER  9
#define DAC_LDAC      (1<<0)
#define DAC_UNI_BIP_A (1<<1)
#define DAC_UNI_BIP_B (1<<2)
#define DAC_RST       (1<<3)

// rename RAILS_REG or REG_RAILS consistent with verilog?
#define RAILS_REGISTER  10
#define RAILS_LP15V   (1<<0)
#define RAILS_LP30V   (1<<1)
#define RAILS_LP60V   (1<<2)
#define RAILS_OE      (1<<3)


// one bit


static void mux_fpga(uint32_t spi)
{
  spi_fpga_reg_setup(spi);
}


static void mux_adc03(uint32_t spi)
{

  usart_printf("mux_adc03\n");
  spi_fpga_reg_setup(spi);
  //spi_fpga_write(spi, SPI_MUX_REGISTER, SPI_MUX_ADC03 );


  spi_fpga_reg_write(spi, SPI_MUX_REGISTER, SPI_MUX_ADC03);

  spi_mcp3208_setup(spi);
}

static void mux_flash(uint32_t spi)
{

  usart_printf("mux_flash\n");
  spi_fpga_reg_setup(spi);
  // spi_fpga_write(spi, SPI_MUX_REGISTER, SPI_MUX_FLASH );

  spi_fpga_reg_write(spi, SPI_MUX_REGISTER, SPI_MUX_FLASH);

  spi_w25_setup(spi);
}



#define SPI_ICE40       SPI1


static void soft_500ms_update(void)
{
  // blink mcu led
  led_toggle();

  ////////
  uint32_t spi = SPI_ICE40;


  // blink led
  static int count = 0;

  // setup spi1 for register control.
  usart_printf("-----------\n");

  // usart_printf( "spi is %d\n", spi );

  mux_fpga(spi);

  // OK. this looks like it's working...

#if 1
  // clear
  spi_fpga_reg_clear(spi, LED_REGISTER, LED1);

  if(count % 2 == 0  )
    spi_fpga_reg_set(spi, LED_REGISTER, LED2);
  else
    spi_fpga_reg_clear(spi, LED_REGISTER, LED2);
#endif


#if 0
  // it's only 4 bits...
  spi_fpga_reg_write(spi, LED_REGISTER, count);
#endif



  // static void spi_fpga_reg_write( uint32_t spi, uint8_t r, uint8_t v)



  mux_adc03(spi);
  float lp15v = spi_mcp3208_get_data(spi, 0) * 0.92 * 10.;
  float ln15v = spi_mcp3208_get_data(spi, 1) * 0.81 * 10.;

  usart_printf("lp15v %f    ln15v %f\n", lp15v, ln15v);



  mux_flash(spi);
  spi_w25_get_data(spi);

  /////////////////
  // pull dac rst lo then high
  mux_fpga(spi);

  spi_fpga_reg_clear(spi, DAC_REGISTER, DAC_RST);
  msleep(20);
  spi_fpga_reg_set( spi, DAC_REGISTER, DAC_RST);
  msleep(20);


  // change name REG_DAC not DAC_REGISTER etc

  // pull high... turn off.

  // turn rails off
  spi_fpga_reg_clear(spi, RAILS_REGISTER, RAILS_LP15V | RAILS_LP30V | RAILS_LP60V);
  // turn rails output enable on
  spi_fpga_reg_clear(spi, RAILS_REGISTER, RAILS_OE);


  /*
    weird.
      fpga poweron , with no mcu, and it comes up high. as expected.
      fpga power,   with mcu - but no write and its lo?

    AHHH. It's clever
      if fpga gpio comes up high - OE - is disabled. so out is low.
      if fpga gpio comes up lo   - buffer is disabled so out low.

      there seems to be a 50-100uS spike though on creset.
      hopefully too short a time to turn on opto coupler.
  */


  count++;
}





static void loop(void)
{

  static uint32_t soft_500ms = 0;

  while(true) {

    // EXTREME - can actually call update at any time, in a yield()...
    // so long as we wrap calls with a mechanism to avoid stack reentrancy
    // led_update(); in systick.


    // pump usart queues
    usart_input_update();
    usart_output_update();


    // 500ms soft timer
    if( system_millis > soft_500ms) {
      // soft_500ms = system_millis + 1000;
      soft_500ms = system_millis + 500;
      soft_500ms_update();
    }

  }

}




int main(void)
{
  // high speed internal!!!

  systick_setup(16000);


  // clocks
  rcc_periph_clock_enable(RCC_SYSCFG); // maybe required for external interupts?

  // LED
  rcc_periph_clock_enable(RCC_GPIOE);

  // USART
  rcc_periph_clock_enable(RCC_GPIOA);     // f407
  // rcc_periph_clock_enable(RCC_GPIOB); // F410
  rcc_periph_clock_enable(RCC_USART1);


  // spi / ice40
  rcc_periph_clock_enable(RCC_SPI1);

  //////////////////////
  // setup

  // led
  led_setup();


  // usart
  usart_setup_();

  ////////////////
  spi1_port_setup();
  spi1_special_gpio_setup();


  ////////////////////


  usart_printf("\n--------\n");
  usart_printf("starting\n");
  // usart_printf("size %d\n", sizeof(fbuf) / sizeof(float));


  loop();

	for (;;);
	return 0;
}


