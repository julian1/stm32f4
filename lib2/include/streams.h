
#pragma once

#ifdef __cplusplus
extern "C" {
#endif


#include <stdio.h>
#include <stdbool.h>

typedef struct CBuf CBuf;


// streams for both stdout, stderr
void cbuf_init_std_streams( CBuf *console_out );

// legacy printf. should move to local util.c
void usart1_printf(const char *format, ...);


void fflush_on_newline( FILE *f, bool val);


#ifdef __cplusplus
}
#endif



