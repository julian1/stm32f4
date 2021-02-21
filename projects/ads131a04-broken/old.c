
#if 0
static void adc_timer_setup(void)
{
  /*
  // it is simple to setup a 1sec timer, and then count the readings we have, to determine
  // reading freq for noise.
  // than calculate from adc using clking dividers and fmod.

  // could use rtos, 
		vTaskDelay(pdMS_TO_TICKS(1000)); // 1Hz
  // no because, it's a delay, and not a sync timer
  // nice not to make peripheral dependent on freertos.

  //// But we shouldn't be doing anything complicated in interrupt (like printing ).
  */

  /*
    countdown timer.
    because for updatable countdown periods simpler.
    since don't have to subtract desired time from the period of a count-up timer.
    stm32f4, want tim2 or tim5, for 32 bit timers
  */
  rcc_periph_clock_enable(RCC_TIM3);
  nvic_enable_irq(NVIC_TIM3_IRQ);         /* Enable TIM3 interrupt. */

  // next level priority - albeit timing is certainly not important
  nvic_set_priority(NVIC_TIM3_IRQ,1);


  rcc_periph_reset_pulse(RST_TIM3);     // reset
  timer_set_mode(TIM3, TIM_CR1_CKD_CK_INT, TIM_CR1_CMS_EDGE, TIM_CR1_DIR_DOWN);
  timer_enable_break_main_output(TIM3);
  timer_set_prescaler(TIM3, 16000 );            // 16MHz / 16000 = 1kHz.

  // Ok faster appears to be more accurate...
  //period = 1000; // 1000 gives four digiss
  // static uint32_t period = 1000;


  timer_set_counter(TIM3, 1000 );

  timer_enable_irq(TIM3, TIM_DIER_UIE);   // counter update
  timer_enable_counter(TIM3);               // start timer, IMPROTANT should be done last!!!.... or will miss.



}


void tim3_isr(void)
{
  // timer interrupt, we've hit a apex or bottom of integration

  if (timer_get_flag(TIM3, TIM_SR_UIF)) {
    timer_clear_flag(TIM3, TIM_SR_UIF);

    // branch timing with if statement is unequal - but doesn't matter

  } // else???
}
#endif

// https://microcontrollerslab.com/change-period-reset-software-timer-freertos-arduino/







#if 0
TimerHandle_t xAutoReloadTimer;
BaseType_t xTimer2Started;

static void prvTimerCallback( TimerHandle_t xTimer );

static void timer_setup ( void) 
{
  // it looks like this needs to be done at init time.

  // Serial.begin(9600); // Enable serial communication library.

  xAutoReloadTimer = xTimerCreate(
    /* Text name for the software timer - not used by FreeRTOS. */
    "AutoReload",
    /* The software timer's period in ticks. */
    // mainAUTO_RELOAD_TIMER_PERIOD,
    pdMS_TO_TICKS( 1000 ),
    /* Setting uxAutoRealod to pdTRUE creates an auto-reload timer. */
    pdTRUE,
    /* This example does not use the timer id. */
    0,
    /* The callback function to be used by the software timer being created. */
    prvTimerCallback
  );

  /* Check the software timers were created. */
  if(( xAutoReloadTimer != NULL ) )
  {
  /* Start the software timers, using a block time of 0 (no block time). The scheduler has
  not been started yet so any block time specified here would be ignored anyway. */
  xTimer2Started = xTimerStart( xAutoReloadTimer, 0 );


  /* The implementation of xTimerStart() uses the timer command queue, and xTimerStart()
  will fail if the timer command queue gets full. The timer service task does not get
  created until the scheduler is started, so all commands sent to the command queue will
  stay in the queue until after the scheduler has been started. Check both calls to
  xTimerStart() passed. */


  // else fail


  if( ( xTimer2Started == pdPASS ) )
  {

    // we're good...

#if 0
  /* Start the scheduler. */
  vTaskStartScheduler();
#endif
  }
  }


}

/*
#find ../rtos/FreeRTOSv10.3.1/ | grep port.c | grep CM4 | grep GCC



cp ../rtos/FreeRTOSv10.3.1/FreeRTOS/Demo/CORTEX_M4F_STM32F407ZG-SK/FreeRTOSConfig.h ./rtos/ -i

cp ../rtos/FreeRTOSv202012.00/FreeRTOS/Source/include/FreeRTOS.h ./rtos/ -i

cp ../rtos/FreeRTOSv202012.00/FreeRTOS/Source/portable/GCC/ARM_CM4F/portmacro.h ./rtos -i


cp ../rtos/FreeRTOSv202012.00/FreeRTOS/Source/portable/GCC/ARM_CM4F/port.c   ./rtos -i

 cp ../rtos/FreeRTOSv202012.00/FreeRTOS/Source/portable/MemMang/heap_4.c rtos/

*/

/*
 For this RTOS API function to be available:

    configUSE_TIMERS and configSUPPORT_DYNAMIC_ALLOCATION must both be set to 1
in FreeRTOSConfig.h (configSUPPORT_DYNAMIC_ALLOCATION can also be left
undefined, in which case it will default to 1).
    The FreeRTOS/Source/timers.c C source file must be included in the build.
*/


static void prvTimerCallback( TimerHandle_t xTimer )
{
  (void) xTimer;






s#if 0
int main(void) 
{

  // ONLY WORKS if fit crystal.
  // rcc_clock_setup_pll(&rcc_hse_8mhz_3v3[RCC_CLOCK_3V3_168MHZ]);

  // LED
  rcc_periph_clock_enable(RCC_GPIOE); // LED_PORT JA
                                      // OK. this does enough to turn led on


  ///////////////
  led_setup();

#if 0
  while(true)
  {
		gpio_toggle(LED_PORT,LED_OUT);

    for(unsigned i = 0; i < 2 * 1000000; ++i) {
      __asm__("nop");
    }
  }
#endif

#if 1
  xTaskCreate(led_task,  "LED",100,NULL,configMAX_PRIORITIES-1,NULL);

	vTaskStartScheduler();
#endif

	for (;;);
	return 0;
}

#endif
tatic void prvTimerCallback( TimerHandle_t xTimer )
{
  (void) xTimer;


#if 0
TickType_t xTimeNow;
uint32_t ulExecutionCount;
/* A count of the number of times this software timer has expired is stored in the timer's
ID. Obtain the ID, increment it, then save it as the new ID value. The ID is a void
pointer, so is cast to a uint32_t. */
ulExecutionCount = ( uint32_t ) pvTimerGetTimerID( xTimer );
ulExecutionCount++;
vTimerSetTimerID( xTimer, ( void * ) ulExecutionCount );
/* Obtain the current tick count. */
xTimeNow = xTaskGetTickCount();
/* The handle of the one-shot timer was stored in xOneShotTimer when the timer was created.
Compare the handle passed into this function with xOneShotTimer to determine if it was the
one-shot or auto-reload timer that expired, then output a string to show the time at which
the callback was executed. */

Serial.print("Auto-reload timer callback executing ");
Serial.println( xTimeNow/31 );
if( ulExecutionCount >= 5 )
{
xTimerChangePeriod( xAutoReloadTimer, /* The timer being updated. */
mainAUTO_RELOAD_TIMER_PERIOD2, /* The new period for the timer. */
0 ); /* Do not block when sending this command. */
#endif
}

#endif


