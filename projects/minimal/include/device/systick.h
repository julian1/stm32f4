
#pragma once

void systick_setup(uint32_t tick_divider);
void systick_handler_set( void (*pf)(void *),  void *ctx);


