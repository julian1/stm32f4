
#pragma once

#include <stdio.h> // FILE
#include "matrix.h"

// void m_write_flash ( MAT *m );
void m_write_flash ( MAT *m , FILE *f);

// MAT * m_read_flash( MAT *out);
MAT * m_read_flash( MAT *out, FILE *f);


