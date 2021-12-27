/*

  - dummy text - to test draw time.
  - coroutines.
  - spi slave - test on other board
  - do storage for actual horizontal.



  ------------
  nix-shell ~/devel/nixos-config/examples/arm.nix
  make

  serial,
  rlwrap -a picocom -b 115200 /dev/ttyUSB0

  usb
  screen /dev/ttyACM0 115200

  screen
  openocd -f openocd.cfg
  rlwrap nc localhost 4444  # in new window

  reset halt ; flash write_image erase unlock /home/me/devel/stm32/stm32f4/projects/control-panel-2/main.elf; sleep 1; reset run

  *********
  with gnu sprintf, and floating point code, this still fits in 27k, tested by editing f410.ld.  good!!
  *********
  -----------------------------
  one mcu or two.

  two two.
    control paenl just issue spi commands and return core structure.
    spi slave - just handle as interupt.
        eg. receive a byte, decide what to do. could be just using spi_xfer()

    - any function in core - can be exposed as spi command. if required.
    - just enable an interupt on CS. then in the isr - handle it.

    - nss pin is set as an interupt input.


*/


#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/spi.h>

#include <libopencm3/cm3/nvic.h>
#include <libopencm3/cm3/systick.h>



#include <libopencm3/usb/usbd.h>


#include <setjmp.h>
#include <stddef.h> // size_t
//#include <math.h> // nanf
//#include <stdio.h>
#include <string.h>   // memset


#include "cbuffer.h"
#include "usart2.h"
#include "util.h"
#include "assert.h"
#include "cdcacm.h"


#include "str.h"  //format_bits




typedef struct app_t
{
  CBuf console_in;
  CBuf console_out;

  usbd_device *usbd_dev ;

} app_t;


static void update_console_cmd(app_t *app)
{

  if( !cBufisEmpty(&app->console_in) && cBufPeekLast(&app->console_in) == '\r') {

    // usart_printf("got CR\n");

    // we got a carriage return
    static char tmp[1000];

    size_t nn = cBufCount(&app->console_in);
    size_t n = cBufCopyString(&app->console_in, tmp, ARRAY_SIZE(tmp));
    ASSERT(n <= sizeof(tmp));
    ASSERT(tmp[n - 1] == 0);
    ASSERT( nn == n - 1);

    // chop off the CR to make easier to print
    ASSERT(((int) n) - 2 >= 0);
    tmp[n - 2] = 0;

    // TODO first char 'g' gets omitted/chopped here, why? CR handling?
    usart_printf("got command '%s'\n", tmp);

    // process_cmd(app, tmp);
  }
}


//////////////////////////////////
// WIRING CS->CS,CLK->CLK,MOSI->MOSI,MISO->MISO.

//////////////////////////////////
// MASTER

// spi1 tx

#define SPI1_PORT    GPIOA
#define SPI1_CS      GPIO4
#define SPI1_SCLK    GPIO5
#define SPI1_MISO    GPIO6
#define SPI1_MOSI    GPIO7


static void spi1_port_setup(void)
{
  uint32_t out = SPI1_CS | SPI1_SCLK | SPI1_MOSI;
  uint32_t all = out | SPI1_MISO ;

  usart_printf("spi1 setup spi\n");

  // spi alternate function
  gpio_mode_setup(SPI1_PORT, GPIO_MODE_AF, GPIO_PUPD_NONE, all);
  gpio_set_af(SPI1_PORT, GPIO_AF5, all );
  gpio_set_output_options(SPI1_PORT, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, out );
}


static void spi_setup(uint32_t spi)
{
  assert(spi == SPI1);

  // rcc_periph_clock_enable(RCC_SPI2);
  spi_init_master(
    spi,
    SPI_CR1_BAUDRATE_FPCLK_DIV_4,
    SPI_CR1_CPOL_CLK_TO_0_WHEN_IDLE,
    SPI_CR1_CPHA_CLK_TRANSITION_2,    // 2 == falling edge
    SPI_CR1_DFF_8BIT,
    SPI_CR1_MSBFIRST
  );

  spi_disable_software_slave_management(spi);
  spi_enable_ss_output(spi);
}



//////////////////////////////////////////
// SLAVE
// spi2

// https://community.st.com/s/question/0D50X0000A4neCu/spi-communication-between-two-stm32-gives-different-values-after-reset-button-pressed


#define SPI2_PORT    GPIOB
#define SPI2_CS      GPIO12
#define SPI2_SCLK    GPIO13
#define SPI2_MISO    GPIO14
#define SPI2_MOSI    GPIO15




static void spi2_port_setup(void)
{
  // MOSI here is an input.
  // CS is input. CLK is input. MISO is *only* output

  uint32_t all = SPI2_CS | SPI2_SCLK | SPI2_MISO | SPI2_MOSI;

  gpio_mode_setup(SPI2_PORT, GPIO_MODE_AF, GPIO_PUPD_NONE, all);
  gpio_set_af(SPI2_PORT, GPIO_AF5, all );
  gpio_set_output_options(SPI2_PORT, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ,  SPI2_MISO );

}


void spi2_isr(void)
{
  uint8_t i = spi_xfer(SPI2, 0);

  usart_printf("spi2 interupt !!!! %u\n", i);
}

static void spi_slave_setup(uint32_t spi)
{
  usart_printf("spi slave setup \n");
  assert(spi == SPI2);

  // spi interupt
  nvic_enable_irq(NVIC_SPI2_IRQ);

  spi_set_slave_mode(spi);
/*
  spi_set_baudrate_prescaler(spi, SPI_CR1_BR_FPCLK_DIV_8); // SPI_CR1_BAUDRATE_FPCLK_DIV_4,

  spi_set_full_duplex_mode(spi);
  spi_set_clock_polarity_1(spi);
  spi_set_clock_phase_1(spi);
  spi_set_dff_8bit(spi);
  spi_send_msb_first(spi);
  spi_enable_software_slave_management(spi);
*/

  spi_set_clock_polarity_1(spi);
  spi_set_clock_phase_1(spi);


  spi_enable_rx_buffer_not_empty_interrupt(spi);  // WILL GIVE US AN INTERUPT ?
  spi_enable(spi);

}



// OK. we need the interupt on the slave to transfer.



static void loop(app_t *app)
{
  /*
    loop() subsumes update()
  */

  // TODO move to app_t structure?.
  static uint32_t soft_500ms = 0;

  while(true) {


		usbd_poll(app->usbd_dev);

    update_console_cmd(app);

    // usart_output_update(); // shouldn't be necessary


    // 500ms soft timer. should handle wrap around
    if( (system_millis - soft_500ms) > 500) {
      soft_500ms += 500;
      led_toggle();


      usart_printf("xfer on spi1\n");

      spi_enable( SPI1 );
      uint8_t val = spi_xfer(SPI1, 0xff );
      UNUSED(val);
      spi_disable( SPI1 );


    }

  }
}




static char buf_console_in[1000];
static char buf_console_out[1000];


static app_t app;






////////////////////
// I don't see that we can transfer control.
// eg. the write will pause until it has sent.
// and only then transfer to the read.

////////////////////



/*
  Ok, vertical is ok.   but we want to flip the horizontal origin.
  to draw from top left. that shoudl be good for fillRect, and for agg letter.
*/

int main(void)
{

  // required for usb
	// rcc_clock_setup_pll(&rcc_hse_8mhz_3v3[RCC_CLOCK_3V3_84MHZ] );  // stm32f411  upto 100MHz. works stm32f407 too.
	rcc_clock_setup_pll(&rcc_hse_8mhz_3v3[RCC_CLOCK_3V3_168MHZ] );  // stm32f407

  /*
  // http://libopencm3.org/docs/latest/stm32f4/html/f4_2rcc_8h.html

    see here for rcc.c example, defining 100MHz, using 25MHz xtal.
      https://github.com/insane-adding-machines/unicore-mx/blob/master/lib/stm32/f4/rcc.c
  */

  // clocks
  rcc_periph_clock_enable(RCC_SYSCFG); // maybe required for external interupts?

  // LED
  rcc_periph_clock_enable(RCC_GPIOA); // f410/f411 led.
  rcc_periph_clock_enable(RCC_GPIOB); // f410/f411 led.

  // USART
  rcc_periph_clock_enable(RCC_GPIOB); // F410/f411
  rcc_periph_clock_enable(RCC_USART1);


  // USB
	rcc_periph_clock_enable(RCC_OTGFS);



  // spi1
  rcc_periph_clock_enable(RCC_SPI1);

  // spi2
  rcc_periph_clock_enable(RCC_SPI2);


  //////////////////////
  // setup


  // 16MHz. from hsi datasheet.
  // systick_setup(16000);
  // systick_setup(16000);
  // systick_setup(84000);  // 84MHz.
  systick_setup(168000);


  // led
  led_setup();


  memset(&app, 0, sizeof(app_t));

  // uart/console
  cBufInit(&app.console_in,  buf_console_in, sizeof(buf_console_in));
  cBufInit(&app.console_out, buf_console_out, sizeof(buf_console_out));

  // usart_setup_gpio_portA();
  usart_setup_gpio_portB();

  usart_set_buffers(&app.console_in, &app.console_out);

  // setup print
  // usart_printf_set_buffer()
  usart_printf_init(&app.console_out);


  ////////////////////////////
  // usb
  // might be better to pass as handler?
	app.usbd_dev = usb_setup();
  ASSERT(app.usbd_dev);



#if 0
  fsmc_gpio_setup();

  fsmc_setup(1);
  tft_reset();

  LCD_Init();
  LCD_SetTearOn();
  // LCD_TestFill();


  xpt2046_gpio_setup();
  xpt2046_spi_port_setup();
  xpt2046_spi_setup( XPT2046_SPI );

  xpt2046_reset( XPT2046_SPI);
#endif


  usart_printf("\n--------");
  usart_printf("\nstarting\n");



  usart_printf("\n--------\n");
  usart_printf("starting loop\n");
  usart_printf("sizeof bool   %u\n", sizeof(bool));
  usart_printf("sizeof float  %u\n", sizeof(float));
  usart_printf("sizeof double %u\n", sizeof(double));

  usart_printf("\n--------\n");

  spi1_port_setup();
  spi_setup(SPI1 );


  spi2_port_setup();
  spi_slave_setup(SPI2 );




  loop(&app);
}

// put the code after the function. to prevent inlining.



