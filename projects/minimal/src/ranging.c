



#include <stdio.h>        // printf, scanf
#include <assert.h>
#include <string.h>
#include <strings.h>      // strcasecmp


#include <data/range.h>   // for init_values

#include <ranging.h>

#include <support.h>      // str_decode_uint
#include <mode.h>



#define RANGING_MAGIC  92379348




void ranging_init( ranging_t *ranging, _mode_t *mode)
{


  const struct ranging_t temp = {

    .magic        = RANGING_MAGIC,
    .mode         = mode,
    .ranges       = range_init_values,
    .ranges_sz    = range_init_sz,     // note const
    .range_idx    = 0,
  };

  // elide over constness
  memcpy( ranging, &temp, sizeof(ranging_t));

}




const range_t * ranging_active_range( ranging_t *ranging)
{
  // printf("active range %u\n" ,   ranging->range_idx );

  assert( ranging->range_idx < ranging->ranges_sz);

  const range_t *range  = & ranging->ranges[ ranging->range_idx];

  return range;

}




static int32_t range_get_idx( const range_t *ranges, size_t sz, const char *name, const char *arg )
{
  // TODO consider rename  lookup..  range_find_idx.

  for( size_t i = 0; i < sz; ++i) {

    const range_t *range = & ranges[ i];

    if( strcasecmp( name, range->name) == 0
      && strcasecmp( arg, range->arg) == 0
    ) {

      printf("found %s-%s\n", range->name, range->arg );
      return i ;
    }
  }

  return -1;
}










static bool ranging_range_dir_valid( ranging_t *ranging, uint32_t range_idx, bool dir)    // 1 up. 0 down
{
  assert(ranging && ranging->magic == RANGING_MAGIC);
  assert( range_idx < ranging->ranges_sz );   // watch out for signess casts.


  const range_t *range = &ranging->ranges[ range_idx];

  return (dir == 1 && !range->top_sentinal)
    || (dir == 0 && !range->bot_sentinal);
}


/*
  OK. this needs the mode passed to it.

*/

static void ranging_range_switch( ranging_t *ranging, uint32_t range_idx)
{
  assert(ranging && ranging->magic == RANGING_MAGIC);

  printf("here2 range idx %lu\n", range_idx);   // is 50???

  // range_idx is unsigned and expected to be valid
  // set the current range_idx. used for decode_update_data
  assert( range_idx < ranging->ranges_sz );   // watch out for signess casts.
  ranging->range_idx = range_idx;


  // rangingly the range mode state transition
  const range_t *range = &ranging->ranges[ range_idx];
  assert( range);
  assert( range->range_set_mode);

  printf( "switch to %s-%s\n", range->name, range->arg);

  range->range_set_mode( range, ranging->mode, ranging->range_10Meg);



#if 0
  // force retrigger to clear buffers
  // HMMMMM.....
  // this is a problem
  ranging->repl_retrigger = true;
#endif

  // who clears this????
  // has to be done on state transition.  which is messy.
  ranging->retrigger = true;

}



void ranging_range_switch1( ranging_t *ranging, const char *name, const char *arg)
{
  assert(ranging && ranging->magic == RANGING_MAGIC);

  // assert() that the range exists

  int32_t range_idx = range_get_idx( ranging->ranges, ranging->ranges_sz, name, arg);


  assert( range_idx >= 0 && range_idx < (int) ranging->ranges_sz);

  ranging_range_switch( ranging, range_idx);
}










bool ranging_repl_range( ranging_t *ranging, const char *cmd)
{
  assert(ranging && ranging->magic == RANGING_MAGIC);



  char s0[ 100 + 1];
  uint32_t u0;

  char name[ 100 + 1];
  char arg[ 100 + 1];

  // in case sscanf will not do this for the second argument when it is not passsed.
  name[0] = 0;
  arg[ 0] = 0;

  unsigned n;


  if( sscanf(cmd, "10Meg %100s", s0) == 1
    && str_decode_uint( s0, &u0))  {

      /*  the 10Meg. impedance state is a high-level range_t state concept and belongs in ranging_t rather than mode_t
          ie. there is no use/relevance when not using ranges
          we can still use the mode_t and write K403 directly whenever needed.
          -------
          for the same reason - we only need to set it, in tests etc, if we use a dcv ranges
          so perhaps should move it out of mode_t. and explicitly set it, for the few cases that tests use the range function.
          perhaps rename  range_10Meg.
      */
      _mode_t  *mode  = ranging->mode;
      assert(mode && mode->magic == MODE_MAGIC);

      // set flag
      ranging->range_10Meg = u0;


      // re-apply range function
      // this modifies the mode
      ranging_range_switch( ranging, ranging->range_idx);

      // set retrigger to clear data buffers
      ranging->retrigger = true;
  }



  // 'u' up in range
  else if(strcmp(cmd, "u") == 0) {

    printf("got u\n");

    bool ret = ranging_range_dir_valid( ranging, ranging->range_idx, 1);
    if(ret) {
      ++ranging->range_idx;
      ranging_range_switch( ranging, ranging->range_idx);
    }
  }
  // 'd' down in range
  else if(strcmp(cmd, "d") == 0) {


    printf("got d\n");

    bool ret = ranging_range_dir_valid( ranging, ranging->range_idx, 0);
    if(ret) {
      --ranging->range_idx;
      ranging_range_switch( ranging, ranging->range_idx);
    }
  }


  /////////////////

#if 0
  unsigned n = sscanf(cmd, "%100s %100s", name, arg);

  // handle no argument version of this also
  if( n == 2 || n == 1) {

#endif

  /* regex is very catch-ally
    so should return false. if the range is not match
    instead of an error message.
  */

  else if( n = sscanf(cmd, "%100s %100s", name, arg), n == 2 || n == 1) {


    // signed for error handling
    signed long range_idx = range_get_idx( ranging->ranges, ranging->ranges_sz, name, arg);

    printf("here0 range idx %ld\n", range_idx);

    assert( range_idx <  (signed long) ranging->ranges_sz);

    if( range_idx >= 0) {
      // pos
      printf("calling range switch\n");
      ranging_range_switch( ranging, range_idx);
    }
    else {
      // neg

      printf("range not found\n");
    }

  }


  else
    return false;



  return true;

}






#if 0

  else if( sscanf(cmd, "10Meg %100s", s0) == 1
    && str_decode_uint( s0, &u0))  {

      /*  the 10Meg. impedance state is a high-level range_t state concept and belongs in ranging_t rather than mode_t
          ie. there is no use/relevance when not using ranges
          we can still use the mode_t and write K403 directly whenever needed.
          -------
          for the same reason - we only need to set it, in tests etc, if we use a dcv ranges
          so perhaps should move it out of mode_t. and explicitly set it, for the few cases that tests use the range function.
          perhaps rename  range_10Meg.
      */
      _mode_t  *mode  = ranging->mode;
      assert(mode && mode->magic == MODE_MAGIC);

      // set flag
      ranging->range_10Meg = u0;


      // re-rangingly the current range function
      // this modifies the mode
      ranging_range_switch( ranging, *ranging->range_idx);

      // set retrigger to clear data buffers
      ranging->repl_retrigger = true;
  }


#endif




#if 0

  /*
      TODO consider  move this code to ranging_repl_range
  */
  // 'u' up in range
  else if(strcmp(cmd, "u") == 0) {

    bool ret = ranging_range_dir_valid( ranging, *ranging->range_idx, 1);
    if(ret) {
      ++ranging->range_idx;
      ranging_range_switch( ranging, *ranging->range_idx);
    }
  }
  // 'd' down in range
  else if(strcmp(cmd, "d") == 0) {

    bool ret = ranging_range_dir_valid( ranging, *ranging->range_idx, 0);
    if(ret) {
      --ranging->range_idx;
      ranging_range_switch( ranging, *ranging->range_idx);
    }
  }

#endif





