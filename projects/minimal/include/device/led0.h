

#pragma once



#include <peripheral/led.h>




// constructors, non opaque, should be called in main()
led_t *led0_create(void);


// could even pass
// led_t *led0_create2(uint32_t port, uint32_t gpios);   // another way...



