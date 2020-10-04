
#include "FreeRTOS.h"
#include "task.h"   // vTaskDelay

#include "sleep.h"


void task_sleep(uint32_t x)
{
  // only works in a task thread... not in main initialization thread
  vTaskDelay(pdMS_TO_TICKS( x )); // 1Hz
}


