
#pragma once


// application specific.

// this is mixing up 4094, and fpga stuff.

#include <stdint.h>




// This no longer controls the led.  used for tests.  should be renamed/removed.
#define REG_LED         7



// need to rename named _4094_GLB_OE or similar to respect prefix convention

// rename this register... GENERAL REG_GENERAL.   else it's too confusing.
#define REG_4094          9
#define GLB_4094_OE       (1<<0)


#define REG_MODE          12



#define MODE_LO           0     // all bits held lo. but blink led. default.
#define MODE_HI           1     // all bits held hi
#define MODE_PATTERN      2     // put modulation pattern on all bits
#define MODE_DIRECT       3     // support direct writing via direct register
#define MODE_PC           4     // simple precharge switching modulation.
#define MODE_AZ           5     // simple az.
#define MODE_NO_AZ        6     // simple no az.
#define MODE_EM           7     // simple electrometer mode


/*
    .e( modulation_pc_out),                   // 4 works.
    .f( modulation_az_out),                   // 5
    .g( modulation_no_az_out   ),  // 7 works.
*/


/*
// default led blink, and monitor test pattern.
#define MODE_DEFAULT 0b00

// output state put under register control
#define REG_MODE_DIRECT  0b01
*/

#define REG_DIRECT        14
#define REG_DIRECT2       15

// change name REG_APERTURE_N is or similar
#define REG_CLK_SAMPLE_DURATION 16

#define REG_STATUS      17


/*
  may be better to define the values for the specific muxes.

  #define AZMUX_LO      S1
  #define AZMUX_4WLO    S2

  #defien HIMUX_DCV_SOURCE   S3.


  etc.

  to make it easy to reassign pins.
  if have to flip ic, and then re-route traces.
*/
// 1of8 muxes.
#define SOFF        0
#define S1          ((1<<3)|(1-1))
#define S2          ((1<<3)|(2-1))
#define S3          ((1<<3)|(3-1))
#define S4          ((1<<3)|(4-1))
#define S6          ((1<<3)|(6-1))
#define S7          ((1<<3)|(7-1))


// could also be a macro #define S(1) == ...


// S for switch maybe SS ? or W
// dual 1of 4 muxes.
#define WOFF        0
#define W1          ((1<<2)|(1-1))
#define W2          ((1<<2)|(2-1))
#define W3          ((1<<2)|(3-1))
#define W4          ((1<<2)|(4-1))


// should perhaps use top RBOT RTOPTOM
#define ROFF      0
#define RBOT      0b01      // top contact closed.
#define RTOP      0b10      // bottom contact closed.


#define SW_PC_SIGNAL    1
#define SW_PC_BOOT      0


