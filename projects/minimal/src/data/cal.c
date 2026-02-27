
/*
  localise this stuff because its a bit messy
  this file should be at toplevel or /app, because it deals with app
*/



#include <stdio.h>    // printf, scanf
#include <assert.h>
#include <string.h>   // memset



#include <libopencm3/stm32/flash.h>

#include <lib2/stream-flash.h>
#include <flash/file-blob.h>

#include <data/cal.h>
#include <data/range.h>





void cal_init(
  cal_t     *cal,

  uint32_t  flash_sect_addr,
  uint8_t   flash_sect_num,

  unsigned  *id,
  double    *w,
  range_t   *ranges,
  size_t    ranges_sz
) {

  assert(cal);
  memset(cal, 0, sizeof(cal_t));

  cal->magic = CAL_MAGIC;

  // feb. 2026. consider pass file. instead.
  cal->flash_sect_addr = flash_sect_addr;
  cal->flash_sect_num = flash_sect_num;

  cal->id  = id;
  cal->w  = w;

  cal->ranges = ranges;
  cal->ranges_sz = ranges_sz;
}





/*

  to store extra info like stddev, date etc.  of cal.
  can just place text in a single string instead.
  it's enough structure


  ---------

  for the cal slot/name .  use a string identifier. not a number.
  good convenience.


     size_t fread(void *restrict ptr, size_t size, size_t nmemb,
                  FILE *restrict stream);
     size_t fwrite(const void *restrict ptr, size_t size, size_t nmemb,
                  FILE *restrict stream);
*/



static void file_write_cal_handler( FILE *f, blob_header_t *header,  cal_t *cal)      // should pass app. to allow stor may be better t
{
  assert(f);
  assert(header);
  assert(cal);
  assert(cal->magic == CAL_MAGIC);


  // set the blob id
  assert(header->len == 0 && header->magic == 0);   // this handler doesn't care or know yet
  header->id = 108;

  fwrite( cal->id,                  sizeof(cal->id), 1, f);
  fwrite( cal->w,                   sizeof(cal->w), 1, f);


  for( unsigned i = 0; i < cal->ranges_sz ; ++i )  {
    // just range idx are ok
    range_t *range = &cal->ranges[ i];
    assert( range->id == i);
    assert( range->name);

    // now write the b and a coeffs.
  }


#if 0
  // write the cal
  fwrite( &cal->model_id,                  sizeof(cal->model_id), 1, f);
  fwrite( &cal->model_spec,                sizeof(cal->model_spec), 1, f);
  fwrite( &cal->model_sigma_div_aperture,  sizeof(cal->model_sigma_div_aperture), 1, f);

  m_foutput_binary( f, cal->model_b);
#endif
}



static void file_scan_cal_handler( FILE *f, blob_header_t *header, cal_t *cal )
{
  assert(header);
  assert(cal);
  assert(cal->magic == CAL_MAGIC);

  // printf("found blob id=%u len=%u\n", header->id, header->len );

  if(header->id == 108) {

    unsigned id;
    fread( &id,  sizeof(id), 1, f);

    printf("found id %u", id);

    if(id == cal->model_id_to_load          // model_id_to_load
      || cal->model_id_to_load == 0 ) {    // to always load, and thus get the most recent.


      *cal->id = id ;
      fread( cal->w,               sizeof(cal->w), 1, f);

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






bool cal_repl_statement( cal_t *cal, const char *cmd)
{

  assert(cal);
  assert(cal->magic == CAL_MAGIC);

  uint32_t u0;

  if(strcmp(cmd, "cal show") == 0) {

    printf("id    %u\n",  *cal->id);
    printf("w     %f\n",  *cal->w);


    return 1;
  }


  // flash erase
  else if(strcmp(cmd, "cal flash erase") == 0) {

    printf("flash erasing sector\n");

    // usart1_flush();
    // fflush(stdout);

    flash_erase_sector_( cal->flash_sect_num /*FLASH_SECT_NUM */ );
    printf("done erase\n");
    return 1;
  }


  else if(sscanf(cmd, "cal flash write %lu", &u0 ) == 1) {

    // perhaps should have a separate repl commend to set the id... in order to save it.
    *cal->id = u0;

    // now save to flash
    printf("flash unlock\n");
    flash_unlock();

    FILE *f = flash_open_file( cal->flash_sect_addr /*FLASH_SECT_ADDR */);
    file_blob_skip_end( f);
    // use callback to write the block.
    file_blob_write( f,  (void (*)(FILE *, blob_header_t *, void *)) file_write_cal_handler, cal);
    fclose(f);

    printf("flash lock\n");
    flash_lock();
    printf("done\n");

    return 1;
  }


  // consider name load?
  else if(sscanf(cmd, "cal flash read %lu", &u0 ) == 1) {

    // set the predicate
    cal->model_id_to_load = u0;

    printf("flash unlock\n");
    flash_unlock();
    FILE *f = flash_open_file( cal->flash_sect_addr /*FLASH_SECT_ADDR */);

    file_blobs_scan( f,  (void (*)( FILE *, blob_header_t *, void *))  file_scan_cal_handler, cal);
    fclose(f);

    printf("flash lock\n");
    flash_lock();
    printf("done\n");
    return 1;
  }

  else
    return 0;

}

#if 0
void cal_init( cal_t *cal, double *b, double *a, size_t sz)
{
  assert(cal);
  memset(cal, 0, sizeof(cal_t));

  cal->magic = CAL_MAGIC;

  cal->sz = sz;
  cal->b = b;
  cal->a = a;

}
#endif

