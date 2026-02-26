
/*
  localise this stuff because its a bit messy
  this file should be at toplevel or /app, because it deals with app
*/



#include <stdio.h>    // printf, scanf
#include <assert.h>
#include <string.h>



#include <libopencm3/stm32/flash.h>

#include <lib2/stream-flash.h>
#include <flash/file-blob.h>

#include <data/cal.h>



// mar 2024.
// see link/f413rgt6.ld
// #  7: 0x00060000 (0x20000 128kB) not protected
// # 11: 0x000e0000 (0x20000 128kB) not protected
#define FLASH_SECT_ADDR   0x080e0000
#define FLASH_SECT_NUM    11




void cal_init( cal_t *cal, double *b, double *a, size_t sz)
{
  assert(cal);
  memset(cal, 0, sizeof(cal_t));

  cal->magic = CAL_MAGIC;

  cal->sz = sz;
  cal->b = b;
  cal->a = a;

}



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

static void handler_write_cal( FILE *f, blob_header_t *header,  cal_t *cal)      // should pass app. to allow stor may be better t
{
  assert(f);
  assert(header);
  assert(cal);
  assert(cal->magic == CAL_MAGIC);


  // set the blob id
  assert(header->len == 0 && header->magic == 0);   // this handler doesn't care or know yet
  header->id = 108;

#if 0
  // write the cal
  fwrite( &cal->model_id,                  sizeof(cal->model_id), 1, f);
  fwrite( &cal->model_spec,                sizeof(cal->model_spec), 1, f);
  fwrite( &cal->model_sigma_div_aperture,  sizeof(cal->model_sigma_div_aperture), 1, f);

  m_foutput_binary( f, cal->model_b);
#endif
}



/*
  feb 2026
  OK.... this function is a pain...

  because we do not have the context - for the the extra missing predicate argument - model_id_to_load

  we will have to dummy up a more complicated structure
*/

typedef struct predicate_t
{

  cal_t     *cal;
  uint32_t  model_id_to_load;

} predicate_t;



static void handler_scan_cal( FILE *f, blob_header_t *header, cal_t *cal /*, uint32_t  model_id_to_load */)
{

  assert(0);

  assert(header);
  assert(cal);
  assert(cal->magic == CAL_MAGIC);

  // printf("found blob id=%u len=%u\n", header->id, header->len );

  if(header->id == 108) {

    unsigned id;
    fread( &id,  sizeof(id), 1, f);

    printf("found id %u", id);

    // TODO FIXME. feb 2026.
    if(id == 999999999 /*model_id_to_load */) {
      // || cal->model_id_to_load == -1    // to always load, and thus get the most recent.

#if 0
      cal->model_id = model_id;

      // read the rest of the cal
      fread( &cal->model_spec,               sizeof(cal->model_spec), 1, f);
      fread( &cal->model_sigma_div_aperture, sizeof(cal->model_sigma_div_aperture), 1, f);

      cal->model_b = m_finput_binary(f, MNULL);
#endif

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

bool cal_flash_repl_statement( cal_t *cal, const char *cmd)
{
/*
  assert(app);
  cal_t *cal = app->cal;
*/
  assert(cal);
  assert(cal->magic == CAL_MAGIC);

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

    // feb 2026
    assert( 0);
#if 0
    if(!cal->model_b) {
      printf("no cal!\n");
      return 1;
    }

    cal->id = u0;
#endif

    // now save to flash
    printf("flash unlock\n");
    flash_unlock();

    FILE *f = flash_open_file( FLASH_SECT_ADDR );
    file_blob_skip_end( f);
    // use callback to write the block.
    file_blob_write( f,  (void (*)(FILE *, blob_header_t *, void *)) handler_write_cal, cal /*cal->model_b */);
    fclose(f);

    printf("flash lock\n");
    flash_lock();
    printf("done\n");

    return 1;
  }


  // change name load?
  else if(sscanf(cmd, "flash cal read %lu", &u0 ) == 1) {

    // OK. we don't want to override the id, if we don't find a valid cal.
    // so use a separate variable.

    // feb 2026
    assert(0);
#if 0
    cal->model_id_to_load = u0;
#endif

    printf("flash unlock\n");
    flash_unlock();
    FILE *f = flash_open_file( FLASH_SECT_ADDR );
    file_blobs_scan( f,  (void (*)( FILE *, blob_header_t *, void *))  handler_scan_cal , cal ); // note passing cal here.
    fclose(f);

    printf("flash lock\n");
    flash_lock();
    printf("done\n");
    return 1;
  }

  else
    return 0;

}


