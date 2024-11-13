
#pragma once


// this file should really be split into separate files. for spi1 and spi2.
// as they are independent devices (eg. port and io pin specific).


void spi1_port_setup(void);

void spi1_port_interupt_setup(void);
void spi1_port_interupt_handler_set( void (*pfunc_)(void *), void *ctx);


//////////////////////


void spi2_port_setup(void);








