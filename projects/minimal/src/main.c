/*
nix-shell ~/devel/nixos-config/examples/arm.nix
  rlwrap -a picocom -b 115200 /dev/ttyUSB0

  nix-shell ~/devel/nixos-config/examples/arm.nix
  make

  nix-shell ~/devel/nixos-config/examples/arm.nix
  cd smu11
  openocd -f ../../openocd.cfg

  nix-shell ~/devel/nixos-config/examples/arm.nix
  rlwrap nc localhost 4444

  reset halt ; flash write_image erase unlock ./projects/minimal/main.elf; sleep 1500; reset run


// vim :colorscheme default. loooks good.

// cjmcu  stm32f407.
// issue. is that board/stlink doesn't appear to reset cleanly. needs sleep.

// - can plug a thermocouple straight into the sense +p, +n, and
// then use the slow (digital) integrator/pid loop - as a temperature controler..


*/


#include <libopencm3/stm32/rcc.h>   // clock
#include <libopencm3/stm32/gpio.h>    // led
#include <libopencm3/stm32/spi.h>   // SPI1

#include <libopencm3/cm3/scb.h>  // reset()


#include <stdio.h>    // printf, scanf
#include <string.h>   // strcmp, memset
#include <ctype.h>    // isspace
#include <assert.h>





// library code
#include <lib2/usart.h>
#include <lib2/streams.h>
#include <lib2/util.h>   // msleep()
#include <lib2/cbuffer.h>
#include <lib2/cstring.h>
// #include <lib2/format.h>   // format_bits()




typedef struct app_t
{


  uint32_t led_port;
  uint32_t led_out;

  bool led_state ;     // should rename, or just use the last bit of the count .

  uint32_t soft_500ms ;

  // updated on interupt. should probably be declared volatile.
  // but functions that use can also declare volatile
  volatile uint32_t system_millis;

  CBuf  console_in;
  CBuf  console_out;
  
  CString     command;



} app_t;



static void app_update_soft_500ms(app_t *app)
{
  assert(app);

  /*
    function should reconstruct to localize scope of app. and then dispatch to other functions.
  */

  app->led_state = ! app->led_state;


  // blink mcu led
  // be explicit. don't hide top-level state.
  if(app->led_state)
    gpio_clear( app->led_port, app->led_out);
  else
    gpio_set(   app->led_port, app->led_out);


}




static char * trim_whitespace_inplace( char *cmd )
{
  assert(cmd);
  // messy functions - should probably get more testing.
  // trim leading whitespace.
  while(*cmd && isspace( (uint8_t ) *cmd ))
    ++cmd;

  char *p = cmd;    // position at string start
  while(*p) ++p;    // find string end
  --p;              // is this is a bug if given - zero length empty string ??.
                    // no because of (p>=cmd) check

  // trim trailing whitespace
  while(p >= cmd && isspace( (uint8_t) *p ))
    *p-- = 0;

  return cmd;
}




static void app_update_console_cmd(app_t *app)
{

  // rename - include REPL.  repl ( ) conso

  while( ! cBufisEmpty(&app->console_in)) {

    // got a character
    int32_t ch = cBufPop(&app->console_in);
    assert(ch >= 0);

    if(ch != '\r' && ch != ';' && cStringCount(&app->command) < cStringReserve(&app->command) ) {
      // must accept whitespace here, since used to demarcate args
      // normal character
      cStringPush(&app->command, ch);
      // echo to output. required for minicom.
      putchar( ch);

    }  else {

      // process...
      // newline or overflow
      putchar('\n');

      char *cmd = cStringPtr(&app->command);

      cmd = trim_whitespace_inplace( cmd );

      // useful for debug
      // printf("cmd '%s'  %u\n", cmd, strlen(cmd) );


/*
      uint32_t u0 , u1;
      char s0[100 + 1 ];
*/
      ////////////////////

      if(strcmp(cmd, "help") == 0) {

        printf("help <command>\n" );
      }


      else if(strcmp(cmd, "reset mcu") == 0) {

        printf("perform mcu reset\n" );
        // reset stm32f4
        // scb_reset_core()
        scb_reset_system();
      }


      else {

        printf("unknown cmd, or bad argument '%s'\n", cmd );

      }



      // reset buffer
      cStringClear( &app->command);

      // issue new command prompt
      printf("> ");
    }
  }
}





static void app_loop(app_t *app)
{

  while(true) {

    // handle console
    app_update_console_cmd(app);

    // 500ms soft timer
    if( (app->system_millis - app->soft_500ms) > 500) {
      app->soft_500ms += 500;
      app_update_soft_500ms(app);
    }

  }
}





static void sys_tick_interupt(app_t *app)
{
  // interupt context. don't do anything compliicated here.

  ++ app->system_millis;
}







/////////////////////////
/*
  TODO.
  Maybe move raw buffers into app structure?


  Why not put on the stack? in app_t ?
*/

static char buf_console_in[1000];
static char buf_console_out[1000];    // changing this and it freezes. indicates. bug

// static char buf_cmds[1000];

static char buf_command[1000];


// move to main ?
// no reason to be static.
// no... because we collect/assemble dependencies. ok in main()
static app_t app;




#if 1


// static Mode mode_current;


int main(void)
{
  // high speed internal!!!
  // TODO. not using.

	rcc_clock_setup_pll(&rcc_hse_8mhz_3v3[RCC_CLOCK_3V3_84MHZ] );  // stm32f411  upto 100MHz.


  // clocks
  rcc_periph_clock_enable(RCC_SYSCFG); // maybe required for external interupts?

  // LED
  // rcc_periph_clock_enable(RCC_GPIOE);
  rcc_periph_clock_enable(RCC_GPIOA);

  // USART
  rcc_periph_clock_enable(RCC_GPIOB); // F410 / f411
  rcc_periph_clock_enable(RCC_USART1);


  // spi / ice40
  rcc_periph_clock_enable(RCC_SPI1);


  // adc/temp
  rcc_periph_clock_enable(RCC_ADC1);

  // this isn't right. it's for the fan.
  // gpio_mode_setup(GPIOA, GPIO_MODE_ANALOG, GPIO_PUPD_NONE, GPIO0);
  // gpio_mode_setup(GPIOA, GPIO_MODE_ANALOG, GPIO_PUPD_NONE, GPIO1);


  /*
    Do led first, even if need update() and systick loop() to blink it.
  */

  // for temp
  // adc_setup();



#define LED_PORT  GPIOA
#define LED_OUT   GPIO9

  gpio_mode_setup(LED_PORT, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, LED_OUT);

  // setup external state for critical error led blink
  // because assert() cannot pass a context
  critical_error_led_setup(LED_PORT, LED_OUT);


  // this is the mcu clock.  not the adc clock. or the fpga clock.
  // systick_setup(16000);

  // extern void systick_setup(uint32_t tick_divider, void (*pfunc)(void *),  void *ctx);
  systick_setup(84000,  (void (*)(void *)) sys_tick_interupt, &app);  // 84MHz.


  //////////////////////
  // main app setup

  memset(&app, 0, sizeof(app_t));


  app.led_port = LED_PORT;
  app.led_out  = LED_OUT;




  // uart/console
  cBufInit(&app.console_in,  buf_console_in, sizeof(buf_console_in));
  cBufInit(&app.console_out, buf_console_out, sizeof(buf_console_out));

  cbuf_init_stdout_streams(  &app.console_out );
  cbuf_init_stdin_streams( &app.console_in );


  cStringInit(&app.command, buf_command, buf_command + sizeof( buf_command));

  //////////////
  // initialize usart before start all the app constructors, so that can print.
  // uart
  // usart1_setup_gpio_portA();
  usart1_setup_gpio_portB();

  usart1_set_buffers(&app.console_in, &app.console_out);



  printf("\n\n\n\n--------\n");
  printf("addr main() %p\n", main );

  printf("\n--------");
  printf("\nstarting\n");


  assert( sizeof(bool) == 1);
  assert( sizeof(float) == 4);
  assert( sizeof(double ) == 8);


  // sep 16, 2023.
  // set ER-CRESET-  hi.  not to reset.   This needs to have an external  pullup fitted.
#define ER_CRESET_PORT  GPIOA
#define ER_CRESET_PIN   GPIO1

  gpio_mode_setup(ER_CRESET_PORT, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, ER_CRESET_PIN);
  gpio_set(ER_CRESET_PORT, ER_CRESET_PIN);



  printf("sizeof app_t %u\n", sizeof(app_t));


  // modes_init();

  // go to main loop
  app_loop(&app);

  for (;;);
  return 0;


}



#endif



/*
  probably from printf
  _close_r  _fstat_r _getpid_r _kill_r _isatty_r

  see.
    -specs=nano.specs -specs=nosys.specs

    arm-none-eabi-gcc: fatal error: /nix/store/3ydyllv3y22qpxcgsf9miwq4dkjwjcj2-gcc-arm-embedded-12.2.rel1/bin/../lib/gcc/arm-none-eabi/12.2.1/../../../../arm-none-eabi/lib/nosys.specs: attempt to rename spec 'link_gcc_c_sequence' to already defined spec 'nosys_link_gcc_c_sequence'


    --specs=rdimon.specs

  https://stackoverflow.com/questions/73742774/gcc-arm-none-eabi-11-3-is-not-implemented-and-will-always-fail
*/

#if 1

void _close_r( void );
void _fstat_r( void );
void _getpid_r( void);
void _kill_r( void);
void _isatty_r( void);
void _lseek_r( void);
void _read_r( void);
void _write_r( void);


void _close_r( void )
{
  assert(0);
}

void _fstat_r( void )
{
  assert(0);
}

void _getpid_r( void)
{
  assert(0);
}

void _kill_r( void)
{
  assert(0);
}

void _isatty_r( void)
{
  assert(0);
}

void _lseek_r( void)
{
  assert(0);
}

void _read_r( void)
{
  assert(0);
}

void _write_r( void)
{
  assert(0);
}

#endif





