

#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <strings.h>      // strcasecmp
#include <math.h>         // fabs



#include <data/range.h>   // for range_t
#include <data/data.h>         // for auto-ranging
#include <ranging.h>
#include <support.h>      // str_decode_uint
#include <mode.h>


#define UNUSED(x) ((void)(x))


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




const range_t * ranging_range_active_get( const ranging_t *ranging)
{
  // simple accessor, with basic checks.

  assert( ranging && ranging->magic == RANGING_MAGIC);
  assert( ranging->range_idx < ranging->ranges_sz);

  return & ranging->ranges[ ranging->range_idx];
}






static void ranging_range_set_by_idx( ranging_t *ranging, uint32_t range_idx)
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


}



void ranging_range_set_by_name( ranging_t *ranging, const char *name, const char *arg)
{
  /*
    convenience for external callers
    expects and asserts the range exists
  */

  assert( ranging && ranging->magic == RANGING_MAGIC);

  int32_t range_idx = range_get_idx( ranging->ranges, ranging->ranges_sz, name, arg);
  assert( range_idx >= 0 && range_idx < (int) ranging->ranges_sz);

  ranging_range_set_by_idx( ranging, range_idx);
}




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

    /*  see comments in ranging.h
        the 10Meg. impedance state is a high-level range_t state concept and belongs in ranging_t rather than mode_t
        it is not relevant when setting the mode directly. and when not using ranges
        we can still use the mode_t and write K403 directly whenever needed.
    */
    _mode_t  *mode  = ranging->mode;
    assert(mode && mode->magic == MODE_MAGIC);

    // set flag
    ranging->range_10Meg = u0;

    // re-apply range function
    // which may modify the mode
    ranging_range_set_by_idx( ranging, ranging->range_idx);

    // board state will be update/transferred on repl '\r'
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
    // not nominal

    const range_t *range =  ranging_range_active_get( ranging);
    if( !range->top_sentinal) {

      ++ranging->range_idx;
      ranging_range_set_by_idx( ranging, ranging->range_idx);
    }


    // state will be transferred at repl '\r'
  }
  // 'd' down in range
  else if(strcmp(cmd, "d") == 0) {


    printf("got d\n");

    const range_t *range =  ranging_range_active_get( ranging);
    if( !range->bot_sentinal) {

      --ranging->range_idx;
      ranging_range_set_by_idx( ranging, ranging->range_idx);

    }
  }



  else if( n = sscanf(cmd, "%100[a-zA-Z-] %100s", name, arg), n == 2 || n == 1) {

    printf("range lookup %s %s\n", name, arg);

    // signed for error handling
    signed long range_idx = range_get_idx( ranging->ranges, ranging->ranges_sz, name, arg);

    printf("range idx %ld\n", range_idx);

    assert( range_idx <  (signed long) ranging->ranges_sz);

    if( range_idx >= 0) {
      // pos
      printf("calling range switch\n");
      ranging_range_set_by_idx( ranging, range_idx);
    }
    else {
      // neg

      printf("range not found\n");

      // required/needed
      return false;
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


  /*
    note, that this decision-making can be delegated back to the ranging
    functions, for more fine-grain control, if needed.

  */


  // not ar, then dont care.
  if( !ranging->ar)
    return false;


  // must have a valid reading
  if( !data->reading_valid)
    return false;

  /*
    we could actually stamp

  */


  // the current range...
  const range_t *range = &ranging->ranges[ ranging->range_idx];

  // current range is also stamped on the data.
  assert( range == data->range);


  // get the lower range if available
  const range_t *lower_range =
      range
      && !range->bot_sentinal
      ? &ranging->ranges[ ranging->range_idx - 1]
      : NULL;



  double hysteresis = 0.95;

  // autorange up
  if( range
    && !range->top_sentinal
    && range->fs != 0
    && fabs( data->reading) > range->fs
    ) {      // second bit indicates above threshold


    // not nominal
    printf("\nswitch up\nreading %f\n", data->reading);

    ++ranging->range_idx;
    ranging_range_set_by_idx( ranging, ranging->range_idx);

    /*
      note - can set the second aperture.
      for fast ranging measurement.
      Or just hardcode.
    */
    return true;
  }

  // autorange down
  else if(
    lower_range
    && lower_range->fs != 0
    && fabs( data->reading) <= (lower_range->fs * hysteresis)) {


    --ranging->range_idx;
    ranging_range_set_by_idx( ranging, ranging->range_idx);

    return true;

  }

  return false;
}







/*
  rather than return a bool here.
  a better interface would just return the new range_idx,
  or else negative, if no range.

  more succinct, and allows more control to skip inactive/disabled ranges.

*/
#if 0
static bool ranging_range_dir_valid( ranging_t *ranging, uint32_t range_idx, bool dir)    // 1 up. 0 down
{
/*
    try to deprecate. -   too much boilerplate.

    just test range->top_sentinel.
    etc.
*/

  assert( ranging && ranging->magic == RANGING_MAGIC);
  assert( range_idx < ranging->ranges_sz );   // watch out for signess casts.


  const range_t *range = &ranging->ranges[ range_idx];

  return (dir == 1 && !range->top_sentinal)
    || (dir == 0 && !range->bot_sentinal);
}
#endif


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

#if 0
    bool ret = ranging_range_dir_valid( ranging, ranging->range_idx, 0);
    if(ret) {
      --ranging->range_idx;
      ranging_range_set( ranging, ranging->range_idx);
    }
#endif



  /////////////////

#if 0
  unsigned n = sscanf(cmd, "%100s %100s", name, arg);

  // handle no argument version of this also
  if( n == 2 || n == 1) {

#endif

  /* this regexy is very catchall-y
    should return false, if the range does not match
    rather than a relaxed error message
    ----------


  */

  // else if( n = sscanf(cmd, "%100s %100s", name, arg), n == 2 || n == 1) {








#if 0

bool ranging_update_data( ranging_t *ranging, const data_t *data)
{
  assert( ranging && ranging->magic == RANGING_MAGIC);
  assert( data && data->magic == DATA_MAGIC);

  reg_sr_t  status = data->status;

  /*
    this decision-making can be delegated back to the ranging
    functions, for more fine-grain control, if needed.

  */


  // not ar, then dont care.
  if( !ranging->ar)
    return false;

  /* not a INPUT/HI, dont care. comparators will not be valid
    although we can use the reading
  */
  if( !data->is_hi)
    return false;



  if( status.cmpr.amp_ovld_gt ) {      // second bit indicates above threshold


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


  else if( status.cmpr.amp_unld_lt    // first bit indicates below  threshold
      && ! status.cmpr.amp_unld_gt
  ) {


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

#endif






#if  0

    bool ret = ranging_range_dir_valid( ranging, ranging->range_idx, 1);
    if(ret) {
      ++ranging->range_idx;
      ranging_range_set( ranging, ranging->range_idx);
    }
#endif

