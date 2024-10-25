
#pragma once

#include <stdbool.h>

void ice40_port_extra_setup(void);

void ice40_port_extra_creset_enable(void);  // enable
void ice40_port_extra_creset_disable(void); // hold in reset.

// bool ice40_port_extra_cdone_get(void);

// this is the trigger. must rename
void ice40_port_trig_sa_enable(void);
void ice40_port_trig_sa_disable(void);


