

#pragma once


#include <stdbool.h>

// consider constrain in data.c
#define CAL_MAGIC 9999123


/*
  structure used for reading and writing to flash.
  It is not exposed/passed to cal routing or data.c etc.



*/


typedef struct range_t range_t;

typedef struct cal_t
{

  uint32_t magic;
  /*
    perhaps remove. since only used to produce the cal.
    so just pass as arg when cal process is called
    - except can be used to encode model representatino if have two models with same number of coefficients
  */

  uint32_t  flash_sect_addr;
  uint8_t   flash_sect_num;

  // referenceable name
  // this state does not need to be shared...
  // unsigned *id;

  // date.

  unsigned  id;
  double    w;          // same as &app->cal_w

  double    b;          // 10V/1x. range scale factor
                        // but this is associated with a range.  with DCV 10.
                        // DCV 1.  will have a different scalar.
                        // whether sampling terminals, lts, daq, or ref.

  double    front_terminal_offset;    // should be represented once.
                                      // and used for all values.
  double    rear_terminal_offset;

  // double    x[ max_ranges ];

/*
  with our proposed refactor.
  these range structures get removed.
  - instead.. the range_t has a function that takes cal_t

*/

/*
  range_t   *ranges;    // same as app->ranges
  size_t    ranges_sz;
*/

  // used for communication during file scan
  uint32_t  model_id_to_load;


  ///////////////////////

} cal_t;



// void cal_init( cal_t *cal, double *b, double *a, size_t sz);
// void cal_init( cal_t *cal, unsigned *id, double *w, range_t *ranges);

void cal_init(
  cal_t     *cal,

  /*
    should pass an already opened file descriptor
  */
  uint32_t  flash_sect_addr,
  uint8_t   flash_sect_num

  // unsigned  *id,
  // double    *w,
  // range_t   *ranges,
  // size_t    ranges_sz
);


bool cal_repl_statement( cal_t *cal, const char *cmd);









#if 0
  unsigned model_spec; // to use.

  // MAT *model_b;           // consider rename cal_b or model_b ?
  void *model_b;           // consider rename cal_b or model_b ?

  double model_sigma_div_aperture;    // stderr() of the regression..  maybe rename model_stderr change name model_sigma_div_aperture?
#endif


