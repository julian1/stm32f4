
#pragma once


typedef struct spi_t spi_t;
typedef struct spi_ice40_t spi_ice40_t;
typedef struct interrupt_t interrupt_t;
typedef struct gpio_t gpio_t;




typedef struct devices_t
{
  // helper struct to ease when this control needs to be passed to the board state transition function.

  // uint32_t magic;


  spi_ice40_t   *spi_fpga0_pc;    //  fpga pre-configuration

  spi_t         *spi_fpga0;       // fpga post-configuration - register set etc

  interrupt_t   *fpga0_interrupt; // TODO review. not clear if really belongs here.
                                  // depends if we configured more than once

  spi_t         *spi_4094;

  spi_t         *spi_mdac0;     // consider rename sts_mdac?

  spi_t         *spi_mdac1;     // rename iso_sts_mdac?


  /* trigger selection - ext/int belongs here, as a device.
    it should be managed by the mode.

    - trigger should perhaps be put here.
  */


  gpio_t        *gpio_trigger_selection;

} devices_t;



