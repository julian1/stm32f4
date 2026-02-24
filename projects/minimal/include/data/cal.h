

#pragma once


#include <stdbool.h>

// consider constrain in data.c
#define CAL_MAGIC 9999123



typedef struct cal_t
{

  uint32_t magic;
  /*
    perhaps remove. since only used to produce the cal.
    so just pass as arg when cal process is called
    - except can be used to encode model representatino if have two models with same number of coefficients
  */

  // change name to just id.
  unsigned id;

#if 0
  unsigned model_spec; // to use.

  // MAT *model_b;           // consider rename cal_b or model_b ?
  void *model_b;           // consider rename cal_b or model_b ?

  double model_sigma_div_aperture;    // stderr() of the regression..  maybe rename model_stderr change name model_sigma_div_aperture?
#endif


  // want better name weight_ref_neg;
  double w;
  double divisor;

} cal_t;



void cal_init( cal_t *cal);

bool cal_flash_repl_statement( cal_t *cal, const char *cmd);



