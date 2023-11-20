
#pragma once

#include <stdio.h> // FILE


#if 0
void flash_read(void);
void flash_write(void);
#endif


void flash_erase_sector_(void);
// could take arguments for the sector to open...

FILE * flash_open_file(void);

