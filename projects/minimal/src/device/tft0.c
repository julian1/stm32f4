


#include <stdio.h>
#include <assert.h>
#include <string.h>

#include <libopencm3/stm32/gpio.h>

#include <device/tft0.h>
#include <lib2/util.h>           // UNUSED



#define FMC_MY_BASE 0x60000000
#define FMC_A16 (1<<(16+1))
#define FMC_A17 (1<<(17+1))
#define FMC_A18 (1<<(18+1))
#define FMC_A19 (1<<(19+1))

/*
  feb. 2026.
  ssd1963
    reset PD6
    led is PD2.
    tear irq. PB1.

*/


// this is device specific
#define TFT_GPIO_PORT       GPIOD
#define TFT_REST            GPIO6
#define TFT_LED_A           GPIO2   // unused, due to jumper.



// control-panel-2 / spectra184
#define TEAR_PORT           GPIOB
#define TEAR_IRQ            GPIO1




static void tft_gpio_setup( tft_t *tft)
{
  assert( tft && tft->magic == TFT_MAGIC);

  // led and reset
  gpio_mode_setup( TFT_GPIO_PORT, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, TFT_LED_A |  TFT_REST);
  gpio_set_output_options(TFT_GPIO_PORT, GPIO_OTYPE_PP, GPIO_OSPEED_100MHZ, TFT_LED_A |  TFT_REST);

  // ssd1963 tear irq. bodge wire.
  gpio_mode_setup( TEAR_PORT, GPIO_MODE_INPUT, GPIO_PUPD_NONE, TEAR_IRQ);

}



static bool tft_getTear( tft_t *tft)
{
  UNUSED( tft);

  // mar. 2026. dont block.
  return 0;

#if 0
  assert( tft && tft->magic == TFT_MAGIC);

  // TODO better prefix name. tft_get_tear() ?
  // hi tft stopped, means we should draw .
  // return gpio_get(TFT_GPIO_PORT, TFT_T_IRQ) & (0x01 << 3);
  return gpio_get( TEAR_PORT, TEAR_IRQ) != 0 ;
#endif
}


static void tft_reset( tft_t *tft, bool val )
{
  assert( tft && tft->magic == TFT_MAGIC);

  printf("tft reset %u\n", val );

  if( val)
    gpio_set( TFT_GPIO_PORT, TFT_REST);
  else
    gpio_clear( TFT_GPIO_PORT, TFT_REST);



  // backlight must be on, to see anything.
  gpio_set( TFT_GPIO_PORT, TFT_LED_A ); // turn on backlight. works!!!
}


void tft0_init( tft_t *tft)
{
  assert( tft);

  const tft_t temp = {

    .magic        = TFT_MAGIC,

    .fmc_addr     = FMC_MY_BASE | FMC_A19,
    .fmc_cd       = FMC_A16,

    // .width        = 128,    // 16 bytes
    // .height_bytes = 8,     //

    .tft_gpio_setup = tft_gpio_setup,
    .tft_getTear    = tft_getTear,
    .tft_reset      = tft_reset,
  };

  memcpy( tft, &temp, sizeof( tft_t));
}







#if 0
  // led is required....
  gpio_mode_setup(TFT_GPIO_PORT, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, TFT_LED_A |  TFT_REST);

  // ssd1963 tear irq. bodge wire.
  gpio_mode_setup(TEAR_PORT, GPIO_MODE_INPUT, GPIO_PUPD_NONE, TEAR_IRQ);
#endif



#if 0

bool getTear()
{
  // TODO better prefix name. tft_get_tear() ?
  // hi tft stopped, means we should draw .
  // return gpio_get(TFT_GPIO_PORT, TFT_T_IRQ) & (0x01 << 3);
  return gpio_get(TEAR_PORT, TEAR_IRQ) != 0 ;
}



void tft_reset(void )
{

  usart1_printf("pull reset lo\n");
  // reset. pull lo then high.
  gpio_clear( TFT_GPIO_PORT, TFT_REST);
  msleep(20);
  usart1_printf("pull reset hi\n");
  gpio_set( TFT_GPIO_PORT, TFT_REST);
  msleep(20);

  // backlight must be on, to see anything.
  gpio_set( TFT_GPIO_PORT, TFT_LED_A ); // turn on backlight. works!!!
}

#endif



#if 0
#define TFT_GPIO_PORT       GPIOE
#define TFT_REST            GPIO1
#define TFT_LED_A           GPIO2   // unused, due to jumper.

#endif


/*
// Mapped to spi2 nss2 unused. using bodge wire.  ssd1963 TE tear interrupt pin.
#define TEAR_PORT           GPIOB
#define TEAR_IRQ            GPIO9
*/

#if 0
// control-panel-2 / spectra184
#define TEAR_PORT           GPIOE
#define TEAR_IRQ            GPIO3

#endif



