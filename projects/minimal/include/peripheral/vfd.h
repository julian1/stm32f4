
#pragma once

void vfd_init_gpio( void );

void vfd_write_cmd( uint8_t v);

void vfd_write_data( uint8_t v);

uint8_t vfd_read_data( void);


