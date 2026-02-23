
/*
  rename measurement?

  localize data handling, and keep this structure opaque in app_t.

*/

#pragma once



#include <stdbool.h>


// #include <mesch12b/matrix.h>   // MAT

/*
typedef struct _mode_t _mode_t;
// typedef struct data_t data_t;
typedef struct devices_t devices_t;
typedef struct gpio_t gpio_t;
*/

typedef struct data_t data_t;





typedef struct cal_t cal_t;
typedef struct spi_t spi_t;


data_t * data_create( cal_t * cal, spi_t *spi  );



// void data_init ( data_t *);


bool data_repl_statement( data_t *data,  const char *cmd );

bool data_flash_repl_statement( data_t *data, const char *cmd);

void data_cal_show( data_t *data );


