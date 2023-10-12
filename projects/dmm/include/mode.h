
#pragma once

/*

// modes of operation.


  - I think we just about deserve a separate header.
  - this could have references to the vectors to

*/


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

// #define MODE_ARR_N    3   // mode array in bytes.


// REMEMBER this is 4094.   not fpga state.

/*
  use lower-case identifiers. why are we using capitalization / macros??

  change name _4094_state. _4094_state_t   or similar.


*/

typedef struct X
{
  // U406 4094.
  uint8_t U408_SW_CTL : 1;      // perhaps change to lower case....   eg. u408_sw
  uint8_t U406_UNUSED : 1;
  uint8_t K406_CTL    : 2;        // Be better to encode as 2 bits.   can then assign 0b01 or 0b10 etc.
  uint8_t U406_UNUSED_2 : 4;

  // U401 4094
  uint8_t U401_UNUSED : 8;    // controls U404.


  // U403
  uint8_t U403_UNUSED : 6;
  uint8_t K405_CTL    : 2;


  // U506
  uint8_t U506        : 3;        // adg1209.  gain mux. (MUST BE BROUGHT UP with enable-pin enabled otherwise power supplies current limit, risk of part damage).
  uint8_t U500_UNUSED : 5;        // change name _500_unused --- or similar.  think we should prefix U505_MUX. or

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
/*
    Actually can even put a mode.
    here .
    and then switch what gets written.
*/

  X     first;
  X     second;

} Mode;


//////////

/*
      SPI_INTERUPT_OUT,
      MEAS_COMPLETE_CTL,
      CMPR_LATCH_CTL,
      adcmux,                  // 19 bits.

      monitor,                // 15.    bit 14 from 0.. + 8= j    bit 10,    1024.
      LED0,                   // bit 13.  8192.
      SIG_PC_SW_CTL,
      himux2,              // remove the himux2  12.
      himux,
      azmux

  so put in direct mode. then try to bllink the led.
  so try to blink
  assert( sizeof(F) == 4);
*/

//  __attribute__((__packed__))

typedef struct  __attribute__((__packed__))
F
{
  /* this is direct mode state.  TODO rename
      in adc mode,   we would have 2 or four sets of mux registers for the values to switch.
      and the other lines would be given to the fpga to run
      ----
      and this state would be written in the main mode state.
      -------
  */

  uint8_t azmux   : 4;
  uint8_t himux   : 4;
  uint8_t himux2  : 4;     // 12
  uint8_t sig_pc_sw_ctl : 1;
  uint8_t led0    : 1;       // 14   2 bytes.

  uint8_t monitor : 8;    // 22  // this bit vector overflows - so gets aligned on a new byte boundary. which is not what we want...

  uint8_t adcmux : 4;     // 26
  uint8_t cmpr_latch_ctl : 1;
  uint8_t meas_complete_ctl : 1;
  uint8_t spi_interupt_ctl : 1;     // 29bits

  uint8_t dummy   : 3;
} F;    // change name REG_DIRECT_MODE ????


