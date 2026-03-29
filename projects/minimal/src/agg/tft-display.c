
/*

*/


#include <string.h>     // memcpy
#include <assert.h>



#include <agg/tft-display.h>




void tft_display_update( tft_display_t *tft_display)
{
  assert( tft_display && tft_display->magic == TFT_DISPLAY_MAGIC);

  if( tft_display->update)
    tft_display->update( tft_display);
}


void tft_display_update_data( tft_display_t *tft_display, data_t *data)
{
  assert( tft_display && tft_display->magic == TFT_DISPLAY_MAGIC);

  if( tft_display->update_data)
    tft_display->update_data( tft_display, data);

}

void tft_display_update_500ms( tft_display_t *tft_display)
{
  assert( tft_display && tft_display->magic == TFT_DISPLAY_MAGIC);

  if( tft_display->update_500ms)
    tft_display->update_500ms( tft_display);

}





static void tft_display_update_( tft_display_t *tft_display)
{
  assert( tft_display && tft_display->magic == TFT_DISPLAY_MAGIC);

  // do nothing

}




bool tft_display_repl_statement( tft_display_t *tft_display,  const char *cmd)
{
  assert( tft_display && tft_display->magic == TFT_DISPLAY_MAGIC);

  if(strcmp(cmd, "tft_test2") == 0)
    tft_display->update = tft_test2;

  else if(strcmp(cmd, "tft_test3") == 0)
    tft_display->update = tft_test3;

  else if(strcmp(cmd, "tft_none") == 0)
    tft_display->update = tft_display_update_;



  else
    return 0;

  return 1;

}


void tft_display_init( tft_display_t *tft_display, tft_t *tft, volatile uint32_t *system_millis)
{

  memset( tft_display, 0, sizeof( tft_display_t));

  tft_display->magic = TFT_DISPLAY_MAGIC;
  tft_display->tft = tft;
  tft_display->system_millis = system_millis;

  tft_display->update = tft_display_update_;    // set to non


}


