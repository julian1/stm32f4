
/*
  measurement/ cal specific matrix funcs

  this code could be moved to cal.c quite reasonably.

*/
#pragma once

#include "calc.h"



MAT * run_to_matrix( // const Run *run,
    uint32_t clk_count_mux_neg,
    uint32_t clk_count_mux_pos,
    uint32_t clk_count_mux_rd,
    unsigned model,
    MAT * out
);



MAT * m_calc_predicted( const MAT *b, const MAT *x, const MAT *aperture);


void m_stats_print( MAT *buffer );


// rename,  print_slope_b_detail() ?

void show_slope_b_detail( unsigned aperture, double slope_b );




