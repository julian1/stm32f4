


#include <string.h>     // memcpy
#include <assert.h>



#include <agg/test.h>


void agg_test_init( agg_test_t *agg_test, tft_t *tft, volatile uint32_t *system_millis)
{

  memset( agg_test, 0, sizeof( agg_test_t));

  agg_test->magic = AGG_TEST_MAGIC;
  agg_test->tft = tft;
  agg_test->system_millis = system_millis;

}


void agg_test_update( agg_test_t *agg_test)
{
  assert( agg_test && agg_test->magic == AGG_TEST_MAGIC);

  if(agg_test->update)
    agg_test->update( agg_test);

}




bool agg_test_repl_statement( agg_test_t *agg_test,  const char *cmd)
{
  assert( agg_test && agg_test->magic == AGG_TEST_MAGIC);

  if(strcmp(cmd, "agg_test2") == 0)
    agg_test->update = agg_test2;

  else if(strcmp(cmd, "agg_test3") == 0)
    agg_test->update = agg_test3;



  else
    return 0;

  return 1;

}



