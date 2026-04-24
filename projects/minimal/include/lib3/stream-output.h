

#pragma once

typedef struct cbuf_t cbuf_t;

FILE *stream_init_output( cbuf_t *circ_buf );

int ffnctl( FILE *f, int cmd );

// flush/sync are equivalent on linus.
#define SYNC_OUTPUT_ON_NEWLINE   0x01



