
#pragma once

#ifdef __cplusplus
extern "C" {
#endif


#include <stdio.h>
#include <stdbool.h>

typedef struct CBuf CBuf;


// streams for both stdout, stderr
void cbuf_init_stdout_streams( CBuf *console_out );


void cbuf_init_stdin_streams( CBuf *console_in );

// legacy printf. should move to local util.c
void usart1_printf(const char *format, ...);

// flush/sync are equivalent on linus.
#define FILE_SYNC_ON_NEWLINE   0x01

// void fflush_on_newline( FILE *f, bool val);
int ffnctl( FILE *f, int cmd );


#ifdef __cplusplus
}
#endif



