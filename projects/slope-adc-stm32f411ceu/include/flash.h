
#pragma once

#include <stdio.h> // FILE


#if 0
void flash_read(void);
void flash_write(void);
#endif

void flash_erase_sector1(void);

// could take arguments for the sector to open...

FILE * open_flash_file(void);

