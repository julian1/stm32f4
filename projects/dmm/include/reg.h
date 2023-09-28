
// application specific.

#include <stdint.h>

/*
extern void ice40_reg_set( uint32_t spi, uint8_t r, uint8_t v);
extern void ice40_reg_clear( uint32_t spi, uint8_t r, uint8_t v);
extern void ice40_reg_write( uint32_t spi, uint8_t r, uint8_t v);
extern void ice40_reg_toggle( uint32_t spi, uint8_t r, uint8_t v);

extern void ice40_reg_write_mask( uint32_t spi, uint8_t r, uint8_t mask, uint8_t v);
*/



// This no longer controls the led.   it should be removed.
#define REG_LED         7

/*
#define LED0            (1<<0)
#define LED1            (1<<1)
#define LED2            (1<<2)
#define LED3            (1<<3)

*/


// need to rename named _4094_GLB_OE or similar to respect prefix convention

// rename this register... GENERAL REG_GENERAL.   else it's too confusing.
#define REG_4094          9
#define GLB_4094_OE       (1<<0)


#define REG_MODE          12



#define MODE_PATTERN       2
#define MODE_DIRECT        3

/*
// default led blink, and monitor test pattern.
#define REG_MODE_DEFAULT 0b00

// output state put under register control
#define REG_MODE_DIRECT  0b01
*/

#define REG_DIRECT        14      // need a different name.  REG_MODE_DIRECT_STATE


// 1of8 muxes.
#define SOFF        0
#define S1          ((1<<3)|(1-1))
#define S2          ((1<<3)|(2-1))
#define S3          ((1<<3)|(3-1))
#define S4          ((1<<3)|(4-1))
#define S6          ((1<<3)|(6-1))


// could also be a macro #define S(1) == ... 

