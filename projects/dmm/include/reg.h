
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
#define LED0            (1<<0)
#define LED1            (1<<1)
#define LED2            (1<<2)
#define LED3            (1<<3)




// need to rename named _4094_GLB_OE or similar to respect prefix convention

// rename this register... GENERAL REG_GENERAL.   else it's too confusing.
#define REG_4094        9
#define GLB_4094_OE    (1<<0)


#define REG_MODE        12
#define REG_TEST_PATTERN        14



