

#pragma once

#include <stdio.h>  // FILE




// change name blob_header_t etc.
struct Header
{
  unsigned magic;
  unsigned len;
  unsigned id;
};

typedef struct Header Header;


void file_skip_blobs_end(  FILE *f);


void file_write_blob( FILE *f,    void (*pf)( FILE *, void *ctx ), void *ctx );

int file_scan_blobs( FILE *f,  void (*pf)( FILE *f, Header *, void *ctx ), void *ctx );


