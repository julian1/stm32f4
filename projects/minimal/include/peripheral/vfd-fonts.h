
#pragma once


/*
  peripheral independent code,
  unrelated to specific vfd instance
  except we do not bother to pass an instance argument at the moment
*/


void vfd_write_bitmap_string2( const char *s, uint8_t xpix, uint8_t ychar );

void vfd_write_string2( const char *s, uint8_t xpix, uint8_t ychar );



void vfd_do_something(void);
