

#include "FreeRTOS.h"
#include "task.h"
// #include "queue.h"



#include <libopencm3/stm32/gpio.h>

#include "led.h"


// should be highest proprity


// OK. this thing can lock up. which isn't fun...

// led blink task - stops / freezes .  which indicates something other than deadlocked queues.
// options
// - check the m4 coretex arch - and m4 freeRTOS config example differs - stack allocation? - check for differences
// - update freertos to current version.
// - check if other examples (not blinky) use more stack.
// - increase stack - see if fixes issue.
//



void led_setup(void) {

  gpio_mode_setup(LED_PORT, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, LED_OUT);
}


void led_toggle(void) {
  // avoid having to export LED_PORT, LED_OUT
  gpio_toggle(LED_PORT, LED_OUT);
}

void led_blink_task(void *args __attribute((unused))) {

	for (;;) {
		gpio_toggle(LED_PORT, LED_OUT);
		vTaskDelay(pdMS_TO_TICKS(500)); // 1Hz
    // eg. task_sleep()...
	}
}
