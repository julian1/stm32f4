
#pragma once

#include <stdio.h> // FILE
#include "matrix.h"



void c_skip_to_end(  FILE *f);

// void c_skip_to_last_valid(  FILE *f);
int c_skip_to_last_valid(  FILE *f);


int c_scan(  FILE *f);



// void m_write_flash ( MAT *m );
// void m_write_flash ( MAT *m , FILE *f);
void m_write_flash ( MAT *m , int slot, FILE *f);

// MAT * m_read_flash( MAT *out);
MAT * m_read_flash( MAT *out, FILE *f);



