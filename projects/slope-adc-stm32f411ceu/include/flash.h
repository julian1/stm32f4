
#pragma once

#include "matrix.h"

// bad conflicts with lib2/include/flash.h

void flash_read(void);
void flash_write(void);

void flash_erase_sector1(void);

void m_write_flash ( MAT *m );
MAT * m_read_flash( MAT *out);


