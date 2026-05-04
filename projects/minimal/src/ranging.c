



#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <strings.h>      // strcasecmp





#include <data/range.h>   // for range_t
#include <data/data.h>         // for auto-ranging
#include <ranging.h>
#include <support.h>      // str_decode_uint
#include <mode.h>





#define RANGING_MAGIC  92379348







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






void ranging_init(
  ranging_t     *ranging,
  _mode_t       *mode,
  const range_t *ranges,      // including cal co-efficients
  const size_t  ranges_sz                  // not sizeof() should be const
) {

  const ranging_t temp = {

    .magic        = RANGING_MAGIC,
    .mode         = mode,
    .ranges       = ranges,
    .ranges_sz    = ranges_sz,     // note const

    .range_idx    = 0,
  };

  // handle field constness
  memcpy( ranging, &temp, sizeof( ranging_t));
}




const range_t * ranging_range_active_get( ranging_t *ranging)
{
  assert( ranging && ranging->magic == RANGING_MAGIC);
  // printf("active range %u\n" ,   ranging->range_idx );

  assert( ranging->range_idx < ranging->ranges_sz);
  const range_t *range  = & ranging->ranges[ ranging->range_idx];
  return range;
}


/*
  rather than return a bool here.
  a better interface would just return the new range_idx,
  or else negative, if no range.

  more succinct, and allows more control to skip inactive/disabled ranges.

*/

static bool ranging_range_dir_valid( ranging_t *ranging, uint32_t range_idx, bool dir)    // 1 up. 0 down
{
  assert( ranging && ranging->magic == RANGING_MAGIC);
  assert( range_idx < ranging->ranges_sz );   // watch out for signess casts.


  const range_t *range = &ranging->ranges[ range_idx];

  return (dir == 1 && !range->top_sentinal)
    || (dir == 0 && !range->bot_sentinal);
}



static void ranging_range_set( ranging_t *ranging, uint32_t range_idx)
{
  assert(ranging && ranging->magic == RANGING_MAGIC);

  printf("\ncurrent range idx %lu\n", range_idx);

  // range_idx is unsigned and expected to be valid
  assert( range_idx < ranging->ranges_sz );
  ranging->range_idx = range_idx;


  // rangingly the range mode state transition
  const range_t *range = &ranging->ranges[ range_idx];
  assert( range);
  assert( range->range_set_mode);

  printf( "switch to %s-%s\n", range->name, range->arg);

  range->range_set_mode( range, ranging->mode, ranging->range_10Meg);



  /*  who clears this flag????
    cleared by app_t after state transistion.  which is messy.
    consider record flag against the mode...
  */

  // why not just always retrigger?
  // ranging->retrigger = true;
}



void ranging_range_set_by_name( ranging_t *ranging, const char *name, const char *arg)
{
  /*
    support external callers
    expects and asserts that the range will exist
  */

  assert( ranging && ranging->magic == RANGING_MAGIC);

  int32_t range_idx = range_get_idx( ranging->ranges, ranging->ranges_sz, name, arg);
  assert( range_idx >= 0 && range_idx < (int) ranging->ranges_sz);

  ranging_range_set( ranging, range_idx);
}




#if 0

// consider something like this...

static bool ranging_maybe_change( ranging_t *ranging, bool dir)
{

    bool ret = ranging_range_dir_valid( ranging, ranging->range_idx, dir);
    if(ret) {

      if( dir)
        ++ranging->range_idx;
      else
        --ranging->range_idx;


      ranging_range_set( ranging, ranging->range_idx);

      return true;
    }


  return false;
}

#endif




bool ranging_repl_range( ranging_t *ranging, const char *cmd)
{
  assert( ranging && ranging->magic == RANGING_MAGIC);



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
    // which may modify the mode
    ranging_range_set( ranging, ranging->range_idx);


    // state will be transferred at repl '\r'

    // set retrigger to clear data buffers
    // ranging->retrigger = true;
  }



  else if(strcmp(cmd, "ar") == 0) {
    ranging->ar = true;
  }
  else if(strcmp(cmd, "noar") == 0) {
    ranging->ar = false;
  }



  // 'u' up in range
  else if(strcmp(cmd, "u") == 0) {

    printf("got u\n");

    bool ret = ranging_range_dir_valid( ranging, ranging->range_idx, 1);
    if(ret) {
      ++ranging->range_idx;
      ranging_range_set( ranging, ranging->range_idx);
    }

    // state will be transferred at repl '\r'
  }
  // 'd' down in range
  else if(strcmp(cmd, "d") == 0) {


    printf("got d\n");

    bool ret = ranging_range_dir_valid( ranging, ranging->range_idx, 0);
    if(ret) {
      --ranging->range_idx;
      ranging_range_set( ranging, ranging->range_idx);
    }
  }


  /////////////////

#if 0
  unsigned n = sscanf(cmd, "%100s %100s", name, arg);

  // handle no argument version of this also
  if( n == 2 || n == 1) {

#endif

  /* regex is very catchall-y
    should return false - if the range is not match
    rather than a relaxed error message
  */

  else if( n = sscanf(cmd, "%100s %100s", name, arg), n == 2 || n == 1) {


    // signed for error handling
    signed long range_idx = range_get_idx( ranging->ranges, ranging->ranges_sz, name, arg);

    printf("here0 range idx %ld\n", range_idx);

    assert( range_idx <  (signed long) ranging->ranges_sz);

    if( range_idx >= 0) {
      // pos
      printf("calling range switch\n");
      ranging_range_set( ranging, range_idx);
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




bool ranging_update_data( ranging_t *ranging, const data_t *data)
{
  assert( ranging && ranging->magic == RANGING_MAGIC);
  assert( data && data->magic == DATA_MAGIC);

  reg_sr_t  status = data->status;

  /*
    this decision-making can be delegated back to the ranging
    functions, for more fine-grain control, if needed.

  */




  // do nothing if not ar.
  if( !ranging->ar)
    return false;

  // do nothing if not a INPUT/HI.
  if( !data->is_hi)
    return false;



  if( !status.cmpr.amp_ovld) {      // ie. active lo. above abs max threshold.


    bool ret = ranging_range_dir_valid( ranging, ranging->range_idx, 1);
    if(ret) {

      printf(", ovld and have valid u range");

      ++ranging->range_idx;
      ranging_range_set( ranging, ranging->range_idx);

      return  true;
    }
    else {

      printf(", ovld and no valid u range - ignore");
    }
  }


  else if( status.cmpr.amp_unld) {      // gone dip below abs min threshold.


    bool ret = ranging_range_dir_valid( ranging, ranging->range_idx, 0);
    if(ret) {

      printf(", unld and have valid d range");
      --ranging->range_idx;
      ranging_range_set( ranging, ranging->range_idx);

      return  true;
    }
    else  {

      printf(", unld and no valid d range - ignore");
    }

  }



  return false;

}








