
#pragma once

typedef struct CBuf CBuf;

void init_std_streams( CBuf *console_out );

// legacy
void usart_printf(const char *format, ...);



