


#include <libopencm3/cm3/nvic.h>
#include <libopencm3/stm32/exti.h>
#include <libopencm3/stm32/gpio.h>


#include "ui_events.h"
#include "cbuffer.h"
#include "util.h"
#include "assert.h"





static CBuf *ui_events_in = NULL;

static void rotary_events_init(void)
{
  // rotary gpio and timer, already configured, in rotary.

  // ie. use exti9-5 for pa8, pa9 for rotary input
  nvic_enable_irq(NVIC_EXTI9_5_IRQ);

  exti_select_source(EXTI8, GPIOA);
  exti_set_trigger(EXTI8, EXTI_TRIGGER_FALLING);
  exti_enable_request(EXTI8);

  exti_select_source(EXTI9, GPIOA);
  exti_set_trigger(EXTI9, EXTI_TRIGGER_FALLING);
  exti_enable_request(EXTI9);
}




#define TACTILE_SW_PORT   GPIOC
#define TACTILE_SW1_IN    GPIO13
#define TACTILE_SW2_IN    GPIO14


#define ROTARY_SW_PORT   GPIOA
#define ROTARY_SW1       GPIO10

static void button_events_init(void)
{
  // MUST be careful. to keep interupts on different pin numbers - when using more than one port.
  // eg. cannot get interrupts on both PA10 and PB10.


  uint16_t all = TACTILE_SW1_IN | TACTILE_SW2_IN;

  gpio_mode_setup(TACTILE_SW_PORT, GPIO_MODE_INPUT, GPIO_PUPD_NONE, all );

  // WARN. this is already setup/repeated in the rotary decoder exti setup.
  // this needs much better factoring.
  nvic_enable_irq(NVIC_EXTI15_10_IRQ);
  nvic_set_priority(NVIC_EXTI15_10_IRQ, 5 );

  // works.
  exti_select_source(all, TACTILE_SW_PORT);
  exti_set_trigger(all, EXTI_TRIGGER_FALLING);
  exti_enable_request(all);

  ////////////////
  // rotary button.
  gpio_mode_setup(ROTARY_SW_PORT, GPIO_MODE_INPUT, GPIO_PUPD_NONE, ROTARY_SW1);

  exti_select_source(ROTARY_SW1, ROTARY_SW_PORT);
  exti_set_trigger(ROTARY_SW1, EXTI_TRIGGER_FALLING);
  exti_enable_request(ROTARY_SW1);

}





void ui_events_init(  CBuf *ui_events_in_ )
{
/*
  EXTR
    even though we cannot get the dimter value during an interupt, it doesn't matter.
    because we only need to queue it and start a on_begin_edit() to record the initial rotary timer value.
*/

  ui_events_in = ui_events_in_;

  rotary_events_init();
  button_events_init();
}


void exti9_5_isr(void)
{
  exti_reset_request(EXTI8 | EXTI9);

  // got rotary encoder event
  cBufPush(ui_events_in, ui_events_rotary_change);
}



void exti15_10_isr(void)
{
  // need to decode...

  uint32_t all = EXTI10 | EXTI13 | EXTI14 ;

  // note that this holds.
  assert(EXTI10 == GPIO10);

  uint32_t flags = exti_get_flag_status ( all /*0xffffffff */);

  if(flags & EXTI10)
    cBufPush(ui_events_in, ui_events_button_rotary);

  if(flags & EXTI13)
    cBufPush(ui_events_in, ui_events_button_left);

  if(flags & EXTI14)
    cBufPush(ui_events_in, ui_events_button_right);

  exti_reset_request( all );

  // usart1_printf("interupt button pressed %d\n", flags);
}




