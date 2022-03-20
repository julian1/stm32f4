
#pragma once

#ifdef __cplusplus
extern "C" {
#endif



typedef struct CBuf CBuf;


// streams for both stdout, stderr
void cbuf_init_std_streams( CBuf *console_out );

// legacy printf
void usart1_printf(const char *format, ...);

#ifdef __cplusplus
}
#endif



