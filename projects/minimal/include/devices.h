
#pragma once


typedef struct spi_t spi_t;
typedef struct spi_ice40_t spi_ice40_t;
typedef struct interrupt_t interrupt_t;





typedef struct devices_t
{
  // helper struct to ease passing these around to the board state transition function.

  // uint32_t magic;


  spi_ice40_t   *spi_fpga0_pc;    //  fpga pre-configuration

  spi_t         *spi_fpga0;       // fpga post-configuration - register set etc

  interrupt_t   *fpga0_interrupt; // TODO review. not clear if really belongs here.

  spi_t         *spi_4094;

  spi_t         *spi_mdac0;     // rename sts_mdac?

  spi_t         *spi_mdac1;     // rename iso_sts_mdac?



} devices_t;



