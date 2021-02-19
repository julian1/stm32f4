/*

*/




#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>

#include <libopencm3/stm32/usart.h>
#include <libopencm3/cm3/nvic.h>


#include <libopencm3/cm3/systick.h>

// #include <libopencm3/cm3/scb.h>

#include <stddef.h> // size_t



#include "miniprintf2.h"

#define LED_PORT  GPIOE
#define LED_OUT   GPIO15


static void critical_error_blink(void)
{
	for (;;) {
		gpio_toggle(LED_PORT,LED_OUT);
		for(uint32_t i = 0; i < 500000; ++i)
       __asm__("nop");
	}
}




////////////////////////////////////////////////////////


static void led_setup(void)
{
  gpio_mode_setup(LED_PORT, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, LED_OUT);
}

/*
  EXTREME...
  OK. perhaps there's an actual hardware issue
  on the pin...
  due to overvoltage on the pin or something...
*/

////////////////////////////////////////////////////////





// isr can't take a context... so need global vars
static void	(*usart_cb_putc)(void *, char) = NULL;
static void *usart_cb_ctx = NULL;


static void usart_setup(
	void	(*putc)(void *, char),
	void 	*ctx
)
{
  // callback on interrupt
  usart_cb_putc = putc; 
  usart_cb_ctx = ctx;


  nvic_enable_irq(NVIC_USART1_IRQ);
  nvic_set_priority(NVIC_USART1_IRQ,  5 );    // changing this has no effect

  gpio_mode_setup(GPIOA, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO9 | GPIO10);


  gpio_set_af(GPIOA, GPIO_AF7, GPIO9 | GPIO10);

  gpio_set_output_options(GPIOA, GPIO_OTYPE_PP, GPIO_OSPEED_25MHZ, GPIO10);

  usart_set_baudrate(USART1, 115200);
  usart_set_databits(USART1, 8);
  usart_set_stopbits(USART1, USART_STOPBITS_1);
  // usart_set_mode(USART1, USART_MODE_TX_RX);

  usart_set_mode(USART1, USART_MODE_TX_RX);
  usart_set_parity(USART1, USART_PARITY_NONE);
  usart_set_flow_control(USART1, USART_FLOWCONTROL_NONE);


  // enabling any kind of interrupt makes freertos hang on interrupt
#if 1
  /* Enable USART1 Receive interrupt. */
  usart_enable_rx_interrupt(USART1);
#endif


  usart_enable(USART1);
}


void usart1_isr(void)
{
  // do nothing, still hangs

  /* Check if we were called because of RXNE. */
  if (((USART_CR1(USART1) & USART_CR1_RXNEIE) != 0) &&
      ((USART_SR(USART1) & USART_SR_RXNE) != 0)) {

    /* Retrieve the data from the peripheral. */
    char ch = usart_recv(USART1);

    usart_cb_putc( usart_cb_ctx, ch ) ;
  }

  return ;
}



////////////////////////////////////////////////////////

/* Common function descriptions */
// #include "clock.h"

/* milliseconds since boot */
static volatile uint32_t system_millis;



/* simple sleep for delay milliseconds */
static void msleep(uint32_t delay)
{
  // TODO. review. don't think this works on wrap around
  uint32_t wake = system_millis + delay;
  while (wake > system_millis);
}

#if 0
/* Getter function for the current time */
static uint32_t mtime(void)
{
  return system_millis;
}
#endif

static void clock_setup(void)
{
  /* clock rate / 168000 to get 1mS interrupt rate */
  // systick_set_reload(168000);
  systick_set_reload(16000);
  systick_set_clocksource(STK_CSR_CLKSOURCE_AHB);
  systick_counter_enable();

  /* this done last */
  systick_interrupt_enable();
}


////////////////////////////////////////////////////////

#if 0

static void led_update(void)
{
  bool state = (system_millis / 500)  % 2 == 0;
  // TODO, check last state... to avoid resetting.
  // bool state = (system_millis % 1000) > 500;
  if(state) {
    gpio_set(LED_PORT, LED_OUT);
  } else {
    gpio_clear(LED_PORT, LED_OUT);
  }
}
#endif



////////////////////////////////////////////////////////

typedef struct A
{
  char *p;
  size_t sz;
  size_t wi;
  size_t ri;
} A;


static void init(A *a, char *p, size_t sz)
{
  a->p = p;
  a->sz = sz;
  a->wi = 0;
  a->ri = 0;
}

static void write(A *a, char val)
{
  (a->p)[ a->wi] = val;

  a->wi = (a->wi + 1) % a->sz;
}

static int32_t read(A *a)
{
  if(a->ri == a->wi)
    return -1;

  char ret = (a->p)[ a->ri];

  a->ri = (a->ri + 1) % a->sz;
  return ret;
}



/////////////////////////////////////////////

// do we need a lib2?...

static void usart_update(A *aaa)
{
  while(true) {
    // order matters
    if(!usart_get_flag(USART1,USART_SR_TXE))
      return;

    int32_t ch = read(aaa);
    if(ch == -1)
      return;

    usart_send(USART1,ch);
  }
}


static A *console1;



void sys_tick_handler(void)
{
  // equivalent to a software timer
  // we are in an interupt  context here... so don't do anything
  // NOTE. we could actually set a flag.  or a vector of functions to call..
  // and then process in main loop.
  // eg. where things can be properly sequenced

  system_millis++;

  if( system_millis % 500 == 0) {

    gpio_toggle(LED_PORT, LED_OUT);
  }


  // one sec timer
  if( system_millis % 1000 == 0) {

    mini_snprintf( (void*)write, console1, "whoot %d\r\n", system_millis);
  }

}






int main(void)
{
  // LED
  rcc_periph_clock_enable(RCC_GPIOE);

  // USART
  rcc_periph_clock_enable(RCC_GPIOA);
  rcc_periph_clock_enable(RCC_USART1);


  // rcc_periph_clock_enable(RCC_SYSCFG); // should only be needed for external interupts.


  // initialize character circular buffer
  char buf[1000];
  struct A console;
  init(&console, buf, sizeof(buf));

  console1 = &console;


  led_setup();
  usart_setup( (void *)write, &console);     // usart char input, would actually use a different queue. eg. state machine.
  clock_setup();


  mini_snprintf((void *)write, &console, "starting\r\n");

  while(true) {

    // led_update();

    usart_update(&console);

    // test_update(&console);
  }


	for (;;);
	return 0;
}



#if 0
// wait. configure the usart interrupt...
// to do this...

static int done = 0;

static void test_update(A *console)
{

  // NO. we need  it in the sysclk kinterupt...

  int x = system_millis % 1000;
  
  if(x > 500 && !done) {
    done = true;
  } else if (x < 500 && done) {
    done = false;
  }
  

  if(done) {

#if 1
    // mini_snprintf( (void*)write, a, "whoot");
    mini_snprintf( (void*)write, console, "whoot %f\r\n", 123.456);
#endif
  }
}

#endif

