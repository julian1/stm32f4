
#pragma once

void ice40_port_extra_setup(void);

void ice40_port_extra_creset_enable(void);  // enable
void ice40_port_extra_creset_disable(void); // hold in reset.

bool ice40_port_extra_cdone_get(void);

