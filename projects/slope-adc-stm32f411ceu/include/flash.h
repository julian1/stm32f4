
#pragma once

#include <stdio.h> // FILE


// bad conflicts with lib2/include/flash.h

void flash_read(void);
void flash_write(void);

void flash_erase_sector1(void);

FILE * open_flash_file(void);
// long ftell2( FILE *f);

