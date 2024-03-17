
/*
  COW handling of binary blob data.
  better name than 'blob' ?
  opaque data.
  caller decides on how much to structure the data and the id field.
  -------

  - id / blob typec
  - adc cal.
  - system cal.
  - whenever mcu is inited/starts. with a timestamp etc.
  - another to log assert error. etc.
*/

#pragma once

#include <stdio.h>  // FILE




// change name blob_header_t etc.
struct blob_header_t
{
  unsigned magic;
  unsigned len;
  unsigned id;
};

typedef struct blob_header_t blob_header_t;


void file_blob_skip_end( FILE *f);

/* need to give callee control over the 'id' field. pass either unsigned or the header structure.
  to let the callee write.
  header structure makes the callbacks symmetric. even if only one field.
  also if add other fields, then they are available.
*/

void file_blob_write( FILE *f, void (*pf)( FILE *, blob_header_t *, void *ctx ), void *ctx );

int file_blobs_scan( FILE *f,  void (*pf)( FILE *f, blob_header_t *, void *ctx ), void *ctx );


