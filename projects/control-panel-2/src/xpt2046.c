

#include <libopencm3/stm32/gpio.h>
#include <libopencm3/cm3/nvic.h>
#include <libopencm3/stm32/exti.h>
#include <libopencm3/stm32/spi.h>



#include "xpt2046.h"
#include "util.h" // usart_printf
#include "assert.h"


//
#define XPT2046_IRQ_PORT    GPIOE
#define XPT2046_IRQ_PIN     GPIO3



void exti3_isr(void)
{
  exti_reset_request(EXTI3);

  // usart_printf("got interupt on pe3 \n");
}




void xpt2046_gpio_setup()
{

  gpio_mode_setup(XPT2046_IRQ_PORT, GPIO_MODE_INPUT, GPIO_PUPD_NONE, XPT2046_IRQ_PIN);

  assert(EXTI3 == GPIO3);
  assert(XPT2046_IRQ_PIN == EXTI3);


  // ie. use exti3 for pa3
  nvic_enable_irq(NVIC_EXTI3_IRQ);

  exti_select_source(EXTI3, XPT2046_IRQ_PORT);
  exti_set_trigger(EXTI3 , EXTI_TRIGGER_FALLING);
  exti_enable_request(EXTI3);

}

/////////////////////////////////////

/*
  issue - spi hung - we were not enabling/disabling spi around xfer.

*/

#define ADC_PORT    GPIOB    // change name ADC_GPIO_PORT
// #define ADC_SPI     SPI2

#define ADC_CS      GPIO12      // SPI2_NSS
#define ADC_SCLK    GPIO13      // SPI2_SCK
#define ADC_MISO    GPIO14      // SPI2_MISO
#define ADC_MOSI    GPIO15      // SPI2_MISO




// why pass spi2...

// TODO. 
// RENAME spi1_port_setup()
// because it's completely generic.
// TODO. also last argument of gpio_set_output_options() should be 'out'

void xpt2046_spi_port_setup()
{
  uint32_t out = ADC_CS | ADC_SCLK | ADC_MOSI;
  uint32_t all = out |  ADC_MISO ;

  usart_printf("adc setup spi\n");

  // spi alternate function
  gpio_mode_setup(ADC_PORT, GPIO_MODE_AF, GPIO_PUPD_NONE, all);

  // af 5 for spi2 on PB
  gpio_set_af(ADC_PORT, GPIO_AF5, all );

  // OK.. THIS MADE SPI WORK AGAIN....
  // note, need harder edges for signal integrity. or else different speed just helps suppress parasitic components
  // see, https://www.eevblog.com/forum/microcontrollers/libopencm3-stm32l100rc-discovery-and-spi-issues/
  gpio_set_output_options(ADC_PORT, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, ADC_CS | ADC_SCLK |  ADC_MOSI /* FIXME */); 

}




void xpt2046_spi_setup( uint32_t spi)
{
  // taking the spi argument makes all actions a lot more generic.

  assert(spi == SPI2);

  spi_init_master(
    spi,
    SPI_CR1_BAUDRATE_FPCLK_DIV_16,   // slow... 16Mhz / 16 = 1Mhz. for 50ksps
    SPI_CR1_CPOL_CLK_TO_0_WHEN_IDLE,
    SPI_CR1_CPHA_CLK_TRANSITION_1,    // 1 == rising edge
    SPI_CR1_DFF_8BIT,
    SPI_CR1_MSBFIRST
  );

  spi_disable_software_slave_management( spi );
  spi_enable_ss_output( spi);
}




void xpt2046_reset( uint32_t spi)
{
  /* makes no sense that this hangs.
    master should be doing everything....
  */
  UNUSED(spi);


  usart_printf("before xpt reset\n");

  // spi_xfer_16(spi, 0x80 );
  // spi_xfer_24(spi, 0x80 << 16 );

  usart_printf("done xpt reset\n");

}



// very strange it's actually locking up...

static uint16_t ts_get_data16(uint32_t spi, uint8_t command)
{
    assert(spi == SPI2);

    spi_xfer(spi, command);
    uint16_t res1 = spi_xfer(spi, 0x00);
    uint16_t res2 = spi_xfer(spi, 0x00);
    return ((res1 << 8) | (res2 && 0xFF)) >> 4;
}



#define Y 0x90
#define X 0xD0



#define TS_SB       (1 << 7)
#define TS_A2       (1 << 6)
#define TS_A1       (1 << 5)
#define TS_A0       (1 << 4)
#define TS_MODE8    (1 << 3)
#define TS_MODE12   (0 << 3)
#define TS_SER      (1 << 2)
#define TS_DFR      (0 << 2)
#define TS_PD1      (1 << 1)
#define TS_PD0      (1 << 0)

#define TS_COMM_Y_SPOS    (TS_MODE12 | TS_SER | TS_SB | TS_A0)
#define TS_COMM_X_SPOS    (TS_MODE12 | TS_SER | TS_SB | TS_A2 | TS_A0)
#define TS_COMM_Y_DPOS    (TS_MODE12 | TS_SB | TS_A0)
#define TS_COMM_X_DPOS    (TS_MODE12 | TS_SB | TS_A2 | TS_A0)
#define TS_COMM_Z1_POS    (TS_MODE12 | TS_SB | TS_A1 | TS_A0)
#define TS_COMM_Z2_POS    (TS_MODE12 | TS_SB | TS_A2)
#define TS_COMM_TEMP      (TS_MODE12 | TS_SB)

#define TS_EVAL_COUNT 5

/*
uint16_t ts_get_x_raw(void) {
    int16_t res = 0;
    for (uint8_t i = 0; i < TS_EVAL_COUNT; i++) {
        res += ts_get_data16(TS_COMM_X_DPOS);
    }
    return res / TS_EVAL_COUNT;
}
*/
void xpt2046_read()
{
  spi_enable( XPT2046_SPI );

  assert(TS_COMM_X_DPOS == X );  // OK.

  uint16_t x = ts_get_data16( XPT2046_SPI, TS_COMM_X_DPOS );


  spi_disable( XPT2046_SPI );

  usart_printf("xpt2046   %u\n", x );
}




/*
bool xpt2046_get_irq()
{
  // polling function.
  usart_printf("isr_called %u\n", isr_called);
  return gpio_get(XPT2046_IRQ_PORT, XPT2046_IRQ_PIN) != 0 ;
}
*/

/*
static uint32_t spi_xfer_16(uint32_t spi, uint16_t val)
{

  // taking spi argument makes all actions a lot more generic.

  spi_enable( spi );
  uint8_t a = spi_xfer(spi, (val >> 8 ) & 0xff );
  uint8_t b = spi_xfer(spi, val & 0xff);
  spi_disable( spi);

  return (a << 8) + b;
}


static uint32_t spi_xfer_24(uint32_t spi, uint32_t val)
{
  uint8_t a = spi_xfer(spi, (val >> 16) & 0xff );
  uint8_t b = spi_xfer(spi, (val >> 8) & 0xff);
  uint8_t c = spi_xfer(spi, val & 0xff);

  return (a << 16) + (b << 8) + c;
}
*/
