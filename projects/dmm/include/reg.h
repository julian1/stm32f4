
// application specific.

#include <stdint.h>

/*
extern void ice40_reg_set( uint32_t spi, uint8_t r, uint8_t v);
extern void ice40_reg_clear( uint32_t spi, uint8_t r, uint8_t v);
extern void ice40_reg_write( uint32_t spi, uint8_t r, uint8_t v);
extern void ice40_reg_toggle( uint32_t spi, uint8_t r, uint8_t v);

extern void ice40_reg_write_mask( uint32_t spi, uint8_t r, uint8_t mask, uint8_t v);
*/


#define REG_LED         7
#define LED0            (1<<0)
#define LED1            (1<<1)
#define LED2            (1<<2)
#define LED3            (1<<3)




// need to rename named _4094_GLB_OE or similar to respect prefix convention

// rename this register... GENERAL REG_GENERAL.   else it's too confusing.
#define REG_4094        9
#define GLB_4094_OE    (1<<0)


#define REG_MODE        12 


/*
// prefix with the IC ?

  U304_
  or _4094_U304_
k

*/

#if 0
#define U304_RAILS_LP5V_CTL   (1<<0)
#define U304_RAILS_LP15V_CTL  (1<<1)
#define U304_RAILS_LP24V_CTL  (1<<2)
#define U304_RAILS_LP50V_CTL  (1<<3)
#define U304_K301_L1_CTL      (1<<4)
#define U304_K301_L2_CTL      (1<<5)
#define U304_K302_L1_CTL      (1<<6)
#define U304_K302_L2_CTL      (1<<7)


////////////
// prefix with the domain. eg. analog domain. or relay domain. or high-side floating domain.
// driver U514.   could prefix.
#define U514_U506_K501_L1_CTL      (1<<1)
#define U514_U506_K501_L2_CTL      (1<<0)
#define U514_U506_K503_L1_CTL      (1<<2)
#define U514_U506_K503_L2_CTL      (1<<3)
#define U514_U506_K410_L1_CTL      (1<<4)
#define U514_U506_K410_L2_CTL      (1<<5)
#define U514_U506_K402_L1_CTL      (1<<6)
#define U514_U506_K402_L2_CTL      (1<<7)


#define U514_U507_K506_L1_CTL      (1<<8)
#define U514_U507_K506_L2_CTL      (1<<9)
#define U514_U507_K507_L1_CTL      (1<<10)
#define U514_U507_K507_L2_CTL      (1<<11)
#define U514_U507_K508_L1_CTL      (1<<12)
#define U514_U507_K508_L2_CTL      (1<<13)
#define U514_U507_K009_L1_CTL      (1<<14)
#define U514_U507_K009_L2_CTL      (1<<15)

//#define U514_U508_K506_L1_CTL      (1<<1)
// #define U514_U508_K506_L2_CTL      (1<<0)
#define U514_U508_K504_L1_CTL      (1<<18)
#define U514_U508_K504_L2_CTL      (1<<19)
#define U514_U508_K701_L1_CTL      (1<<20)
#define U514_U508_K701_L2_CTL      (1<<21)
#define U514_U508_K702_L1_CTL      (1<<22)
#define U514_U508_K702_L2_CTL      (1<<23)


////////////



////////////



#define REG_DAC         12    // TODO fix/remove

// #define REG_DAC         9
#define DAC_LDAC        (1<<0)
#define DAC_UNI_BIP_A   (1<<1)
#define DAC_UNI_BIP_B   (1<<2)
#define DAC_RST         (1<<3)


#define REG_RAILS       10
#define RAILS_LP5V      (1<<0)
#define RAILS_LP15V     (1<<1)
#define RAILS_LP24V     (1<<2)
#define RAILS_LP50V     (1<<3)

#define CORE_SOFT_RST   11


#define REG_DAC_REF_MUX   12    // TODO fix/ remove
#define DAC_REF_MUX_A     (1<<0)
#define DAC_REF_MUX_B     (1<<1)


#define REG_ADC           14
#define ADC_M0            (1<<0)
#define ADC_M1            (1<<1)
#define ADC_M2            (1<<2)
#define ADC_RST           (1<<3)


// dropping the _ctl, suffix?
// order follows dg444 pin order
#define REG_MUX_POL        15
#define MUX_POL_VSET       (1<<0)
#define MUX_POL_ISET       (1<<1)
#define MUX_POL_ISET_INV   (1<<2)
#define MUX_POL_VSET_INV   (1<<3)

#define REG_MUX_SEL        16
#define MUX_SEL_MIN        (1<<0)
#define MUX_SEL_INJECT_ERR (1<<1)
#define MUX_SEL_INJECT_VFB (1<<2)
#define MUX_SEL_MAX        (1<<3)


#define REG_RELAY_COM    17
#define RELAY_COM_X_CTL      (1<<0)
#define RELAY_COM_Y_CTL      (1<<1)
#define RELAY_COM_Z_CTL      (1<<2)

#define REG_IRANGE_X_SW  18
#define IRANGE_X_SW1_CTL (1<<0)
#define IRANGE_X_SW2_CTL (1<<1)
#define IRANGE_X_SW3_CTL (1<<2)
#define IRANGE_X_SW4_CTL (1<<3)


// read register.
#define REG_MON_RAILS     19

//#define REG_RELAY         19
//#define RELAY_VRANGE      (1<<0)
// #define RELAY_OUTCOM      (1<<1)
// #define RELAY_SENSE       (1<<2)


// better name? just irange_sense does not communicate its a mux
// #define IRANGE_MUX2_SENSE  20
//#define REG_IRANGE_SENSE  20
// #define IRANGE_SENSE1     (1<<0)
// #define IRANGE_SENSE2     (1<<1)
// #define IRANGE_SENSE3     (1<<2)
// #define IRANGE_SENSE4     (1<<3)  // unused



#define REG_RAILS_OE      24
#define RAILS_OE          (1<<0)


/////////////////////////
// smu10
#define REG_INA_VFB_SW    25
#define INA_VFB_SW1_CTL   (1<<0)
#define INA_VFB_SW2_CTL   (1<<1)
#define INA_VFB_SW3_CTL   (1<<2)



/*
#define REG_INA_ISENSE_SW 27
#define ISENSE_SW1_CTL    (1<<0)
#define ISENSE_SW2_CTL    (1<<1)
#define ISENSE_SW3_CTL    (1<<2)
*/

#define REG_INA_IFB_SW    28
#define INA_IFB_SW1_CTL   (1<<0)
#define INA_IFB_SW2_CTL   (1<<1)
#define INA_IFB_SW3_CTL   (1<<2)


#define REG_INA_VFB_ATTEN_SW  29
#define INA_VFB_ATTEN_SW1_CTL (1<<0)
#define INA_VFB_ATTEN_SW2_CTL (1<<1)
#define INA_VFB_ATTEN_SW3_CTL (1<<2)


#define REG_ISENSE_MUX    30
#define ISENSE_MUX1_CTL   (1<<0)
#define ISENSE_MUX2_CTL   (1<<1)
#define ISENSE_MUX3_CTL   (1<<2)



#define REG_RELAY_OUT     31
#define REG_RELAY_OUT_COM_HC_CTL  (1<<0)
#define REG_RELAY_GUARD_CTL       (1<<1)
#define REG_RELAY_SENSE_EXT_CTL   (1<<2)
#define REG_RELAY_SENSE_INT_CTL   (1<<3)





#define REG_RELAY_VSENSE  32
#define RELAY_VSENSE_CTL  (1<<0)



#define REG_IRANGE_YZ_SW  33
#define IRANGE_YZ_SW1_CTL (1<<0)
#define IRANGE_YZ_SW2_CTL (1<<1)
#define IRANGE_YZ_SW3_CTL (1<<2)
#define IRANGE_YZ_SW4_CTL (1<<3)


#endif
