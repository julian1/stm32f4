
#pragma once

#include <stdint.h>

// better name clear_reset?
// void buffer_set_size( MAT *buffer, uint32_t sz);

MAT * buffer_reset( MAT *buffer, uint32_t sz);

void buffer_push( MAT *buffer, uint32_t *idx, double val );

void buffer_stats_print( MAT *buffer /* flags */ );

