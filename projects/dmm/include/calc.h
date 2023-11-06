

#pragma once

// #include "matrix.h"
#include "regression.h"



MAT * run_to_matrix( // const Run *run,
    uint32_t clk_count_mux_neg,
    uint32_t clk_count_mux_pos,
    uint32_t clk_count_mux_rd,
    unsigned model,
    MAT * out
);



void mat_set_row (  MAT *xs, unsigned row_idx,   MAT *whoot );

void vec_set_val (  MAT *xs, unsigned row_idx,   double x);


MAT * m_calc_predicted( const MAT *b, const MAT *x, const MAT *aperture);


// should not expose app here.
// void calc_cal( app_t *app,  MAT *y, MAT *xs, MAT *aperture  );


/*
  TODO
    nov, 2023.

  change name and remove the index argument,
  m_push_val1(  )


  oversize. the matrix.
  also add m_push_val2() and m_push_val3()  - etc
  or m_push_vals_n(  double [] vals )
  and avoid the double handling of creating a row matrix
  before appending...

  - also if have this flexible buffer.  then can do the calibration collection gathering, much more simply.

  - also put the cal structure on app. and then just free and reallocate in place. to avoid leaking.
*/
bool push_buffer1( MAT *buffer, unsigned *i, double value);


void m_stats_print( MAT *buffer );

void show_slope_b_detail( unsigned aperture, double slope_b );




