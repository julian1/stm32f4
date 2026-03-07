

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
      consider instead passing already open file descriptor on construction
  */

  uint32_t  flash_sect_addr;
  uint8_t   flash_sect_num;


  unsigned  id;
  double    w;          // ref current weighting

  double    b;          // 10V/1x. range scale factor
                        // but this is associated with a range.  with DCV 10.
                        // DCV 1.  will have a different scalar.
                        // whether sampling terminals, lts, daq, or ref.

  // amplifier
  double    b10;         // 10x gain
  double    b100;
  double    b1000;

  // hv div
  double    div100;       // with b10 or not?
  double    div1000;      // with b or not?


  // consider use a double array for everything
  // and indexable enums ?
  // simplifies the save/load. and values


  double    front_terminal_offset;    // should be represented once.
  double    rear_terminal_offset;


  // used for communication during file scan
  uint32_t  model_id_to_load;


} cal_t;



void cal_init(
  cal_t     *cal,
  /*
    should pass an already opened file descriptor
  */
  uint32_t  flash_sect_addr,
  uint8_t   flash_sect_num

);


void cal_show( cal_t *cal);

bool cal_repl_statement( cal_t *cal, const char *cmd);







#if 0

  // unsigned  *id,
  // double    *w,
  // range_t   *ranges,
  // size_t    ranges_sz


  unsigned model_spec; // to use.

  // MAT *model_b;           // consider rename cal_b or model_b ?
  void *model_b;           // consider rename cal_b or model_b ?

  double model_sigma_div_aperture;    // stderr() of the regression..  maybe rename model_stderr change name model_sigma_div_aperture?
#endif


