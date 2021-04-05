
#include <stdint.h> // uint32_t

//////////////////////////////////////////////

// REGISTER_DAC?

// change name REG_DAC not DAC_REGISTER etc

// what the hell is happening...

// RENAME OR = output register and SRR set reset register

// OK. Now is there a way to do both...

#define LED_REGISTER  7
#define LED1 (1<<0)    // D38
#define LED2 (1<<1)    // D37
#define LED3 (1<<2)    // D37
#define LED4 (1<<3)    // D37


#define SPI_MUX_REGISTER  8
#define SPI_MUX_ADC03     (1<<0)
#define SPI_MUX_DAC       (1<<1)
#define SPI_MUX_FLASH     (1<<2)
#define SPI_MUX_ADC       (1<<3)

/*
  shouldn't these name the component.  DAC8734, MCP3208, W25 etc.  maybe not
  because they will be repeated if have more than one.
  No. naming follows schematic labels. and verilog.  mostly.
*/

#define DAC_REGISTER  9
#define DAC_LDAC      (1<<0)
#define DAC_UNI_BIP_A (1<<1)
#define DAC_UNI_BIP_B (1<<2)
#define DAC_RST       (1<<3)

// TODO change name, RAILS_REG or REG_RAILS consistent with verilog?
#define RAILS_REGISTER  10
#define RAILS_LP15V   (1<<0)
#define RAILS_LP30V   (1<<1)
#define RAILS_LP60V   (1<<2)
#define RAILS_OE      (1<<3)

// 11 is soft reset

#define DAC_REF_MUX_REGISTER  12
#define DAC_REF_MUX_A     (1<<0)
#define DAC_REF_MUX_B     (1<<1)


#define ADC_REGISTER  14
#define ADC_M0      (1<<0)
#define ADC_M1      (1<<1)
#define ADC_M2      (1<<2)
#define ADC_RST     (1<<3)

// inputs
// #define ADC_DONE    GPIO9
// #define ADC_DRDY    GPIO10



// naming. there may be more than one.
// type not so important.

extern void mux_fpga(uint32_t spi);   // rename mux_io.
extern void mux_adc03(uint32_t spi);
extern void mux_w25(uint32_t spi);
extern void mux_dac(uint32_t spi);
extern void mux_adc(uint32_t spi);


////////////////
// abstract and avoid including ice40.h.
// functions build on top of spi_ice40_reg_*

extern void mux_io(uint32_t spi);
extern void io_set( uint32_t spi, uint8_t r, uint8_t v);
extern void io_clear( uint32_t spi, uint8_t r, uint8_t v);


