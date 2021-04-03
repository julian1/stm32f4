/*
  helper stuff that belongs in separate file, but not in separate library
  because might change
*/


extern void led_setup(void);
extern void led_toggle(void);
extern void critical_error_blink(void);


/////////////////////


extern void systick_setup(uint32_t tick_divider);
extern volatile uint32_t system_millis;
extern void msleep(uint32_t delay);

///////////////

extern void usart_printf( const char *format, ... );
// extern int printf( const char *format, ... );  compiler complains about inbuit formatting conventions
extern void flush( void);
extern void usart_setup_( void );


//////////////////
uint32_t spi_write_register_16(uint32_t spi, uint32_t r);

///////////////
extern void spi1_port_setup(void);
extern void spi1_special_gpio_setup(void);

extern void spi_special_flag_clear(uint32_t spi);
extern void spi_special_flag_set(uint32_t spi);



#define UNUSED(x) (void)(x)

