
/*
  COW handling of binary blob data.
  better name than 'blob' ?
  opaque data.
  caller decides on structure of data. except for id.

*/

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


void file_blob_skip_end(  FILE *f);


void file_blob_write( FILE *f,    void (*pf)( FILE *, void *ctx ), void *ctx );

int file_blobs_scan( FILE *f,  void (*pf)( FILE *f, Header *, void *ctx ), void *ctx );


