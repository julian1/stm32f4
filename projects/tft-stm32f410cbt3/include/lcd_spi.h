

// low-level spi primitives, that hide underlying stm32 spi details.

void lcd_spi_setup( void );



void lcd_spi_assert_rst(void);
void lcd_spi_deassert_rst(void);

void lcd_spi_turn_on_backlight( void );

// these will synch. - so should be used.
void lcd_spi_enable(void);
void lcd_spi_disable(void);


void lcd_send_command(uint8_t command, const uint8_t *dataBytes, uint32_t numDataBytes);


void lcd_send_command_repeat_data(uint8_t command, uint16_t x, uint32_t n );




