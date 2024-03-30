
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


       size_t fread(void *restrict ptr, size_t size, size_t nmemb,
                    FILE *restrict stream);
       size_t fwrite(const void *restrict ptr, size_t size, size_t nmemb,
                    FILE *restrict stream);

*/

static void handler_write_cal( FILE *f, blob_header_t *header,  data_t *data )      // should pass app. to allow stor may be better t
{
  assert(f);
  assert(header);
  assert(data);
  assert(data->magic == DATA_MAGIC);


  // set the blob id
  assert(header->len == 0 && header->magic == 0);   // this handler doesn't care or know yet
  header->id = 108;

  // write the data
  fwrite( &data->model_id,                  sizeof(data->model_id), 1, f);
  fwrite( &data->model_cols,                sizeof(data->model_cols), 1, f);
  fwrite( &data->model_sigma_div_aperture,  sizeof(data->model_sigma_div_aperture), 1, f);

  m_foutput_binary( f, data->model_b);

}



static void handler_scan_cal( FILE *f, blob_header_t *header, data_t *data )
{
  assert(header);
  assert(data);
  assert(data->magic == DATA_MAGIC);

  // printf("whoot got blob id=%u len=%u\n", header->id, header->len );

  if(header->id == 108) {

    unsigned model_id;
    fread( &model_id,  sizeof(model_id), 1, f);

    printf("found model_id %u", model_id);

    if(model_id == data->model_id_to_load) {
      // || data->model_id_to_load == -1    // to always load, and thus get the most recent.

      data->model_id = model_id;

      // read the rest of the data
      fread( &data->model_cols,               sizeof(data->model_cols), 1, f);
      fread( &data->model_sigma_div_aperture, sizeof(data->model_sigma_div_aperture), 1, f);

      data->model_b = m_finput_binary(f, MNULL);

      // payload should be readable.
      printf(", loaded cal OK\n");
    } else {

      printf(", ignore\n");
    }
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


  else if(sscanf(cmd, "flash cal write %lu", &u0 ) == 1) {

    if(!data->model_b) {
      printf("no cal!\n");
      return 1;
    }

    data->model_id = u0;

    // now save to flash
    printf("flash unlock\n");
    flash_unlock();

    FILE *f = flash_open_file( FLASH_SECT_ADDR );
    file_blob_skip_end( f);
    // use callback to write the block.
    file_blob_write( f,  (void (*)(FILE *, blob_header_t *, void *)) handler_write_cal, data /*data->model_b */);
    fclose(f);

    printf("flash lock\n");
    flash_lock();
    printf("done\n");

    return 1;
  }


  // change name load?
  else if(sscanf(cmd, "flash cal read %lu", &u0 ) == 1) {

    // OK. we don't want to override the model_id, if we don't find a valid cal.
    // so use a separate variable.

    data->model_id_to_load = u0;

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


