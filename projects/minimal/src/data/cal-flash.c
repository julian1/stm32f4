
/*
  localise this stuff because its a bit messy
  this file should be at toplevel or /app, because it deals with app
*/



#include <stdio.h>    // printf, scanf
#include <assert.h>



#include <libopencm3/stm32/flash.h>


#include <lib2/stream-flash.h>


#include <flash/file-blob.h>    //
#include <data/matrix.h>        // m_foutput_binary ()


// possibly this file should be at top level. or in /app.
// not sure it's good to be including this like this
#include <app.h>
#include <data/data.h>




// mar 2024. 
// see link/f413rgt6.ld
// #  7: 0x00060000 (0x20000 128kB) not protected
// # 11: 0x000e0000 (0x20000 128kB) not protected
#define FLASH_SECT_ADDR   0x080e0000
#define FLASH_SECT_NUM    11




/*
  EXTR.

    for stuff like stddev, date etc.  of cal.
    just store it all in a single string instead . it's enough structure
    like - can we can record this when we do a cal in a string. or string buffer.
    ----
    rename file_blob_serialize() d

    ---------

    for the cal slot/name .  use a string identifier. not a number.
    good convenience.
*/

static void handler_write_cal( FILE *f, blob_header_t *header, MAT *b )      // should pass app. to allow stor may be better t
{
  assert(f);
  assert(header);
  assert(b);

  // set the blob id
  assert(header->len == 0 && header->magic == 0);   // this handler doesn't care or know yet
  header->id = 106;

  // write the data
  m_foutput_binary( f, b);
}



static void handler_scan_cal( FILE *f, blob_header_t *header, data_t *data )
{
  assert(header);
  assert(data);
  assert(data->magic == DATA_MAGIC);


  printf("whoot got blob id=%u len=%u\n", header->id, header->len );

  if(header->id == 106) {

    // payload should be readable.
    printf("loading cal OK\n"); 

    data->b = m_finput_binary(f, MNULL);

    m_foutput( stdout, data-> b);

  }
}



/*
  try maintaining this using data as context.

*/

bool data_flash_repl_statement( data_t *data, const char *cmd)
{
/*
  assert(app);
  data_t *data = app->data;
*/
  assert(data);
  assert(data->magic == DATA_MAGIC);

  uint32_t u0;


  // flash erase
  if(strcmp(cmd, "flash erase") == 0) {

    printf("flash erasing sector\n");

    // usart1_flush();
    // fflush(stdout);


    flash_erase_sector_( FLASH_SECT_NUM );
    printf("done erase\n");
    return 1;
  }


  else if(sscanf(cmd, "flash cal save %lu", &u0 ) == 1) {

    // u0 arg ignored at moment,
    if(!data->b) {
      printf("no cal!\n");
      return 1;
    }

    // now save to flash
    printf("flash unlock\n");
    flash_unlock();

    FILE *f = flash_open_file( FLASH_SECT_ADDR );
    file_blob_skip_end( f);
    // use callback to write the block.
    file_blob_write( f,  (void (*)(FILE *, blob_header_t *, void *)) handler_write_cal, data->b );
    fclose(f);

    printf("flash lock\n");
    flash_lock();
    printf("done\n");

    return 1;
  }


  else if(sscanf(cmd, "flash cal save %lu", &u0 ) == 1) {

    if(!data->b) {
      printf("no cal!\n");
      return 1;
    }

    // now save to flash
    printf("flash unlock\n");
    flash_unlock();

    FILE *f = flash_open_file( FLASH_SECT_ADDR );
    file_blob_skip_end( f);
    // use callback to write the block.
    file_blob_write( f,  (void (*)(FILE *, blob_header_t *, void *)) handler_write_cal, data->b );
    fclose(f);

    printf("flash lock\n");
    flash_lock();
    printf("done\n");
    return 1;
  }


  // change name load?
  else if(sscanf(cmd, "flash cal read %lu", &u0 ) == 1) {

    printf("flash unlock\n");
    flash_unlock();
    FILE *f = flash_open_file( FLASH_SECT_ADDR );
    file_blobs_scan( f,  (void (*)( FILE *, blob_header_t *, void *))  handler_scan_cal , data ); // note passing data here.
    fclose(f);

    printf("flash lock\n");
    flash_lock();
    printf("done\n");
    return 1;
  }

  else
    return 0;

}


