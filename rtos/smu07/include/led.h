
// move gpio defines to central file,

// should define these in led.h for the board. or else move all code here
// led_setup() is separately defined...
#if 0
#define LED_PORT      GPIOE
#define LED_OUT       GPIO0
#endif

#define LED_PORT      GPIOA
#define LED_OUT       GPIO15



void led_setup(void);
void led_toggle(void);
void led_blink_task(void *args __attribute((unused)));
