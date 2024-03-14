
#pragma once

#include <stdbool.h>

void ice40_port_extra_setup(void);

void ice40_port_extra_creset_enable(void);  // enable
void ice40_port_extra_creset_disable(void); // hold in reset.

bool ice40_port_extra_cdone_get(void);

void ice40_port_trigger_source_internal_enable(void);
void ice40_port_trigger_source_internal_disable(void);

