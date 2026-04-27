

#pragma once

#ifdef __cplusplus
extern "C" {
#endif



#include <stdint.h>
#include <stdbool.h>
#include <assert.h>


#define TFT_MAGIC  82342351


typedef struct tft_t tft_t;

typedef struct tft_t
{
  uint32_t  magic;

  uint32_t  fmc_addr;     // FMC_MY_BASE |  FMC_A19
  uint32_t  fmc_cd;       // command/data bit. FMC_A16.


  // consider - none of these really need to be polymorphic
  // except allows run multiple instances.
  void (*tft_gpio_port_configure)( tft_t *);
  bool (*tft_getTear)( tft_t *);                  // renae
  void (*tft_reset)( tft_t *, bool val );

/*
  // remmember the bits p
  // in pix
  uint32_t  width;          // pix
  uint32_t  height_bytes;   // hieght in bytes is / 8.

  //
  bool page;
*/



} tft_t;





inline void tft_write_cmd( tft_t *tft, uint16_t cmd)
{

  *((volatile uint16_t *)  (tft->fmc_addr )) = cmd;
}

inline void tft_write_data( tft_t *tft, uint16_t data)
{
  *((volatile uint16_t *)  (tft->fmc_addr | tft->fmc_cd)) = data;
}

inline uint16_t tft_read_data( tft_t *tft)
{
  return *((volatile uint16_t *)  (tft->fmc_addr | tft->fmc_cd));
}



inline bool tft_get_tear( tft_t *tft)
{
  assert( tft);
  return tft->tft_getTear( tft);
}




/////////////

// TODO rename rgb_pack565
uint16_t packRGB565(  uint16_t r, uint16_t g, uint16_t b);



void tft_init( tft_t *tft, volatile uint32_t *system_millis);

void tft_read_ddb( tft_t *tft);

void tft_set_scrollstart( tft_t *tft, uint16_t y);

void tft_test_fill( tft_t *tft);

void tft_set_xy( tft_t *tft,  uint16_t x1,  uint16_t y1,uint16_t x2,  uint16_t y2 );

void tft_fill_rect( tft_t *tft,  uint16_t x1,  uint16_t y1,uint16_t x2,  uint16_t y2, uint16_t c );





/*
  cannot interleave and change on the fly.
  since governs 1963 memory to screen. not blt operations.
*/

// conventional
void tft_set_origin_top_left(  tft_t *tft);

// cartesion/ fonts/ postscript
void tft_set_origin_bottom_left( tft_t *tft );


/*
  think - these are software/not hardware
  consider remove
*/
uint16_t tft_get_tear_effect_status( tft_t *tft);
void tft_set_tear_on( tft_t *tft);




#ifdef __cplusplus
}
#endif



/*
  antigrain font loading. appears to use flip_y on load
  https://coconut2015.github.io/agg-tutorial/tutorial__font__1_8cpp_source.htm

  i think we can use top-left. ok.
*/


