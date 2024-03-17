
#pragma once

#include <stdio.h> // FILE



void flash_erase_sector_( uint8_t flash_sect_num);

FILE * flash_open_file( uint32_t flash_sect_addr );

void flash_test_write(void);
void flash_test_read(void);




