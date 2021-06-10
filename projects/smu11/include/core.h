/*
  core - is comprised of spi mux, and io.

  all these registers - are the ice40/fpga verilog
  and design function.
    it's design.h. or control.h or core.h.
*/
#include <stdint.h> // uint32_t


// REGISTER_DAC?
// change name REG_REG_DAC not DAC etc
// maybe RENAME OR = output register and SRR set reset register

/*
  include specific part,  DAC8734, MCP3208, W25 etc.  maybe not
  because they will be repeated if have more than one.
  No. naming follows schematic labels. and verilog.  mostly.
  -------

  using 'reg' we don't have to pass these around
*/


#define REG_LED  7
#define LED1 (1<<0)    // D38
#define LED2 (1<<1)    // D37
#define LED3 (1<<2)    // D37
#define LED4 (1<<3)    // D37


#define REG_SPI_MUX  8
#define SPI_MUX_ADC03     (1<<0)
#define SPI_MUX_DAC       (1<<1)
#define SPI_MUX_FLASH     (1<<2)
#define SPI_MUX_ADC       (1<<3)


#define REG_DAC  9
#define DAC_LDAC      (1<<0)
#define DAC_UNI_BIP_A (1<<1)
#define DAC_UNI_BIP_B (1<<2)
#define DAC_RST       (1<<3)

// TODO change name, RAILS_REG or REG_RAILS consistent with verilog?
#define REG_RAILS  10
#define RAILS_LP5V    (1<<0)
#define RAILS_LP15V   (1<<1)
#define RAILS_LP30V   (1<<2)
#define RAILS_LP60V   (1<<3)


#define REG_RAILS_OE  24
#define RAILS_OE      (1<<0)




// 11 is soft reset
#define CORE_SOFT_RST   11

#define REG_DAC_REF_MUX  12
#define DAC_REF_MUX_A     (1<<0)
#define DAC_REF_MUX_B     (1<<1)


#define REG_ADC  14
#define ADC_M0      (1<<0)
#define ADC_M1      (1<<1)
#define ADC_M2      (1<<2)
#define ADC_RST     (1<<3)


// dropping the _ctl, suffix?
// order follows dg444 pin order
#define REG_CLAMP1 15
#define CLAMP1_VSET     (1<<0)
#define CLAMP1_ISET     (1<<1)
#define CLAMP1_ISET_INV (1<<2)
#define CLAMP1_VSET_INV (1<<3)

#define REG_CLAMP2   16
#define CLAMP2_MIN        (1<<0)
#define CLAMP2_INJECT_ERR (1<<1)
#define CLAMP2_INJECT_VFB (1<<2)
#define CLAMP2_MAX        (1<<3)


#define REG_RELAY_COM  17
#define RELAY_COM_X         (1<<0)
#define RELAY_COM_Y         (1<<1)
#define RELAY_COM_Z         (1<<2)

#define REG_IRANGEX_SW     18
#define IRANGEX_SW1         (1<<0)
#define IRANGEX_SW2         (1<<1)
#define IRANGEX_SW3         (1<<2)
#define IRANGEX_SW4         (1<<3)



#define REG_RELAY       19
#define RELAY_VRANGE        (1<<0)
#define RELAY_OUTCOM        (1<<1)
#define RELAY_SENSE         (1<<2)


// better name? just irange_sense does not communicate its a mux
// #define IRANGE_MUX2_SENSE  20
#define REG_IRANGE_SENSE  20
#define IRANGE_SENSE1 (1<<0)
#define IRANGE_SENSE2 (1<<1)
#define IRANGE_SENSE3 (1<<2)
#define IRANGE_SENSE4 (1<<3)  // unused


// use bitmask write, to write these in individual combination
// OK. no. it's messy. we really want to separate these,
// into different regs to make easier to write...
#define REG_GAIN_IFB    21
#define GAIN_IFB_OP1  (1<<0)
#define GAIN_IFB_OP2  (1<<1)


/*
// better name?
#define REG_IRANGEX_SW58    22
#define IRANGEX_SW5         (1<<0)
#define IRANGEX_SW6         (1<<1)
#define IRANGEX_SW7         (1<<2)
#define IRANGEX_SW8         (1<<3)
*/


// same dg444 as REG_GAIN_IFB
// but separating makes it easier
#define REG_GAIN_VFB    23
#define GAIN_VFB_OP1  (1<<0)
#define GAIN_VFB_OP2  (1<<1)

/////////////////////////
// smu10
#define REG_INA_VFB_SW   25
#define INA_VFB_SW1_CTL   (1<<0)
#define INA_VFB_SW2_CTL   (1<<1)
#define INA_VFB_SW3_CTL   (1<<2)


// #define REG_INA_DIFF_SW   26
// #define INA_DIFF_SW1_CTL    (1<<0)
// #define INA_DIFF_SW2_CTL    (1<<1)



#define REG_INA_ISENSE_SW     27
#define ISENSE_SW1_CTL   (1<<0)
#define ISENSE_SW2_CTL   (1<<1)
#define ISENSE_SW3_CTL   (1<<2)


#define REG_INA_IFB_SW   28
#define INA_IFB_SW1_CTL  (1<<0)
#define INA_IFB_SW2_CTL  (1<<1)
#define INA_IFB_SW3_CTL  (1<<2)


#define REG_INA_VFB_ATTEN_SW   29
#define INA_VFB_ATTEN_SW1_CTL   (1<<0)
#define INA_VFB_ATTEN_SW2_CTL   (1<<1)
#define INA_VFB_ATTEN_SW3_CTL   (1<<2)


#define REG_ISENSE_MUX    30
#define ISENSE_MUX1_CTL   (1<<0)
#define ISENSE_MUX2_CTL   (1<<1)
#define ISENSE_MUX3_CTL   (1<<2)



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


