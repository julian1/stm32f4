

#pragma once

#include <stdint.h>
// #include <stdbool.h>

#include <device/spi-fpga0-reg.h>     // for reg_sr_t

typedef struct spi_t spi_t ; 


typedef struct data_t     // only for tests. will conflict with data_t in data/data.h
{

  // uint32_t  magic;

  reg_sr_t status;

  // included for cal w
  uint32_t clk_count_refmux_pos;
  uint32_t clk_count_refmux_neg;
  uint32_t clk_count_sigmux;

  uint32_t clk_count_aperture;  // needed when sigmux not active

} data_t;




void spi_read_registers( spi_t *spi, data_t *data);
void print_data( data_t * data);

