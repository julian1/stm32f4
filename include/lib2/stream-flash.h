
#pragma once

#include <stdio.h> // FILE



void flash_erase_sector_(void);
// could take arguments for the sector to open...

FILE * flash_open_file(void);

void flash_test_write(void);
void flash_test_read(void);




