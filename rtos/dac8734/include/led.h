
// should define these in led.h for the board. or else move all code here
// led_setup() is separately defined...
#define LED_PORT      GPIOE
#define LED_OUT       GPIO0



void led_setup(void);
void led_toggle(void);
void led_blink_task(void *args __attribute((unused)));
