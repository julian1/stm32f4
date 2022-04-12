

/*
  voltage source, 
    - hardware starts with rails powered on. due to glitchy 4094 data pin . or adum isolator.
    - and sometimes the ltz1000 won't start properly. (drag probe over output).
*/




#include <stdio.h>
#include <assert.h>

#include <libopencm3/stm32/spi.h> 


#include "util.h" // msleep

#include "voltage-source-2/voltage-source.h"
#include "voltage-source-2/dac8734.h"
#include "voltage-source-2/4094.h"
#include "voltage-source-2/spi.h"




void voltage_source_2_powerdown(uint32_t spi, uint8_t *spi_4094_reg)
{
  // better to avoid hanging app dependency here.

  /*
  if( !( *spi_4094_reg & REG_RAILS_ON)) {
    printf("voltage_source_2 not powered on\n");
    return;
  }
  */

  assert(spi == SPI2);

  printf("turn off rails\n");
  spi_port_cs2_setup( spi );
  spi_4094_setup(spi);

  /*
    When UNI/BIP-A is tied to IOVDD, group A
    is in unipolar output mode; when tied to DGND, group A is in bipolar output mode. The input data
    written to the DAC are straight binary for unipolar output mode and twos complement for bipolar
    output mode
  */

  // should persist the register in app structure
  // and I think we should.
  *spi_4094_reg &= ~REG_RAILS_OE; // output enabled (active lo)
  *spi_4094_reg &= ~REG_RAILS_ON; // but rails power off

  spi_4094_reg_write(spi, *spi_4094_reg);

  printf("sleep 10ms\n");
  msleep(100);
}


void voltage_source_2_setup(uint32_t spi, uint8_t *spi_4094_reg)
{

  // better to avoid hanging app dependency here.


  if( *spi_4094_reg & REG_RAILS_ON) {
    printf("voltage_source_2 is already powered on\n");
    return;
  }

  assert(spi == SPI2);

  spi_cs2_clear( spi );


  // first ensure rails are off
  printf("\n--------\n");
  printf("make sure rails are off\n");
  spi_port_cs2_setup( spi);
  spi_4094_setup(spi);

  *spi_4094_reg &= ~REG_RAILS_ON;

  spi_4094_reg_write(spi, *spi_4094_reg);
  msleep(1);


  /*
    Note. that we are passing the register by reference so that dac init can manipulate.
    functionality here controls rails. but dac setup sets other register configuration
  */
  int ret = dac_init(spi, spi_4094_reg); // bad name?
  if(ret != 0) {
    assert(0);
  }


  // after dac digital init, turn on rails.
  printf("\n--------\n");
  printf("turn on rails\n");

  spi_port_cs2_setup( spi );
  spi_4094_setup(spi);

  *spi_4094_reg |= REG_RAILS_ON;
  spi_4094_reg_write(spi, *spi_4094_reg);


  printf("sleep 100ms\n");
  msleep(100);

  // write an output
  printf("\n--------\n");
  printf("writing register for dac0 1V output. \n");

  spi_port_cs1_setup(spi); // with CS.
  spi_dac_setup( spi);

  msleep(100);


  spi_dac_write_register( spi, DAC_DAC0_REGISTER, voltage_to_dac( 1.0 ));

}


