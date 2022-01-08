
#ifndef ROTARY_H
#define ROTARY_H

#ifdef __cplusplus
extern "C" {
#endif



void rotary_setup_gpio_portA(void);

int rotary_init_timer( uint32_t tim ); 

 
void rotary_setup_interupt(void);


#endif // ROTARY_H

#ifdef __cplusplus
}
#endif



