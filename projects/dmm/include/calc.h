

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


bool push_buffer1( MAT *buffer, unsigned *i, double value);


void m_stats_print( MAT *buffer );
