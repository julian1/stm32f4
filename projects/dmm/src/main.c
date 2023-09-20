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
  reset halt ; flash write_image erase unlock ./main.elf; sleep 1500; reset run


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


#include <stdio.h>    // printf
#include <string.h>   // strcmp, memset


// library code
#include "usart.h"
#include "assert.h"
// #include "cbuffer.h"
// #include "cstring.h"
#include "streams.h"
#include "util.h"


#include "spi-port.h"
#include "spi-ice40.h"
#include "mux.h"
#include "reg.h"



#include "fbuffer.h"

#include "format.h"   // format_bits()



// app structure
#include "app.h"

// #include "dac8734.h"
#include "4094.h"



static void spi1_interupt(app_t *app)
{
  UNUSED(app);
#if 0
  /*
    interupt context. avoid doing work here...
  */
  if(app->adc_drdy == true) {
    // still flagged from last time, then code is too slow, and we missed an adc read
    ++app->adc_drdy_missed;
  }

  // set adc_drdy flag so that update() knows to read the adc...
  app->adc_drdy = true;
#endif
}


static void update_soft_1s(app_t *app)
{
  // maybe review this...
  UNUSED(app);


}



static uint32_t write_bits8( uint8_t v, uint8_t set, uint8_t clear )
{
  // *v = ~(  ~(*v | set ) | clear);  // clear has priority over set

  return ~( ~v | clear)  | set;    // set has priority
}


static uint8_t create_mask8( uint8_t pos, uint8_t width)
{
  uint8_t val = (1 << (pos + width )) - 1;   // set all bits below pos + width.
  uint8_t val2 = (1 << pos) - 1;            // set all bits below pos

  return val & ~val2;
}




static uint8_t write_val8 ( uint8_t v, uint8_t pos, uint8_t width, uint8_t value)
{
  // write value into v - at pos with width
  assert(pos < 8);
  assert(width < 8);

  uint8_t mask = create_mask8( pos, width);

  return write_bits8( v, (value << pos) & mask, mask);
}


// we manipulate the state. then write it.
// there is writing state to state var. then there is spi writing it.

static void state_write ( uint8_t *state, size_t n, unsigned pos, uint8_t width, uint8_t value)
{
  // determine index to use into byte array, and set value
  unsigned idx = pos >> 3 ;   // div 8
  assert( idx < n );
  uint8_t *v = & state[ idx ];

  *v = write_val8 ( *v, pos % 8, width, value);
}



static void state_format ( uint8_t *state, size_t n)
{
  assert(state);

  char buf[100];
  for(unsigned i = 0; i < n; ++i ) {

    printf("v %s\n",  format_bits(buf, 8, state[ i ]  ));
  }
}



/*
  EXTR. we can do the three step sequence of b2b fets in two steps. relying on faster speed

  1. turn off b2b fets.   turn on relay.   relay slower.
  2. turn off latch to relay. and turn on b2b fets.         <- so this is more than a mask.
  -----------------

  OR.    have two bits. and the interpreter.   will manage

  Eg. b2b-fets stage1. b2b-fets stage2.   - and encode... issue is that noo

  --------
  - So we don't duplicate everything.  just if a relay has a different state.
  - OR we just encode all states - as two states.  transition
  - eg. just double  the bitvector.
  ---
  - then we can encode.   - and don't need messy mask abstractions.

*/

#define MODE_ARR_N    3   // mode array in bytes.


typedef struct X
{
  // U406
  uint8_t U408_SW_CTL : 1;
  uint8_t U406_UNUSED : 1;
  uint8_t K406_CTL    : 2;        // Be better to encode as 2 bits.   can then assign 0b01 or 0b10 etc.
  uint8_t U406_UNUSED_2 : 4;

  // U401
  uint8_t U401_UNUSED : 8;


  // U403
  uint8_t U403_UNUSED : 6;
  uint8_t K405_CTL    : 2;
  // uint8_t K405_L1_CTL : 1;
  // uint8_t K405_L2_CTL : 1;


  // U506
  uint8_t U506_UNUSED : 8;

  // jumpered.
  // 600
  // 700

  // U1004  - 5th bit.
  uint8_t U1003   : 4;    // adg1208  4 bits.  EN last. is inconsistent.  with chip pin-order. and 500, 600, 700.  good keep.....
  uint8_t U1006   : 4;    // adg1208  4 bits.


} X;



// mode_t
typedef struct Mode
{
  // put AZ mux here. also.

  X     first;
  X     second;

} Mode;


/*
  EXTR.
    don't care about defining inidividual registers for muxes etc.
    instead the entire state representation is considered as register. with a pre-determined set of modes / elements.

    - the clearing mask for relays, is normally always the same. but the need to manipulate b2b fets changes thing.
    - with a straight array.   WE *CAN* also define using a parallel alternative structure with bitfield.
  ----
    - sequencing - may also want to switch relays, wait. then turn on the analog switches.

    - EXTR. THE state of ALL relays must be defined.  0 just means use prior state.  which is wrong.  either L1, or L2.  not 0.

    - It would be easier to do this with a memcpy.

                                      first transition            // second transition
                                      U406    U401
*/

// is wrong. we have to switch all the relays to a defined state


// Mode mode_zero;     //  useful to work out which pin is flipping .
// change name mode_initial. to mode-off/ or disconnect
Mode mode_initial;      // all inputs turned off.

Mode mode_dcv_az ;

/* IMPORTANT the precharge switch relay - is held constant/closed for the mode.
  but we cycle one of the muxes - to read gnd/ to reset the charge on the cap.
  we can also. use dc-source to add an initial voltage bias to cap.
*/

// Mode mode_test_accumulation;


static void init_modes( void )
{
  /*
    instead of having individual registers. we have individual elemental mode states , that exist.

    is there an alignment issue... with setting the bitfield???

  */

  // memset( &mode_zero,    0, sizeof( Mode) );
  memset( &mode_initial, 0, sizeof( Mode) );
  memset( &mode_dcv_az,  0, sizeof( Mode) );
  // memset( &mode_test_accumulation,  0, sizeof( Mode) );

  //  should be explicit for all values  U408_SW_CTL. at least for the initial mode, from which others derive.
  mode_initial.first .K406_CTL  = 0b10;     // accumulation relay off
  mode_initial.first .U408_SW_CTL = 0;      // b2b fets/ input protection off/open
  mode_initial.first. K405_CTL  = 0b01;     // dcv-input relay k405 switch off

  mode_initial.second.K406_CTL  = 0b00;     // clear relay
  mode_initial.second.U408_SW_CTL = 0;
  mode_initial.second.K405_CTL  = 0b00;     // clear relay

  /* EXTR. with series resistors to 4094 - for driving lower coil-voltage relays - there is a drop on the drive side.
      AND a drop on the lo side.  eg. with 50R. this looks like a poor on-pulse

  */

  //////////
  mode_dcv_az = mode_initial;               // eg. turn all relays off
  mode_dcv_az.first.K405_CTL    = 0b10;     // turn dcv-input K405 on.
  mode_dcv_az.first.U408_SW_CTL = 0;        // turn off b2b fets, while switching relay on.
  mode_dcv_az.second.K405_CTL    = 0b00;    // clear relay.  don't really need since inherits from initial.
  mode_dcv_az.second.U408_SW_CTL = 1;       // turn on/close b2b fets.


  ////
  // cap-accumulation mode.
  // mode_test_accumulation = mode_initial;
  // mode_test_accumulation.first .K406_CTL  = 0b01;  // accumulation relay on

}





// flashing of led... is writing ????


static void do_transition( unsigned spi, Mode *mode)
{
  // should we be passing as a separate argument
  assert( sizeof(X) == 5 );


  // mux spi to 4094. change mcu spi params, and set spi device to 4094
  mux_4094( spi);


  printf("-----------\n");

  printf("do_transition write first state\n");
  state_format (  (void *) &mode->first, sizeof(X) );

  // and write device
  spi_4094_reg_write_n(spi, (void *) &mode->first, sizeof( X) );

  // sleep 10ms
  msleep(10);


  // and format
  printf("do_transition write second state\n");
  state_format ( (void *) &mode->second, sizeof(X) );

  // and write device
  spi_4094_reg_write_n(spi, (void *) &mode->second, sizeof(X) );
}






static void update_soft_500ms(app_t *app)
{
  // UNUSED(app);

  /*
    function should reconstruct to localize scope of app. and then dispatch to other functions.

  */

  //////////////////
  // EXTR. these static vars belong in app, to make this more testable.
  // char buf[100];
  static bool led_state = 0;
  led_state = ! led_state;


  static int count = 0;
  printf("count %u\n", ++ count);


  // blink the mcu led
  led_set( led_state );



  // blink the fpga led
  mux_ice40(app->spi);


  // doing a read.... of reg_4094 - is interferre

  // Ok, writing led state. screws up the signalling.

  if(led_state)
    spi_ice40_reg_write32(app->spi, REG_LED, LED0);
  else
    spi_ice40_reg_write32(app->spi, REG_LED, 0 );   // we don't have the set and clear bits...


  /*

  //
  if(led_state)
    do_transition( app->spi, &mode_initial);
  else
    do_transition( app->spi, &mode_dcv_az);
*/

  // OK.
  // hang on. we know the bits ...
  // seems to work.

  if(led_state) {
    Mode j = mode_initial;
    j.second.U1003  = 0b1000;   // s1. +10V.
    // j.second.U1006  = 0b1000;   // s1. ref 10V.
    // j.second.U1006  = 0b1001;   // s2. ref 1V.
    j.second.U1006  = 0b1010;       // s3. ref 0.1V.
    do_transition( app->spi, &j );
  }
  else {

    Mode j = mode_initial;
    j.second.U1003  = 0b1001;   // s2.  -10V.
    // j.second.U1006  = 0b1000;   // s1.    +10.
    // j.second.U1006  = 0b1001;   // s2. ref 1V.
    j.second.U1006  = 0b1010;       // s3. ref 0.1V.
    do_transition( app->spi, &j);
  }




  // turn off 4094 muxing
  mux_ice40(app->spi);



}





static void update_console_cmd(app_t *app)
{


  while( ! cBufisEmpty(&app->console_in)) {

    // got a character
    int32_t ch = cBufPop(&app->console_in);
    assert(ch >= 0);

    if(ch != '\r' && cStringCount(&app->command) < cStringReserve(&app->command) ) {
      // normal character
      cStringPush(&app->command, ch);
      // echo to output. required for minicom.
      putchar( ch);

    }  else {

      // newline or overflow
      putchar('\n');

      char *cmd = cStringPtr(&app->command);

      // printf("cmd whoot is '%s'\n", cmd);


      uint32_t u0;
      UNUSED(u0);

      if( strcmp(cmd, "test") == 0) {

        printf("got test\n");
      }

      else if(strcmp(cmd, "reset mcu") == 0) {
        // reset stm32f4
        // scb_reset_core()
        scb_reset_system();
      }


/*
      else if( strcmp(cmd, "relay on") == 0) {  // output on

        relay_set( app->spi, app->state_4094, sizeof(app->state_4094), REG_K406,  1 );
      }
      else if( strcmp(cmd, "relay off") == 0) {  // output off

        relay_set( app->spi, app->state_4094, sizeof(app->state_4094), REG_K406,  0 );
      }
*/


#if 0
      else if(strcmp(cmd, "mon") == 0) {

        // this will powerdown rails
        mux_ice40(app->spi);
        uint8_t val = ice40_reg_read( app->spi, REG_MON_RAILS );
        char buf[10];
        printf("reg_mon_rails read bits %s\n", format_bits(buf, 4, val) );

      }


      else if(strcmp(cmd, "reset") == 0) {

        // this will powerdown rails
        printf("reset ice40\n");
        mux_ice40(app->spi);
        ice40_reg_set(app->spi, CORE_SOFT_RST, 0 );
      }


      else if(strcmp(cmd, "powerup") == 0) {

        // ok without dac being initialized?
        // powerup 5 and +-15V rails and set analog switches

        printf("powerup ice40\n");
        mux_ice40(app->spi);

        // check rails monitor.
        uint8_t val = ice40_reg_read( app->spi, REG_MON_RAILS );
        // char buf[10];
        // printf("reg_mon_rails read bits %s\n", format_bits(buf, 4, val) );

        if(val == 1) {
          ice40_reg_set(app->spi, 6 , 0 );
        }  else {
          printf("problem with rails monitor\n");
        }
      }


      else if( strcmp(cmd, "on") == 0) {  // output on

        ice40_reg_set(app->spi, REG_RELAY_OUT, REG_RELAY_OUT_COM_HC_CTL);
        ice40_reg_set(app->spi, REG_LED, LED1);
      }
      else if( strcmp(cmd, "off") == 0) {  // output off

        ice40_reg_clear(app->spi, REG_RELAY_OUT, REG_RELAY_OUT_COM_HC_CTL);
        ice40_reg_clear(app->spi, REG_LED, LED1);
      }

      else if( strcmp(cmd, "guard on") == 0) {  // output on

        ice40_reg_set(app->spi, REG_RELAY_OUT, REG_RELAY_GUARD_CTL);
      }
      else if( strcmp(cmd, "guard off") == 0) {  // output off

        ice40_reg_clear(app->spi, REG_RELAY_OUT, REG_RELAY_GUARD_CTL);
      }


      else if( sscanf(cmd, "sense ext %lu", &u0 ) == 1) {

        if(u0) {
          ice40_reg_clear(app->spi, REG_RELAY_OUT, REG_RELAY_SENSE_INT_CTL);    // FIXME. write not set // perhaps push to own register to make exclusive.
          ice40_reg_set(app->spi, REG_RELAY_OUT, REG_RELAY_SENSE_EXT_CTL);
        } else {
          ice40_reg_set(app->spi, REG_RELAY_OUT, REG_RELAY_SENSE_INT_CTL);
          ice40_reg_clear(app->spi, REG_RELAY_OUT, REG_RELAY_SENSE_EXT_CTL);
        }

      }






#endif


      // chage name to start, init sounds like initial-condition

      else if( strcmp(cmd, "start") == 0) {

        // app_start2( app );
      }

      else if( strcmp( cmd , "") == 0) {

      }

      else {

            printf("unknown '%s'\n", cmd );

      }



      // reset buffer
      cStringClear( &app->command);

      // issue new command prompt
      printf("> ");
    }
  }
}



static void loop(app_t *app)
{
  // move this into the app var structure ?.
  // static uint32_t soft_50ms = 0;
  static uint32_t soft_500ms = 0;
  static uint32_t soft_1s = 0;

  /*
    Think all of this should be done/moved to update()...
  */


  // while(true);

  while(true) {


    // EXTREME - could actually call update at any time, in a yield()...
    // so long as we wrap calls with a mechanism to avoid stack reentrancy
    // led_update(); in systick.
    // but better just to flush() cocnsole queues.conin/out

//    update(app);

     /*
        JA
        we always want to update console first. so that we can always issue commands.
        and know that we have control
        -----------------

        All of this code needs to be refactored. so that the the command dispatch happens in the top-level loop.
    */

    update_console_cmd(app);



    // 500ms soft timer. should handle wrap around
    if( (system_millis - soft_500ms) > 500) {
      soft_500ms += 500;
      update_soft_500ms(app);
    }


    if( (system_millis - soft_1s) > 1000 ) {

      // THIS IS FUNNY....
      soft_1s += 1000;
      update_soft_1s(app);
    }


#if 0
    if(app->state == STATE_FIRST) {

      // rename ca
      state_change(app, STATE_ANALOG_UP);
    }
#endif


  }
}




#if 0

static void assert_app(app_t *app, const char *file, int line, const char *func, const char *expr)
{
  /*
    note the usart tx interupt will continue to flush output buffer,
    even after jump to critical_error_blink()
  */
  printf("\nassert_app failed %s: %d: %s: '%s'\n", file, line, func, expr);

  state_change(app, STATE_HALT );

#if 1
  // we have to do a critical error here... else caller code will just progress,
  // if we were in a state transition, then it will continue to just progress...
  critical_error_blink();
#endif
}

#endif








/////////////////////////
/*
  TODO.
  Maybe move raw buffers into app structure?


  Why not put on the stack? in app_t ?
*/

static char buf_console_in[1000];
static char buf_console_out[1000];

// static char buf_cmds[1000];

static char buf_command[1000];




// move init to a function?
// no... because we collect/assemble dependencies. ok in main()
static app_t app;



// chage name spi_ice40_stress test.

static void spi_ice40_stress_test_spi( uint32_t spi)
{
  // TODO better name. move code to separate test folder.
  // prefix with test ? perhaps

  printf("stress test spi comms\n");
  mux_ice40(spi);
  uint8_t magic = 0;

  while(1) {
    assert(REG_LED == 7);
    spi_ice40_reg_write32(spi, REG_LED, magic++);
    // spi_ice40_reg_write( spi, REG_LED, magic ++ );
    uint32_t ret = spi_ice40_reg_read32( spi, REG_LED);
    char buf[ 100] ;
    printf("v %lu  %s\n",  ret,  format_bits(buf, 32, ret ));
    msleep( 150);
  }
}


static void spi_ice40_wait_for_ice40( uint32_t spi)
{
  // TODO better ame doto

  mux_ice40(spi);

  printf("wait for ice40\n");
  uint32_t ret = 0;
  // uint8_t magic = 0b0101; // ok. not ok now.  ok. when reset the fpga.
  uint8_t magic = 0b1010;   // this is returning the wrong value....
  do {
    printf(".");

    // ice40_reg_set( spi, REG_LED,  magic );
    spi_ice40_reg_write32( spi, REG_LED, magic);
    // ret = ice40_reg_read( spi, REG_LED);
    ret = spi_ice40_reg_read32( spi, REG_LED);

    char buf[ 100] ;
    printf("v %s\n",  format_bits(buf, 8, ret ));

    msleep( 50);
  }
  while( ret != magic );
  printf("\n");

}



static void spi_ice40_just_read_reg ( uint32_t spi)
{
  mux_ice40(spi);

  while(1) {
    uint32_t v = spi_ice40_reg_read32(spi, REG_LED);
    // uint32_t v = spi_ice40_reg_read32(spi, 0b00111111 );
    char buf[32+1];
    printf("value of led %lu %s\n", v, format_bits(buf, 32, v ));
    msleep( 500 );
  }
}






int main(void)
{
  // high speed internal!!!
  // TODO. not using.

	rcc_clock_setup_pll(&rcc_hse_8mhz_3v3[RCC_CLOCK_3V3_84MHZ] );  // stm32f411  upto 100MHz.

  // this is the mcu clock.  not the adc clock. or the fpga clock.
  // systick_setup(16000);
  systick_setup(84000);  // 84MHz.

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

  /*
    Do led first, even if need update() and systick loop() to blink it.
  */

#define LED_PORT  GPIOA
#define LED_OUT   GPIO9

  led_setup(LED_PORT, LED_OUT);




  //////////////////////
  // main app setup

  memset(&app, 0, sizeof(app_t));


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



#if 0
  {
    char buf[100];

      // printf("val %s\n",  format_bits(buf, 8,  write_val8( 0xff, 2, 1, 0xff ) ));

    // printf("val %s\n",  format_bits(buf, 8,  write_val8( 0x0, 1, 5, 0xff ) ));

    uint8_t v[ 3 ];
    memset(v, 0xff, sizeof(v));
    state_write ( v, sizeof(v), 16, 3, 0x0 );

    printf("v[0] %s\n",  format_bits(buf, 8, v[0 ]));
    printf("v[1] %s\n",  format_bits(buf, 8, v[1 ]));
    printf("v[2] %s\n",  format_bits(buf, 8, v[2 ]));
  }
#endif

  //////////////////////////////




  ////////////////
  spi1_port_cs1_setup();
#if 0
  spi1_special_gpio_setup();
#endif
  // adc interupt...
  spi1_port_interupt_gpio_setup( (void (*) (void *))spi1_interupt, &app);


  ////////////////////

  // TODO . this is very specific. not sure this should be here.
  // should have separate app_t setup.

  app.spi = SPI1 ;
  // app.print_adc_values = true;
  // app.output = false;



#if 0
  // do fpga reset
  spi1_port_cs1_cs2_gpio_setup();

  msleep(1000);

  //while(1)
  {
    // set cs lo, to pull creset lo
    printf("assert cs\n");
    spi1_port_cs1_enable();
    spi1_port_cs2_enable();
    msleep(100);
    printf("deassert cs\n");
    spi1_port_cs1_disable();
    spi1_port_cs2_disable();
    msleep(100);
  }

#endif



#if 1


  // mux spi to ice40
  mux_ice40(app.spi);


  spi_ice40_wait_for_ice40( app.spi );

  // spi_ice40_stress_test_spi( app.spi);

  // spi_ice40_just_read_reg ( app.spi);

  /*
    not quite right.
    we need to clear/reset the 4094 register values s first before turning on OE.
    otherwise 4094 flip/flops can come up in any state,

    should also verify that value was correct. by writing.  and without asserting strobe.
    relays latch could be caught in on state.
  */

  printf("turning on 4094 OE\n");
  // output enable 4094
  spi_ice40_reg_write32( app.spi, REG_4094,  GLB_4094_OE );

#endif


  printf("sizeof X %u\n", sizeof(X));
  printf("sizeof Mode %u\n", sizeof(Mode ));

  init_modes();

  // go to main loop
  loop(&app);

	for (;;);
	return 0;
}




#if 0

static void relay_set( unsigned spi, uint8_t *state_4094, size_t n, unsigned reg_relay,   unsigned relay_state )
{


  // mux spi to 4094. change mcu spi params, and set spi device to 4094
  mux_4094( spi);


  // set two relay bits, according to dir
  state_write ( state_4094, n, reg_relay, 2, relay_state ? 0b01 : 0b10 );

  // output/print - TODO change name state_format_stdout, or something.
  state_format ( state_4094, n);

  // and write device
  spi_4094_reg_write_n(spi, state_4094, n );

  // sleep 10ms
  msleep(10);

  // clear the two relay bits
  state_write ( state_4094, n, reg_relay, 2, 0b00 );   // clear

  // and write device
  spi_4094_reg_write_n(spi, state_4094, n );


  // turn off spi muxing
  mux_no_device(spi);

}
#endif




#if 0
// position in state.
/*
#define REG_K406    2
*/

#define REG_K406    2   // pins 2 and 3.  works well.


// #define REG_K401    (17  ) // third 4094.


// #define REG_K405_4094    2 (16 + 6  ) // third 4094.     L0.

#define REG_K405    (16 + 6  ) // third 4094.     L0.

#define REG_K405_L1    (2 * 8 + 6)
#define REG_K405_L2    (2 * 8 + 7)



#define REG_U404      8   // 2nd 4094.  4 bits.  works. well.

#endif

// we have to OR together an array....
// mode or state.

// so it would be easier to create values in functions.  where we can use an OR function - and record the bit position jj.
// Need to be able to indicate if the relay
// Does C99 have static initializers?
// static void my_or( uint8_t *a, uint8_t *b
// HANG on. it is still tricky. even with functions.... No with functions.
// mode_latch_relay_mask








#if 0
  // 4094 OE should have been configured already,
  uint32_t v = spi_ice40_reg_read32( app->spi, REG_4094);
  UNUSED(v);

  // assert(v == 1);
  // char buf[32+1];
  // printf("4094 state %lu %s\n", v, format_bits(buf, 32, v ));


  //////////////////////////////////
  // EXTR. THE specific led should be injected into APP state on construction.
  // not accessed as a global macro. makes it hard to test.
  //////////////////////////////////
#endif


#if 0
  // spi_ice40_reg_write32(app->spi, REG_LED, count );   // we don't have the set and clear bits...

/*
  uint32_t v = spi_ice40_reg_read32(app->spi, REG_LED);
  char buf[32+1];
  printf("value of led %lu %s\n", v, format_bits(buf, 32, v ));
*/


  // set mcu led state
  led_set( led_state );

#endif


  // toggles relay ok.
  // relay_set( app->spi, app->state_4094, sizeof(app->state_4094), REG_K406  , led_state );


  // toggles relay K405 - works well..
  // relay_set( app->spi, app->state_4094, sizeof(app->state_4094), REG_K405, led_state );
//


/*
  /// toggle ctrl pins of U404
  mux_4094( app->spi);
  // set two relay bits, according to dir.  is 4 is the number of bits.
  state_write ( app->state_4094, sizeof(app->state_4094), REG_U404 , 4, led_state ? 0b1000 : 0b0000 );
  // output/print - TODO change name state_format_stdout, or something.
  state_format ( app->state_4094, sizeof(app->state_4094));
  // and write device
  spi_4094_reg_write_n(app->spi, app->state_4094, sizeof(app->state_4094) );
*/

  /*
      - every state change has to be encoded - as two states.  eg. transition state, and resulting state.
          and has to be handled as a sequency.
          this is a little complicated.

      - But can still be encoded/recorded as a fixed string.

      - Eg. for DCV.
      - - have intermittent relay state.

      - perhaps even three states. eg. for dcv on. the sequencing of turn off b2b fets, then manipulate the relay, then turn on b2b fets.
      - but we may not need this.
      ------------------

      - EXTR.   AND we should encode the MUX registers to use. in the 4094 string. to make it single authoritative source.
                even if use a different way to write these to the fpga.
      -------
      - possible to also use maskable writes.

      - should be a single function that handles the 20ms timing. regardless of which state we want to use
      - we don't need to encode much.

      - we don't necessarily have to record the current state.  unless we
      - transition (  STATE_DCV  )    - and this would have the three transitions .
      - we don't keep individual registers arounds.   only some defines.
      -------------------

      Also the ability to copy.  with memcpy.   and then modify.

      - OR use single byte-array.    and store the two states end-to-end.   eg. if have 5 bytes for state. then use 10 bytes. for 2 transition states.

      - Also can store the mask.

      - latching relays have to use the transition data.   but analog switches need to remain.   Also makes sense to switch relays - before doing analog switches.

      - muxing has the 3x4 muxes = 12 bits. repeated for the signal, and zero.   so these would both be embedded in the string.  perhaps with a number representing sequence.
      --------
      EXTR.
      - eg. don't have separate registers for analog switches etc.  instead entire unit state encoded as single register.

      EXTR.
      - it may be possible to encode everything as single transition - if we manage how to .   eg. a relay set mask.  at least for   .

      - eg. relay clear mask.   this would be fixed/static   that would be or'ed,

      - initialize in a main functino    MODE_DCV_AZ
      - the writing of these bit valuees can be done in a function.
      - and post state mask - to clear relays.
      - transition(   state0, state1, state2 ).

      ------
      - simplest is to have the transition function manage - the latch clear state -  just with a fixed.

  */






/*
  if(led_state)
    spi_4094_reg_write(app->spi , 0b11111111 );
  else
    spi_4094_reg_write(app->spi , 0 );
*/

  // we can factor this fairly easily into a single function -- with the delay.
  // actualyl variable name probably not great. because will chain them.
/*
  if(led_state) {
    // hmmmmm we need a local
    app->u304 |=  U304_K302_L1_CTL;
    spi_4094_reg_write(app->spi, app->u304 );
    msleep(10);
    app->u304 &= ~ U304_K302_L1_CTL ;
    spi_4094_reg_write(app->spi, app->u304 );
  }
  else {

    app->u304 |= U304_K302_L2_CTL ;
    spi_4094_reg_write(app->spi, app->u304 );
    msleep(10);
    app->u304 &= ~ U304_K302_L2_CTL ;
    spi_4094_reg_write(app->spi, app->u304 );

  }
  */

  /*
  // the issue is the verilog 4094 vector - will continue to follow cs2 - when we change from gpio back to spi alternate function.
      eg. and it will go high - as the ordinary parking state of cs.

      options - invert the connection to the output.
      use fpga register to control cs of the 4094. so do write. and then toggle the register.

      eg. an independent approach.

      maybe the whole thing would be easier - if just used ice40 regsisters for CS.
      issue is. the sequence. set to mux ice40 and assert. then set to peripheral peripheral, write. set to muxice 40 and to cs deassert.
      -----
      what if changed. so on mcu side.

      WE don't alternate the cs pins.   instead we just assert an extra pin/ to disambiguate target  using gpio .

      eg. SO ALL WRITES USE ordinary CS1.
      Whether it's too the fpga or the fpga peripheral.  is whether the additional gpio is hi or lo.
      there are no synchronization issues with mcu spi.

      and it avoids constantly having to reconfigure the spi-ports
      --------------

      Doesn't fix. the issue for 4094.  but simplifying things - would make inverting the strobe simpler. / or assign
      --------------

      lets try the invert trick as is. to see if can get work. and test relay.
      -------------
      EXTR - there may be synchronization issues. eg. CS not deasserted. before gpio is swapped.
  */

#if 0
  mux_4094(app->spi);
  spi_4094_reg_write(app->spi , 0b01010101 );

  // msleep(1);    // if we put a sleep here we get a diffferent read value?????

  /*
  // perhaps the action of reading is also setting the value????
      becasue it's an 8 bit register and the bit is in the clear bits....
    -----
    Or it's an overflow....   on read.

    SOLUTION - might be not to remove all the bit setting - but have a parallel register that is direct write.
    -------

    - try writing something that fits in 3 bits. and see if that gets overwritten.
    - maybe change the 16bit width to 24 bit.
  */
  mux_ice40(app->spi);

  val = ice40_reg_read( app->spi, REG_SPI_MUX);
  printf("reg_spi_mux read bits %u %s\n", val, format_bits(buf, 8, val) );

  msleep(1);

  val = ice40_reg_read( app->spi, REG_SPI_MUX);
  printf("reg_spi_mux read bits %u %s\n", val, format_bits(buf, 8, val) );

#endif


