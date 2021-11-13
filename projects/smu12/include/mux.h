

#include <stdint.h> // uint32_t




/////////////////
/*
  inputs.
  this will be another register.
  can read on fpga. or have fpga route back on adum gpio pin used as an interupt
  ideally both would be good.
  #define ADC_DONE    GPIO9
  #define ADC_DRDY    GPIO10
*/


// configure the muxing of the spi port

extern void mux_fpga(uint32_t spi);
extern void mux_adc03(uint32_t spi);
extern void mux_w25(uint32_t spi);
extern void mux_dac(uint32_t spi);
extern void mux_adc(uint32_t spi);


////////////////
// abstract to avoid including ice40.h.
// eg. consumers shouldn't care if they're dealing with fpga or ice40

extern void mux_io(uint32_t spi);
extern void io_set( uint32_t spi, uint8_t r, uint8_t v);
extern void io_clear( uint32_t spi, uint8_t r, uint8_t v);
extern void io_write( uint32_t spi, uint8_t r, uint8_t v);
extern void io_toggle( uint32_t spi, uint8_t r, uint8_t v);

extern void io_write_mask( uint32_t spi, uint8_t r, uint8_t mask, uint8_t v);


/*
 supports or'ing mask selected values we want to write. eg.

 io_write_mask( spi, reg, GAIN_VFB_OP1 | GAIN_VFB_OP2, 0);
 io_write_mask( spi, reg, GAIN_VFB_OP1 | GAIN_VFB_OP2, GAIN_VFB_OP2);
*/

// use bitmask write, to write these in individual combination
// OK. no. it's messy. we really want to separate these,
// into different regs to make easier to write...
// #define REG_GAIN_IFB    21
// #define GAIN_IFB_OP1  (1<<0)
// #define GAIN_IFB_OP2  (1<<1)


/*
// better name?
#define REG_IRANGE_X_SW58    22
#define IRANGE_X_SW5         (1<<0)
#define IRANGE_X_SW6         (1<<1)
#define IRANGE_X_SW7         (1<<2)
#define IRANGE_X_SW8         (1<<3)
*/


// same dg444 as REG_GAIN_IFB
// but separating makes it easier
// #define REG_GAIN_VFB    23
// #define GAIN_VFB_OP1  (1<<0)
// #define GAIN_VFB_OP2  (1<<1)
// #define REG_INA_DIFF_SW   26
// #define INA_DIFF_SW1_CTL    (1<<0)
// #define INA_DIFF_SW2_CTL    (1<<1)


