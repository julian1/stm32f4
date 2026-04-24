

#pragma once

typedef struct cbuf_t cbuf_t;

FILE *file_open_output_cbuf( cbuf_t *circ_buf );

int ffnctl( FILE *f, int cmd );

// flush/sync are equivalent on linus.
#define O_SYNC_NEWLINE   0x01



