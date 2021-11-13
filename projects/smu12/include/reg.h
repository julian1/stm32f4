


extern void reg_set( uint32_t spi, uint8_t r, uint8_t v);
extern void reg_clear( uint32_t spi, uint8_t r, uint8_t v);
extern void reg_write( uint32_t spi, uint8_t r, uint8_t v);
extern void reg_toggle( uint32_t spi, uint8_t r, uint8_t v);

extern void reg_write_mask( uint32_t spi, uint8_t r, uint8_t mask, uint8_t v);



#define REG_LED         7
#define LED1            (1<<0)
#define LED2            (1<<1)
// #define LED3            (1<<2)
// #define LED4            (1<<3)


#define REG_SPI_MUX     8
#define SPI_MUX_ADC03   (1<<0)
#define SPI_MUX_DAC     (1<<1)
#define SPI_MUX_FLASH   (1<<2)
#define SPI_MUX_ADC     (1<<3)


#define REG_DAC         9
#define DAC_LDAC        (1<<0)
#define DAC_UNI_BIP_A   (1<<1)
#define DAC_UNI_BIP_B   (1<<2)
#define DAC_RST         (1<<3)


#define REG_RAILS       10
#define RAILS_LP5V      (1<<0)
#define RAILS_LP15V     (1<<1)
#define RAILS_LP30V     (1<<2)
// #define RAILS_LP60V     (1<<3)

#define CORE_SOFT_RST   11


#define REG_DAC_REF_MUX   12
#define DAC_REF_MUX_A     (1<<0)
#define DAC_REF_MUX_B     (1<<1)


#define REG_ADC           14
#define ADC_M0            (1<<0)
#define ADC_M1            (1<<1)
#define ADC_M2            (1<<2)
#define ADC_RST           (1<<3)


// dropping the _ctl, suffix?
// order follows dg444 pin order
#define REG_CLAMP1        15
#define CLAMP1_VSET       (1<<0)
#define CLAMP1_ISET       (1<<1)
#define CLAMP1_ISET_INV   (1<<2)
#define CLAMP1_VSET_INV   (1<<3)

#define REG_CLAMP2        16
#define CLAMP2_MIN        (1<<0)
#define CLAMP2_INJECT_ERR (1<<1)
#define CLAMP2_INJECT_VFB (1<<2)
#define CLAMP2_MAX        (1<<3)


#define REG_RELAY_COM    17
#define RELAY_COM_X      (1<<0)
#define RELAY_COM_Y      (1<<1)
#define RELAY_COM_Z      (1<<2)

#define REG_IRANGE_X_SW  18
#define IRANGE_X_SW1_CTL (1<<0)
#define IRANGE_X_SW2_CTL (1<<1)
#define IRANGE_X_SW3_CTL (1<<2)
#define IRANGE_X_SW4_CTL (1<<3)



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
#define RELAY_OUT_COM_HC  (1<<0)
#define RELAY_OUT_COM_LC  (1<<1)



#define REG_RELAY_VSENSE  32
#define RELAY_VSENSE_CTL  (1<<0)



#define REG_IRANGE_YZ_SW  33
#define IRANGE_YZ_SW1_CTL (1<<0)
#define IRANGE_YZ_SW2_CTL (1<<1)
#define IRANGE_YZ_SW3_CTL (1<<2)
#define IRANGE_YZ_SW4_CTL (1<<3)



