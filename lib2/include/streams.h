
#pragma once

#ifdef __cplusplus
extern "C" {
#endif



typedef struct CBuf CBuf;

void init_std_streams( CBuf *console_out );

// legacy printf
void usart_printf(const char *format, ...);

#ifdef __cplusplus
}
#endif



