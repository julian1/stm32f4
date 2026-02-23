
/*
  low-level data update

*/

#pragma once



#include <stdbool.h>


typedef struct cal_t cal_t;
typedef struct spi_t spi_t;


// consider place in data.c
#define DATA_MAGIC 123



typedef struct data_t
{

  uint32_t magic;


  // feb 2026.
  // move line_freq to app


  cal_t *cal;
  spi_t *spi ;

  // first reading
  bool  first;

  // reading value
  double value;

  bool show_counts;
  bool show_stats;
  bool show_extra;


} data_t;



data_t * data_create( cal_t * cal, spi_t *spi  );



// void data_init ( data_t *);


bool data_repl_statement( data_t *data,  const char *cmd );

bool data_flash_repl_statement( data_t *data, const char *cmd);

void data_cal_show( data_t *data );

void data_update( data_t *data );

