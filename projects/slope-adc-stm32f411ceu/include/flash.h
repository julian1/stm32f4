
#pragma once

#include <stdio.h> // FILE

#include "matrix.h"

// bad conflicts with lib2/include/flash.h

void flash_read(void);
void flash_write(void);

void flash_erase_sector1(void);

// void m_write_flash ( MAT *m );
void m_write_flash ( MAT *m , FILE *f);

// MAT * m_read_flash( MAT *out);
MAT * m_read_flash( MAT *out, FILE *f);



FILE * open_flash_file(void);
long ftell2( FILE *f);

