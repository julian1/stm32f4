
// change filename, adc_temp.h  or mcu_adc_temp

#pragma once

void adc_setup(void);
uint16_t adc_read_naiive(uint8_t channel);


double adc_temp_read(void );
double adc_temp_read10(void );


