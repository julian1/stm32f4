
#pragma once


/*  this is peripheral code, unrelated to underlying vfd instance
    except we are not bothering to pass an instance at the moment
*/


void vfd_write_bitmap_string2( const char *s, uint8_t xpix, uint8_t ychar );

void vfd_write_string2( const char *s, uint8_t xpix, uint8_t ychar );



void vfd_do_something(void);
