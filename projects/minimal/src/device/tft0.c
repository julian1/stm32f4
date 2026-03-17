


#include <stdio.h>
#include <assert.h>

#include <libopencm3/stm32/gpio.h>

#include <device/tft0.h>
#include <lib2/util.h>           // UNUSED
#include <string.h>



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

  // ssd1963 tear irq. bodge wire.
  gpio_mode_setup( TEAR_PORT, GPIO_MODE_INPUT, GPIO_PUPD_NONE, TEAR_IRQ);

}



static bool tft_getTear( tft_t *tft)
{
  assert( tft && tft->magic == TFT_MAGIC);

  // TODO better prefix name. tft_get_tear() ?
  // hi tft stopped, means we should draw .
  // return gpio_get(TFT_GPIO_PORT, TFT_T_IRQ) & (0x01 << 3);
  return gpio_get( TEAR_PORT, TEAR_IRQ) != 0 ;
}



static void tft_reset( tft_t *tft, bool val )
{
  assert( tft && tft->magic == TFT_MAGIC);

  if( val)
    gpio_set( TFT_GPIO_PORT, TFT_REST);
  else
    gpio_clear( TFT_GPIO_PORT, TFT_REST);



  // backlight must be on, to see anything.
  gpio_set( TFT_GPIO_PORT, TFT_LED_A ); // turn on backlight. works!!!
}



void tft0_init( tft_t *tft)
{
  memset( tft, 0, sizeof( tft_t));

  tft->magic        = TFT_MAGIC;

  tft->fmc_addr     = FMC_MY_BASE | FMC_A19;
  tft->fmc_cd       = FMC_A16;

  // tft->width        = 128;    // 16 bytes
  // tft->height_bytes = 8;     //


  tft->tft_gpio_setup = tft_gpio_setup;
  tft->tft_getTear    = tft_getTear;
  tft->tft_reset      = tft_reset;

}






#if 0
static void tft_reset( tft_t *tft, volatile uint32_t *system_millis)
{

  printf("pull reset lo\n");
  // reset. pull lo then high.
  gpio_clear( TFT_GPIO_PORT, TFT_REST);

  msleep( 20, system_millis);
  printf("pull reset hi\n");
  gpio_set( TFT_GPIO_PORT, TFT_REST);
  msleep( 20, system_millis);

  // backlight must be on, to see anything.
  gpio_set( TFT_GPIO_PORT, TFT_LED_A ); // turn on backlight. works!!!
}
#endif

#if 0
typedef struct tft_h
{
  uint32_t  magic;

  uint32_t  fmc_addr;     // FMC_MY_BASE |  FMC_A18
  uint32_t  fmc_cd;       // command/data bit. FMC_A16.   change name vmc_cd_bit.  it is bit in an address not address

/*
  // remmember the bits p
  // in pix
  uint32_t  width;          // pix
  uint32_t  height_bytes;   // hieght in bytes is / 8.

  //
  bool page;
*/

  void (*tft_gpio_setup)( tft_t *, void);
  bool (*tft_getTear)( tft_t *, void);

  void (*tft_reset)( tft_t *, bool val );
} tft_h;

#endif


