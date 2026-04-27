

#pragma once

#include <stdint.h>
// #include <stdbool.h>

#include <device/spi-fpga0-reg.h>     // for reg_sr_t

typedef struct spi_t spi_t ;




/*
  lightweight data_t
  and read functions only for tests.
  will conflict or confusion with data_t in data/data.h

*/


// potential for confusion with /data/data.h  data_t.

typedef struct data_t
{

  // uint32_t  magic;

  reg_sr_t status;

  // included for cal w
  uint32_t clk_count_refmux_pos;
  uint32_t clk_count_refmux_neg;
  uint32_t clk_count_sigmux;

  // required when sigmux not active
  uint32_t clk_count_aperture;

} data_t;




void spi_read_registers( spi_t *spi, data_t *data);
void print_data( data_t * data);

