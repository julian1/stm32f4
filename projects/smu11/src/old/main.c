





// old uart code.


/*

    when logging from a long func,
    we do not need to wait for control to return to superloop, in order to flush the console output
    we can sync/flush anytime
    or even automatically on '\n' line buffered...

*/

#if 0
  while(true) {

    // check for output to flush...
    int32_t ch = cBufPop(output_buf);
    if(ch == -1)
      return;

    // block, until tx empty
    while(!usart_get_flag(USART1,USART_SR_TXE));

    usart_send(USART1,ch);
  }
#endif


#if 0
  // eg. called from superloop.
  // disadvantage, is superloop timing.

  while(true) {
    // if tx queue not empty, nothing todo, just return
    if(!usart_get_flag(USART1,USART_SR_TXE))
      return;

    // check for output to flush...
    int32_t ch = cBufPop(output_buf);
    if(ch == -1)
      return;

    // non blocking?????
    usart_send(USART1,ch);
  }
#endif


/*
  TODO.
    OK. think this can be done better.

    update() being called at 4kHz. but usart is 115k baud.
    so superloop update() is effectively blocking transmission.

    Use an interupt. whenever the TXE is empty...
    on interupt for txe, pop the ring buffer and push next char.

    We still have to manually call in update, to prime for first char in the circular buffer.

    Or else just enable the tx_interupt. whenever we push a char to the ring buffer?


    void usart_enable_tx_interrupt(uint32_t usart)
    void usart_enable_tx_complete_interrupt(uint32_t usart)
    --------

*/

// maybe change name update_usart_output() ?  no.


#if 0

// this consumes the input queue, which means other code cannot process it...
// don't think we want.

void usart_input_update()
{
  // just transfer any input chars to output so that they appear on output
  // note that this consumes input.
  // more an example, can do line buffering etc, fsm on input also.
  while(true) {

    // read input buf
    int32_t ch = cBufPop(input_buf);
    if(ch == -1)
      return;

    // echo, by transfering to output buf
    // handle line return
    if(ch == '\r') {
      cBufPush(output_buf, '\n');
    }

    cBufPush(output_buf, ch);
  }
}
#endif














#if 0
      usart_printf("=================\n" );
      char buf[1000];
      // ok. nice  we have libc snprintf
      // No. i think this might be linking against the snprintf in miniprintf2. yes.
      // because sprintf which we have not got implemented works.
      // %.5f doesn't work?
      sprintf(buf, "whoot %.5f yyy\n", 123.456);
      usart_printf(buf);

      float val;
      sscanf("999.1234", "%f", &val);

      usart_printf("val is %f\n", val );
      usart_printf("=================\n" );

      // so the issue is that usart_printf writes to a char taking function and doesn't block.
      // but we don't have that.

#endif



#if 0
static void sync(app_t *app)
{
  /*
    No. it's better to make changes correctlu.
  */
  // ranges and values can be changed outside our control...
  // variables out of sync with hardware state.

  if(app->irange == app->iset_range) {
      dac_current_set( fabs(app->iset));

  } else if( app->irange < app->iset_range ) {
      dac_current_set( 11.f );
  } else {

    // bad condition reset. to rh
    // should avoid.  rather than calling range_current_set() which will switch relays
  }
}
#endif


#if 0
typedef struct core_t
{
  // having this as a separate strucutre localizes state extent.


  // uint32_t  spi;

  // the current measurement/regulation range.
  // float     vdac;
  // float     idac;
  vrange_t  vrange;
  irange_t  irange;


  // the set regulation range.
  float     vset;
  vrange_t  vset_range;

  float     iset;
  irange_t  iset_range;

} core_t;
#endif


#if 0
    if(strcmp(tmp, ":halt") == 0) {
      // go to halt state
      // usart_printf("switch off\n");
      // app->state = HALT;

      state_change( app, HALT);
      return;
    }
#endif



#if 0
  if(count % 2 == 0)
  {
    // io_toggle(spi, REG_INA_VFB_SW, INA_VFB_SW1_CTL | INA_VFB_SW2_CTL | INA_VFB_SW3_CTL);
    // io_toggle(spi, REG_INA_DIFF_SW, INA_DIFF_SW1_CTL | INA_DIFF_SW2_CTL);
    // io_toggle(spi, REG_INA_ISENSE_SW,   ISENSE_SW1_CTL | ISENSE_SW2_CTL | ISENSE_SW3_CTL);
    // io_toggle(spi, REG_INA_IFB_SW1_CTL, INA_IFB_SW1_CTL | INA_IFB_SW2_CTL | INA_IFB_SW3_CTL);

    // io_toggle(spi, REG_RELAY_COM, RELAY_COM_X);

    // io_toggle(spi, REG_RELAY_OUT, RELAY_OUT_COM_HC);

    // #define REG_RELAY_OUT         31
    // #define RELAY_OUT_COM_HC    (1<<0)


    // io_toggle(spi, REG_RELAY_VSENSE, RELAY_VSENSE_CTL);



    // io_toggle(spi, REG_RELAY_OUT, RELAY_OUT_COM_HC);
    /////// CAREFUL io_toggle(spi, REG_RELAY_OUT, RELAY_OUT_COM_LC);    // dangerous if on high-current range.

    // io_write(app->spi, REG_IRANGE_YZ_SW, IRANGE_YZ_SW4_CTL);

    // io_toggle(app->spi, REG_IRANGE_YZ_SW, IRANGE_YZ_SW4_CTL); // ok.
    // io_toggle(app->spi, REG_IRANGE_YZ_SW, IRANGE_YZ_SW3_CTL);  ok.
    // io_toggle(app->spi, REG_IRANGE_YZ_SW, IRANGE_YZ_SW2_CTL);    // 1.92V.  and toggles both ina1 and ina2 . bridge on switch or fpga?
    // io_toggle(app->spi, REG_IRANGE_YZ_SW, IRANGE_YZ_SW1_CTL);     // fixed bridge.

  }
#endif

#if 0
  mux_adc03(spi);
  float lp15v = spi_mcp3208_get_data(spi, 0) * 0.92 * 10.;
  float ln15v = spi_mcp3208_get_data(spi, 1) * 0.81 * 10.;
  usart_printf("lp15v %f    ln15v %f\n", lp15v, ln15v);
#endif


 // tests

  // io_write(app->spi, REG_IRANGE_YZ_SW, count);

  // io_write(spi, REG_ISENSE_MUX,  count);    // works
  // io_write(spi, REG_INA_IFB_SW_CTL,  count);    // works
  // io_write(spi, REG_INA_VFB_ATTEN_SW, count);    // works

  // io_write(spi, REG_CLAMP1, count);  // works
  // io_write(spi, REG_CLAMP2, count);  // works
  // io_write(spi, REG_RELAY_COM, count);
  // io_write(spi, REG_IRANGE_X_SW, count);
  // io_write(spi, REG_IRANGE_SENSE, count);


  // io_write(spi, REG_GAIN_IFB, count);
  // io_write(spi, REG_GAIN_VFB, count);

  // test

  // usart_printf("count %d\n", count);
  // io_write(spi, REG_IRANGE_X_SW58, count);

  // io_toggle(spi, REG_RELAY_COM, RELAY_COM_X);
  // io_toggle(spi, REG_RELAY, RELAY_VRANGE);
  // io_toggle(spi, REG_RELAY, RELAY_OUTCOM);
  // io_toggle(spi, REG_RELAY, RELAY_SENSE);



// I think, maybe we only need two ranges.
// or maybe even one.


// source=positive current. sink = negative current.
// can source positive voltage. but might be

// whether the value is inverse should not be a property here... i don't think.
// maybe function should always be min... due to negative fb.


/*
  source a voltage - let current be compliance.
  source a current - let voltage be compliance.

  sink a voltage - let current be compliance.
  sink a current - let voltage be compliance.

  when sourcing, (voltage and current are positive) Q1  or (voltage and current are both negative) Q3.
  when sinking,  (voltage pos and current neg)  Q2      or (voltage neg and current pos). Q4

  ------------
  think the main thing. is function( source or sink) then compliance.
  9V battery. set to 1V sink.   is that a short. or is that just letting a small amount
  Do we have to flip the min/max. around at a cross. quite possibly.
  --------------

  function - is either source or sink. but we may have to flip compliance.

  the compliance function should work in the same direction as the source function sign.
  source and compliance.
    eg.
    source positive voltage.  compliance should be positive current limit.
    source negative voltage.  (eg. reverse on a diode). compliance needs to be negative current limit. (test leakage)
    YES.
    source positive current. compiance is positive voltage limit.
    source negative current. compiance is negative voltage limit.


  sink positive voltage



*/

#if 0
static void clamps_set_source_pve(uint32_t spi)
{
#if 1
  // bahaves correctly relay on or off
  // OK. this can also source or sink... depending on voltage.
  // this sources a positive voltage. eg. the min or +ve v or +ve current.
  io_write(spi, REG_CLAMP1, ~(CLAMP1_VSET_INV | CLAMP1_ISET_INV));
  io_write(spi, REG_CLAMP2, ~CLAMP2_MAX );     // min of current or voltage
                                                // MAX is min. eg. min or 3V,1mA. is 1mA. sourcing.
#endif

#if 0
  // this sources a negative voltage. eg. the min or +ve v or +ve current.
  io_write(spi, REG_CLAMP1, ~(CLAMP1_VSET | CLAMP1_ISET));
  io_write(spi, REG_CLAMP2, ~CLAMP2_MIN );     // min is max due to integration inverter
#endif

// where are our little wires.

#if 0
  // this behaves correctly relay on or off
  // this sinks a positive current.  or sources depending on voltage.
  // not sure. if set to 3V then it will try to sink 3V.
  // OR. set to 1V should be trying to sink everything. which is what it's doing. if set to 3V. it will start sourcing.
  // so i think this might be wrong.

  // not sure.

  io_write(spi, REG_CLAMP1, ~(CLAMP1_VSET_INV | CLAMP1_ISET));
  io_write(spi, REG_CLAMP2, ~CLAMP2_MIN );     // the max of current or voltage. where current is negative
#endif

  /*
    WE SHOULD BE USING WRITE HERE....

    and everything is active lo.  so must use ~ for all arguments.
  */



}
#endif


// ok. ads131. ought to be able to read value - without interrupt.
//

/*
  - current feedback appears working.
  - OK. we want ads to report current as well.
  - connect the output up to a led.
  - different valued sense resistor.
*/

/*
  there is no reason cannot have nested event driven fsm.  this is simple and works well.
  and there is no reason cannot have tasks in 500ms soft timer/ separate from main fsm state.
*/

/*
  we can test the mask write. just on the dg333. without analog power.
*/













/*
  OLD
  io_write(spi, REG_INA_VFB_SW, INA_VFB_SW1_CTL);
  // io_write(spi, REG_INA_DIFF_SW, INA_DIFF_SW1_CTL); // ina154
  // io_write(spi, REG_INA_DIFF_SW, INA_DIFF_SW2_CTL); // ina143
*/



#if 0

static void range_current_set_1A(uint32_t spi)
{
  // 2V on 1A is 200mA, 5V is 0.5A
  // sense gain = 0.1x  ie. 0.1ohm sense resistor
  // ina gain x10.
  // extra amp gain = x10.

  // write() writes all the bits.

  // turn on current relay range X.
  io_write(spi, REG_RELAY_COM, RELAY_COM_X);


  // turn on 1st b2b fets.
  io_write(spi, REG_IRANGE_X_SW, IRANGE_X_SW1 | IRANGE_X_SW2);

  // turn on current sense ina 1
  io_write(spi, REG_IRANGE_SENSE, ~IRANGE_SENSE1);

  // active lo. turn on ifb gain op1, x10
  io_write(spi, REG_GAIN_IFB, ~GAIN_IFB_OP1);


  imultiplier = 0.1f;
}

static void range_current_set_10A(uint32_t spi)
{
  // 0.1ohm
  // 300mV=3A across 0.1R sense.   could use 3.33 (10/3.3) gain after ina to get to 0-10V..
  // 1 / ( 1 +  2 )  = 0.3333333333333333
  // = divider with r1=1 and r2=2. eg. a 2 to 1.
  // eg. make op2 be

  // 10A is the same as 1A, except no 10x gain
  range_current_set_1A(spi);

  // active lo. turn off both ifb gain stages...
  // using 10x gain from ina, on 0.1R only.
  io_write(spi, REG_GAIN_IFB, GAIN_IFB_OP1 | GAIN_IFB_OP2 );

  imultiplier = 1.0f;
}



static void range_current_set_100mA(uint32_t spi)
{
  // 10ohm.
  // 2V on 100mA range should be 20mA.
  // 0.2V across 10ohm. g=10x, 0.2 / 10 = 0.02A = 20mA.
  // adc imultiplier should be 0.1.

  // turn on current relay range X.
  io_write(spi, REG_RELAY_COM, RELAY_COM_X);


  // turn on 2nd b2b fets.
  io_write(spi, REG_IRANGE_X_SW, IRANGE_X_SW3 | IRANGE_X_SW4);


  // active lo, current sense 2
  io_write(spi, REG_IRANGE_SENSE, ~IRANGE_SENSE2);

  // active lo. turn off both current gain ops
  io_write(spi, REG_GAIN_IFB, GAIN_IFB_OP1 | GAIN_IFB_OP2 );

  imultiplier = 0.01f; // sense gain = x10 (10ohm) and x10 gain.
}




static void range_current_set_10mA(uint32_t spi)
{
  // UNUSED(spi);
  /*
      10V = 0.01A * 1k.
      = 100mW.      maybe ok.  with high-watt 1
     1x gain.
  */


  // turn on current relay range X.
  io_write(spi, REG_RELAY_COM, RELAY_COM_X);

  // turn off other fets
  io_write(spi, REG_IRANGE_X_SW, 0);


  // active lo, current sense 3
  io_write(spi, REG_IRANGE_SENSE, ~IRANGE_SENSE3);

  // active lo, turn off both current gain ops
  io_write(spi, REG_GAIN_IFB, GAIN_IFB_OP1 | GAIN_IFB_OP2 );

  imultiplier = 0.001f;
}


#endif


////////////////////////////


#if 0
static void range_voltage_set_100V(uint32_t spi)
{
  // now using ina 143. with 1:10 divide by default

  io_clear(spi, REG_RELAY, RELAY_VRANGE ); // no longer used. must be off.


  // active lo, turn both vfb gain stages off
  io_write(spi, REG_GAIN_VFB, GAIN_VFB_OP1 | GAIN_VFB_OP2 );

  vmultiplier = 10.f;
}


static void range_voltage_set_10V(uint32_t spi)
{
  // now using ina 143. with 1:10 divide by default

  io_clear(spi, REG_RELAY, RELAY_VRANGE ); // no longer used. must be off.

  // active lo. turn on OP1
  io_write(spi, REG_GAIN_VFB, ~GAIN_VFB_OP1 );

  vmultiplier = 1.f;
}


static void range_voltage_set_1V(uint32_t spi)
{
  // now using ina 143. with 1:10 divide by default

  io_clear(spi, REG_RELAY, RELAY_VRANGE ); // no longer used. must be off.

  // active lo.  turn on both OP1 and OP2
  io_write(spi, REG_GAIN_VFB, ~(GAIN_VFB_OP1 | GAIN_VFB_OP2) );

  vmultiplier = 0.1f;
}
#endif

#if 0

  // io_write(spi, REG_INA_VFB_ATTEN_SW, INA_VFB_ATTEN_SW1_CTL);                         // atten = non = 1x
  io_write(spi, REG_INA_VFB_ATTEN_SW, INA_VFB_ATTEN_SW2_CTL | INA_VFB_ATTEN_SW3_CTL);    // atten = 0.1x


  // fix in fpga code. init should be 0b4
  // io_write(spi, REG_INA_VFB_SW, ~INA_VFB_SW1_CTL);    // x1 direct feedback. works.
  io_write(spi, REG_INA_VFB_SW, ~INA_VFB_SW2_CTL);    // x10 . works.
  // io_write(spi, REG_INA_VFB_SW, ~INA_VFB_SW3_CTL);       // x100  works. 0.1V diff gives  8.75V out.

  // 9.1 - 9.0 -> *.1*100 = 0.818.
  // 1.1 - 1.0 -> *.1 x100 = 0.859
  // 0.1 - 1.0             = 0.845

#endif
  // try it without the atten...
  // io_write(spi, REG_INA_IFB_SW1_CTL,  count);    // works

#if 0
  // turn on sense dual op, for high-current range b2b fets
  io_write(spi, REG_INA_ISENSE_SW,  ~ISENSE_SW1_CTL);

  // turn on no resistor divider fb for gain = 1x.
  // io_write(spi, REG_INA_IFB_SW1_CTL, ~INA_IFB_SW1_CTL); // 1x gain.
  // io_write(spi, REG_INA_IFB_SW1_CTL, ~INA_IFB_SW2_CTL);    // 10x gain.
  io_write(spi, REG_INA_IFB_SW1_CTL, ~INA_IFB_SW3_CTL);    // 100x gain.

#endif
  // io_write(spi, REG_ISENSE_MUX,  ~ ISENSE_MUX1_CTL);    // select dedicated 0.1 ohm sense resistor and op. active lo
  // io_write(spi, REG_ISENSE_MUX,  ~ ISENSE_MUX2_CTL);    // select dedicated 10 ohm sense resistor and op. active lo
  // io_write(spi, REG_ISENSE_MUX,  ~ ISENSE_MUX3_CTL);        // select any other range resistor

  // 100x. is stable flickers at 6th digit. nice!!!...



#if 0
  switch(irange)
  {

    case irange_1x:
      io_write(spi, REG_INA_IFB_SW,  ~INA_IFB_SW1_CTL);   //  active low
      imultiplier = 1.f;
      break;

    case irange_10x:
      io_write(spi, REG_INA_IFB_SW,  ~INA_IFB_SW2_CTL);   //  active low
      imultiplier = 10.f;
      break;


    case irange_100x:
      io_write(spi, REG_INA_IFB_SW,  ~INA_IFB_SW3_CTL);   //  active low
      imultiplier = 100.f;
      break;


  }
#endif


#if 0
      // change name GAIN_IFB_OP1 ... GAIN_VFB_OP2   etcc
      // eg. clear ifb regs.
      io_write_mask(spi, REG_GAIN_FB, GAIN_IFB_OP1 | GAIN_IFB_OP2, GAIN_IFB_OP1 | GAIN_IFB_OP2);
      io_write_mask(spi, REG_GAIN_FB, GAIN_VFB_OP1 | GAIN_VFB_OP2,  GAIN_VFB_OP1 | GAIN_VFB_OP2);
      io_write_mask(spi, REG_GAIN_FB, GAIN_IFB_OP1 | GAIN_IFB_OP2, 0 );
      state = HALT;
      return;
#endif

      // range_voltage_set(spi, vrange_10V_2);
      // range_voltage_set(spi, vrange_100V);
      // range_voltage_set(spi, vrange_1V);
      // range_voltage_set(spi, vrange_100mV);


#if 0
        // EXTREME. feedback is always negative. why we just plug vfb and ifb without inverses.
        // its easier to think of everything without polarity.   (the polarity just exists because we tap/ com at 0V).

        // turn on set voltages 2V and 4V outputs. works.

        /*
          OK. can talk to fpga for io, or peripheral, without having to intersperse calls to mux_io() and mux_dac()
            special is asserted for io.
            ---
            but issue is the spi parameters might change for ice40 versus peripheral.
            use a second channel. and it would work.
        */
        //////////////////////////////////
        // set up clamps

        mux_io(spi);
        clamps_set_source_pve(spi);

        // WE DO need the mux() calls. to setup the spi parameters which may differ.
        // sometimes it looks like we don't because they use the *same* clock polarity.

        // voltage
        mux_dac(spi);
        spi_dac_write_register(spi, REG_DAC_VSET, voltage_to_dac( 1.f ) ); // 10V

        mux_io(spi);
        // range_voltage_set_100V(spi);       // ie. 1.2  = 12V, 1.5=15V etc
        range_voltage_set_10V(spi);           // ie 1.2 = 1.2V
        // range_voltage_set_1V(spi);         // ie 1.2 = 0.12V

        // current
        mux_dac(spi);
        spi_dac_write_register(spi, REG_DAC_ISET, voltage_to_dac( 1.f ) );  // 5.f

        mux_io(spi);
        // range_current_set_10A(spi);           // ie 1=1A, 0.5=0.5A, 0.1=0.1V
        // range_current_set_1A(spi);         // ie. 1=0.1A,10=1A
        // range_current_set_100mA(spi);      // 1=10mA, 10=100mA.
        range_current_set_10mA(spi);          // 1=1mA, 10=100mA.
        // range_current_set_none(spi);       // won't work. there's no circuit.

        // turn on output relay
        io_set(spi, REG_RELAY, RELAY_OUTCOM);


        /////////////////
        // adc init has to be done after rails are up...
        // adc init
        int ret = adc_init(spi, REG_ADC);
        if(ret != 0) {
          state = ERROR;
          return;
        }

        usart_printf("analog init ok\n" );
        // maybe change name RAILS_OK, RAILS_UP ANALOG_OK, ANALOG_UP

        // turn on power rails
        // effectively turn on output
#if 1
        ////////////////////
        // power rails
        usart_printf("turn on power rails - lp30v\n" );
        mux_io(spi);
        // io_set(spi, REG_RAILS, RAILS_LP30V );
        io_set(spi, REG_RAILS, RAILS_LP60V );  // actually 15V
        msleep(50);
#endif

        // analog and power... change name?

#endif





#if 0

// whether the value is inverse should not be a property here... i don't think.

static void clamps_set_source_pve(uint32_t spi)
{
/*
  should also have the option to regulate on vfb directly here.
  and errset?
*/
  // TODO needs to take an enum. arguemnt.


  // change name first_quadrant
  // sourcing, charging adc val 1.616501V
  // source +ve current/voltage.
  /*
    TODO change name CLAMP_MUX_MAX, or CLAMP_MAX_MUX
    actually not sure.
  */
#if 0
  // source a positive voltage. on +2mA. off +5V.
  io_write(spi, REG_CLAMP1, CLAMP1_VSET_INV | CLAMP1_ISET_INV);
  io_write(spi, REG_CLAMP2, CLAMP2_MAX);    // TODO this is terrible register naming.
#endif

#if 0
  // this works.
  // source a negative voltage. on its -2mA. off its -5V.
  io_write(spi, REG_CLAMP1, CLAMP1_VSET | CLAMP1_ISET);
  io_write(spi, REG_CLAMP2, CLAMP2_MIN  );    // TODO this is terrible register naming.
#endif

#if 1

  // When using min. it will source or sink depending on voltage.  'not less than'

  // source/ or sink positive.
  // current has to be negative to sink. otherwise it's charging.
  // now sink.
  // sinking current. with battery. on.  1.58V. -2mA.   but off reads -1.58V...

  // lower voltage == sink more current (eg. more of a short). it's confusing.
  // this is correct. limit at the min... means whatever will produce the lowest current. eg. if -2mA is limit use that.
  // io_write(spi, REG_CLAMP1, ~(CLAMP1_VSET_INV | CLAMP1_ISET));
  // io_write(spi, REG_CLAMP2, ~CLAMP2_MIN );     // min is max due to integration inverter


  io_write(spi, REG_CLAMP1, ~(CLAMP1_VSET_INV | CLAMP1_ISET));
  io_write(spi, REG_CLAMP2, ~CLAMP2_MIN );     // min is max due to integration inverter


#endif

  /*
    WE SHOULD BE USING WRITE HERE....

    and everything is active lo.  so must use ~ for all arguments.
  */

  // VERY IMPORTANT. rather than just show the adc values.
  // need to show the set values as well. in the log.

  /////////////////
  // EXTREME. when relay is off. the bad adc code. is because output is at -14V because it's trying to sink. but cannot because relay off. this is correct.
  // eg. we putting -3.2V measured into adc. BAD. rail is -2.5V... eg. ESD diodes are sinking everything. op saturated?
  // eg. 3 / (10 + 3) * 15V = 3.46
  // BUT. can fix. with attenuation? no.
  // if used +-30V output. then it would try to output -30V. and that would be even more...
  // this might be a problem.
  // OR. when sinking. we only set dac values. when relay is on.
  // also. the adc out-of-bounds flag is sticking. because we are not clearing it.
  // voltage is (hopefully) limited by the driving op.
    //
  // the alternative. is that the subtraction should work the other way. so current can be high. but it's limited if voltage goes down to 1V.
  // this could be a mistake in our mux. logic.
  // in which case we do need to invert.
  // OR. we can represent it. but need to flip signs when outputting values.
  //
  // no. think it's ok. eg. 3V.2mA will regulate on 2mA. but 1V,3A  and it won't short any more than 1V. think that is correct.
  //////////////

  /////////////////

  // sink negative. shows negative voltage. and positive current. but limits correctly.
#if 0
  io_write(spi, REG_CLAMP1, CLAMP1_VSET | CLAMP1_ISET_INV);
  io_write(spi, REG_CLAMP2, CLAMP2_MAX );  // retest.

#endif



}

#endif

#if 0
        // source pos voltage, with pos compliance current.  (current can be Q1 positive or Q2 negative to dut battery) depending on DUT and DUT polarity.
        if(false) {
          // ok. this is correct. source 2mA. with compliance of 3V.
          // alternatively can source voltage 1V with compliance of 10mA.
          // and outputs the source voltage 3V compliance when relay is off.
          // limits DUT battery in both polarities.ways
          mux_dac(app->spi);
          // voltage
          spi_dac_write_register(app->spi, DAC_VOUT0_REGISTER, voltage_to_dac( 3.f  ) ); // 3V
          // current
          spi_dac_write_register(app->spi, DAC_VOUT1_REGISTER, voltage_to_dac( 2.0f ) );  // 2mA.
          mux_io(app->spi);
          io_write(app->spi, REG_CLAMP1, ~(CLAMP1_VSET_INV | CLAMP1_ISET_INV));   // positive voltage and current.
          io_write(app->spi, REG_CLAMP2, ~CLAMP2_MAX );     // min of current or voltage
        }

        // Q3  source neg voltage, or neg current.   correct if DUT = resistive load.
        // DUT=battery.  Q4.  this is correct .... positive voltage, and negative current.
        if(false ) {
          // correct sources negative voltage. and negative current compliance or vice versa.
          // relay off shows -3V. correct.
          mux_dac(app->spi);
          // voltage
          spi_dac_write_register(app->spi, DAC_VOUT0_REGISTER, voltage_to_dac( 1.50 /*3.0f*/  ) );     // this has no effect. either below or above dut V. if DUT is battery. ...
          // current
          spi_dac_write_register(app->spi, DAC_VOUT1_REGISTER, voltage_to_dac( 5.0f ) );      // -1mA. resistor or battery


          mux_io(app->spi);
          io_write(app->spi, REG_CLAMP1, ~(CLAMP1_VSET | CLAMP1_ISET));   // positive voltage and current.
          io_write(app->spi, REG_CLAMP2, ~CLAMP2_MIN );     // min of current or voltage
        }
        // source pos voltage. and sink current. for DUT.   Q4.
        // will source neg voltage. sink current. for resistor.   Q3.
        if(true) {

          // OK. this is better for DUT sinking. compliance voltage is positive.  current is negative.
          // -1mA. regardless of DUT polarity.
          // the only way to limit voltage exercusion. is to sink more current. this is correct. why set -100mA. and +21V  compliance
          mux_dac(app->spi);
          // voltage
          spi_dac_write_register(app->spi, DAC_VOUT0_REGISTER, voltage_to_dac( 3.50 /*3.0f*/  ) );     // 1.5 is respected. it will limit voltage. by sinking more current.
          // current
          spi_dac_write_register(app->spi, DAC_VOUT1_REGISTER, voltage_to_dac( 1.0f ) );      // -1mA. resistor or battery

          mux_io(app->spi);
          io_write(app->spi, REG_CLAMP1, ~(CLAMP1_VSET_INV | CLAMP1_ISET));   // positive voltage and current.
          io_write(app->spi, REG_CLAMP2, ~CLAMP2_MAX );     // min of current or voltage

           }
#endif


#if 0
      // RULES.
      // so. if voltage is positive use clamp max.  clamp min/max follows voltage.
      // negative current. can still be source or sink. depending on polarity.

        mux_dac(app->spi);
        // voltage
        spi_dac_write_register(app->spi, DAC_VOUT0_REGISTER, voltage_to_dac( 3.f  ) ); // 3V
        // current
        spi_dac_write_register(app->spi, DAC_VOUT1_REGISTER, voltage_to_dac( 5.0f ) );  // 2mA.
        quadrant_set( app, false, false) ;


       // usart_printf(" -0.123 %f    %f \n",   -0.123,  fabs(-0.123) );
#endif

#if 0
      usart_printf("=================\n" );
      char buf[1000];
      // ok. nice  we have libc snprintf
      // No. i think this might be linking against the snprintf in miniprintf2. yes.
      // because sprintf which we have not got implemented works.
      // %.5f doesn't work?
      sprintf(buf, "whoot %.5f yyy\n", 123.456);
      usart_printf(buf);

      float val;
      sscanf("999.1234", "%f", &val);

      usart_printf("val is %f\n", val );
      usart_printf("=================\n" );

      // so the issue is that usart_printf writes to a char taking function and doesn't block.
      // but we don't have that.

#endif



#if 0
static void sync(app_t *app)
{
  /*
    No. it's better to make changes correctlu.
  */
  // ranges and values can be changed outside our control...
  // variables out of sync with hardware state.

  if(app->irange == app->iset_range) {
      dac_current_set( fabs(app->iset));

  } else if( app->irange < app->iset_range ) {
      dac_current_set( 11.f );
  } else {

    // bad condition reset. to rh
    // should avoid.  rather than calling range_current_set() which will switch relays
  }
}
#endif


#if 0
typedef struct core_t
{
  // having this as a separate strucutre localizes state extent.


  // uint32_t  spi;

  // the current measurement/regulation range.
  // float     vdac;
  // float     idac;
  vrange_t  vrange;
  irange_t  irange;


  // the set regulation range.
  float     vset;
  vrange_t  vset_range;

  float     iset;
  irange_t  iset_range;

} core_t;
#endif


#if 0
    if(strcmp(tmp, ":halt") == 0) {
      // go to halt state
      // usart_printf("switch off\n");
      // app->state = HALT;

      state_change( app, HALT);
      return;
    }
#endif



#if 0
  if(count % 2 == 0)
  {
    // io_toggle(spi, REG_INA_VFB_SW, INA_VFB_SW1_CTL | INA_VFB_SW2_CTL | INA_VFB_SW3_CTL);
    // io_toggle(spi, REG_INA_DIFF_SW, INA_DIFF_SW1_CTL | INA_DIFF_SW2_CTL);
    // io_toggle(spi, REG_INA_ISENSE_SW,   ISENSE_SW1_CTL | ISENSE_SW2_CTL | ISENSE_SW3_CTL);
    // io_toggle(spi, REG_INA_IFB_SW1_CTL, INA_IFB_SW1_CTL | INA_IFB_SW2_CTL | INA_IFB_SW3_CTL);

    // io_toggle(spi, REG_RELAY_COM, RELAY_COM_X);

    // io_toggle(spi, REG_RELAY_OUT, RELAY_OUT_COM_HC);

    // #define REG_RELAY_OUT         31
    // #define RELAY_OUT_COM_HC    (1<<0)


    // io_toggle(spi, REG_RELAY_VSENSE, RELAY_VSENSE_CTL);



    // io_toggle(spi, REG_RELAY_OUT, RELAY_OUT_COM_HC);
    /////// CAREFUL io_toggle(spi, REG_RELAY_OUT, RELAY_OUT_COM_LC);    // dangerous if on high-current range.

    // io_write(app->spi, REG_IRANGE_YZ_SW, IRANGE_YZ_SW4_CTL);

    // io_toggle(app->spi, REG_IRANGE_YZ_SW, IRANGE_YZ_SW4_CTL); // ok.
    // io_toggle(app->spi, REG_IRANGE_YZ_SW, IRANGE_YZ_SW3_CTL);  ok.
    // io_toggle(app->spi, REG_IRANGE_YZ_SW, IRANGE_YZ_SW2_CTL);    // 1.92V.  and toggles both ina1 and ina2 . bridge on switch or fpga?
    // io_toggle(app->spi, REG_IRANGE_YZ_SW, IRANGE_YZ_SW1_CTL);     // fixed bridge.

  }
#endif

#if 0
  mux_adc03(spi);
  float lp15v = spi_mcp3208_get_data(spi, 0) * 0.92 * 10.;
  float ln15v = spi_mcp3208_get_data(spi, 1) * 0.81 * 10.;
  usart_printf("lp15v %f    ln15v %f\n", lp15v, ln15v);
#endif


 // tests

  // io_write(app->spi, REG_IRANGE_YZ_SW, count);

  // io_write(spi, REG_ISENSE_MUX,  count);    // works
  // io_write(spi, REG_INA_IFB_SW_CTL,  count);    // works
  // io_write(spi, REG_INA_VFB_ATTEN_SW, count);    // works

  // io_write(spi, REG_CLAMP1, count);  // works
  // io_write(spi, REG_CLAMP2, count);  // works
  // io_write(spi, REG_RELAY_COM, count);
  // io_write(spi, REG_IRANGE_X_SW, count);
  // io_write(spi, REG_IRANGE_SENSE, count);


  // io_write(spi, REG_GAIN_IFB, count);
  // io_write(spi, REG_GAIN_VFB, count);

  // test

  // usart_printf("count %d\n", count);
  // io_write(spi, REG_IRANGE_X_SW58, count);

  // io_toggle(spi, REG_RELAY_COM, RELAY_COM_X);
  // io_toggle(spi, REG_RELAY, RELAY_VRANGE);
  // io_toggle(spi, REG_RELAY, RELAY_OUTCOM);
  // io_toggle(spi, REG_RELAY, RELAY_SENSE);



// I think, maybe we only need two ranges.
// or maybe even one.


// source=positive current. sink = negative current.
// can source positive voltage. but might be

// whether the value is inverse should not be a property here... i don't think.
// maybe function should always be min... due to negative fb.


/*
  source a voltage - let current be compliance.
  source a current - let voltage be compliance.

  sink a voltage - let current be compliance.
  sink a current - let voltage be compliance.

  when sourcing, (voltage and current are positive) Q1  or (voltage and current are both negative) Q3.
  when sinking,  (voltage pos and current neg)  Q2      or (voltage neg and current pos). Q4

  ------------
  think the main thing. is function( source or sink) then compliance.
  9V battery. set to 1V sink.   is that a short. or is that just letting a small amount
  Do we have to flip the min/max. around at a cross. quite possibly.
  --------------

  function - is either source or sink. but we may have to flip compliance.

  the compliance function should work in the same direction as the source function sign.
  source and compliance.
    eg.
    source positive voltage.  compliance should be positive current limit.
    source negative voltage.  (eg. reverse on a diode). compliance needs to be negative current limit. (test leakage)
    YES.
    source positive current. compiance is positive voltage limit.
    source negative current. compiance is negative voltage limit.


  sink positive voltage



*/

#if 0
static void clamps_set_source_pve(uint32_t spi)
{
#if 1
  // bahaves correctly relay on or off
  // OK. this can also source or sink... depending on voltage.
  // this sources a positive voltage. eg. the min or +ve v or +ve current.
  io_write(spi, REG_CLAMP1, ~(CLAMP1_VSET_INV | CLAMP1_ISET_INV));
  io_write(spi, REG_CLAMP2, ~CLAMP2_MAX );     // min of current or voltage
                                                // MAX is min. eg. min or 3V,1mA. is 1mA. sourcing.
#endif

#if 0
  // this sources a negative voltage. eg. the min or +ve v or +ve current.
  io_write(spi, REG_CLAMP1, ~(CLAMP1_VSET | CLAMP1_ISET));
  io_write(spi, REG_CLAMP2, ~CLAMP2_MIN );     // min is max due to integration inverter
#endif

// where are our little wires.

#if 0
  // this behaves correctly relay on or off
  // this sinks a positive current.  or sources depending on voltage.
  // not sure. if set to 3V then it will try to sink 3V.
  // OR. set to 1V should be trying to sink everything. which is what it's doing. if set to 3V. it will start sourcing.
  // so i think this might be wrong.

  // not sure.

  io_write(spi, REG_CLAMP1, ~(CLAMP1_VSET_INV | CLAMP1_ISET));
  io_write(spi, REG_CLAMP2, ~CLAMP2_MIN );     // the max of current or voltage. where current is negative
#endif

  /*
    WE SHOULD BE USING WRITE HERE....

    and everything is active lo.  so must use ~ for all arguments.
  */



}
#endif


// ok. ads131. ought to be able to read value - without interrupt.
//

/*
  - current feedback appears working.
  - OK. we want ads to report current as well.
  - connect the output up to a led.
  - different valued sense resistor.
*/

/*
  there is no reason cannot have nested event driven fsm.  this is simple and works well.
  and there is no reason cannot have tasks in 500ms soft timer/ separate from main fsm state.
*/

/*
  we can test the mask write. just on the dg333. without analog power.
*/













/*
  OLD
  io_write(spi, REG_INA_VFB_SW, INA_VFB_SW1_CTL);
  // io_write(spi, REG_INA_DIFF_SW, INA_DIFF_SW1_CTL); // ina154
  // io_write(spi, REG_INA_DIFF_SW, INA_DIFF_SW2_CTL); // ina143
*/



#if 0

static void range_current_set_1A(uint32_t spi)
{
  // 2V on 1A is 200mA, 5V is 0.5A
  // sense gain = 0.1x  ie. 0.1ohm sense resistor
  // ina gain x10.
  // extra amp gain = x10.

  // write() writes all the bits.

  // turn on current relay range X.
  io_write(spi, REG_RELAY_COM, RELAY_COM_X);


  // turn on 1st b2b fets.
  io_write(spi, REG_IRANGE_X_SW, IRANGE_X_SW1 | IRANGE_X_SW2);

  // turn on current sense ina 1
  io_write(spi, REG_IRANGE_SENSE, ~IRANGE_SENSE1);

  // active lo. turn on ifb gain op1, x10
  io_write(spi, REG_GAIN_IFB, ~GAIN_IFB_OP1);


  imultiplier = 0.1f;
}

static void range_current_set_10A(uint32_t spi)
{
  // 0.1ohm
  // 300mV=3A across 0.1R sense.   could use 3.33 (10/3.3) gain after ina to get to 0-10V..
  // 1 / ( 1 +  2 )  = 0.3333333333333333
  // = divider with r1=1 and r2=2. eg. a 2 to 1.
  // eg. make op2 be

  // 10A is the same as 1A, except no 10x gain
  range_current_set_1A(spi);

  // active lo. turn off both ifb gain stages...
  // using 10x gain from ina, on 0.1R only.
  io_write(spi, REG_GAIN_IFB, GAIN_IFB_OP1 | GAIN_IFB_OP2 );

  imultiplier = 1.0f;
}



static void range_current_set_100mA(uint32_t spi)
{
  // 10ohm.
  // 2V on 100mA range should be 20mA.
  // 0.2V across 10ohm. g=10x, 0.2 / 10 = 0.02A = 20mA.
  // adc imultiplier should be 0.1.

  // turn on current relay range X.
  io_write(spi, REG_RELAY_COM, RELAY_COM_X);


  // turn on 2nd b2b fets.
  io_write(spi, REG_IRANGE_X_SW, IRANGE_X_SW3 | IRANGE_X_SW4);


  // active lo, current sense 2
  io_write(spi, REG_IRANGE_SENSE, ~IRANGE_SENSE2);

  // active lo. turn off both current gain ops
  io_write(spi, REG_GAIN_IFB, GAIN_IFB_OP1 | GAIN_IFB_OP2 );

  imultiplier = 0.01f; // sense gain = x10 (10ohm) and x10 gain.
}




static void range_current_set_10mA(uint32_t spi)
{
  // UNUSED(spi);
  /*
      10V = 0.01A * 1k.
      = 100mW.      maybe ok.  with high-watt 1
     1x gain.
  */


  // turn on current relay range X.
  io_write(spi, REG_RELAY_COM, RELAY_COM_X);

  // turn off other fets
  io_write(spi, REG_IRANGE_X_SW, 0);


  // active lo, current sense 3
  io_write(spi, REG_IRANGE_SENSE, ~IRANGE_SENSE3);

  // active lo, turn off both current gain ops
  io_write(spi, REG_GAIN_IFB, GAIN_IFB_OP1 | GAIN_IFB_OP2 );

  imultiplier = 0.001f;
}


#endif


////////////////////////////


#if 0
static void range_voltage_set_100V(uint32_t spi)
{
  // now using ina 143. with 1:10 divide by default

  io_clear(spi, REG_RELAY, RELAY_VRANGE ); // no longer used. must be off.


  // active lo, turn both vfb gain stages off
  io_write(spi, REG_GAIN_VFB, GAIN_VFB_OP1 | GAIN_VFB_OP2 );

  vmultiplier = 10.f;
}


static void range_voltage_set_10V(uint32_t spi)
{
  // now using ina 143. with 1:10 divide by default

  io_clear(spi, REG_RELAY, RELAY_VRANGE ); // no longer used. must be off.

  // active lo. turn on OP1
  io_write(spi, REG_GAIN_VFB, ~GAIN_VFB_OP1 );

  vmultiplier = 1.f;
}


static void range_voltage_set_1V(uint32_t spi)
{
  // now using ina 143. with 1:10 divide by default

  io_clear(spi, REG_RELAY, RELAY_VRANGE ); // no longer used. must be off.

  // active lo.  turn on both OP1 and OP2
  io_write(spi, REG_GAIN_VFB, ~(GAIN_VFB_OP1 | GAIN_VFB_OP2) );

  vmultiplier = 0.1f;
}
#endif

#if 0

  // io_write(spi, REG_INA_VFB_ATTEN_SW, INA_VFB_ATTEN_SW1_CTL);                         // atten = non = 1x
  io_write(spi, REG_INA_VFB_ATTEN_SW, INA_VFB_ATTEN_SW2_CTL | INA_VFB_ATTEN_SW3_CTL);    // atten = 0.1x


  // fix in fpga code. init should be 0b4
  // io_write(spi, REG_INA_VFB_SW, ~INA_VFB_SW1_CTL);    // x1 direct feedback. works.
  io_write(spi, REG_INA_VFB_SW, ~INA_VFB_SW2_CTL);    // x10 . works.
  // io_write(spi, REG_INA_VFB_SW, ~INA_VFB_SW3_CTL);       // x100  works. 0.1V diff gives  8.75V out.

  // 9.1 - 9.0 -> *.1*100 = 0.818.
  // 1.1 - 1.0 -> *.1 x100 = 0.859
  // 0.1 - 1.0             = 0.845

#endif
  // try it without the atten...
  // io_write(spi, REG_INA_IFB_SW1_CTL,  count);    // works

#if 0
  // turn on sense dual op, for high-current range b2b fets
  io_write(spi, REG_INA_ISENSE_SW,  ~ISENSE_SW1_CTL);

  // turn on no resistor divider fb for gain = 1x.
  // io_write(spi, REG_INA_IFB_SW1_CTL, ~INA_IFB_SW1_CTL); // 1x gain.
  // io_write(spi, REG_INA_IFB_SW1_CTL, ~INA_IFB_SW2_CTL);    // 10x gain.
  io_write(spi, REG_INA_IFB_SW1_CTL, ~INA_IFB_SW3_CTL);    // 100x gain.

#endif
  // io_write(spi, REG_ISENSE_MUX,  ~ ISENSE_MUX1_CTL);    // select dedicated 0.1 ohm sense resistor and op. active lo
  // io_write(spi, REG_ISENSE_MUX,  ~ ISENSE_MUX2_CTL);    // select dedicated 10 ohm sense resistor and op. active lo
  // io_write(spi, REG_ISENSE_MUX,  ~ ISENSE_MUX3_CTL);        // select any other range resistor

  // 100x. is stable flickers at 6th digit. nice!!!...



#if 0
  switch(irange)
  {

    case irange_1x:
      io_write(spi, REG_INA_IFB_SW,  ~INA_IFB_SW1_CTL);   //  active low
      imultiplier = 1.f;
      break;

    case irange_10x:
      io_write(spi, REG_INA_IFB_SW,  ~INA_IFB_SW2_CTL);   //  active low
      imultiplier = 10.f;
      break;


    case irange_100x:
      io_write(spi, REG_INA_IFB_SW,  ~INA_IFB_SW3_CTL);   //  active low
      imultiplier = 100.f;
      break;


  }
#endif


#if 0
      // change name GAIN_IFB_OP1 ... GAIN_VFB_OP2   etcc
      // eg. clear ifb regs.
      io_write_mask(spi, REG_GAIN_FB, GAIN_IFB_OP1 | GAIN_IFB_OP2, GAIN_IFB_OP1 | GAIN_IFB_OP2);
      io_write_mask(spi, REG_GAIN_FB, GAIN_VFB_OP1 | GAIN_VFB_OP2,  GAIN_VFB_OP1 | GAIN_VFB_OP2);
      io_write_mask(spi, REG_GAIN_FB, GAIN_IFB_OP1 | GAIN_IFB_OP2, 0 );
      state = HALT;
      return;
#endif

      // range_voltage_set(spi, vrange_10V_2);
      // range_voltage_set(spi, vrange_100V);
      // range_voltage_set(spi, vrange_1V);
      // range_voltage_set(spi, vrange_100mV);


#if 0
        // EXTREME. feedback is always negative. why we just plug vfb and ifb without inverses.
        // its easier to think of everything without polarity.   (the polarity just exists because we tap/ com at 0V).

        // turn on set voltages 2V and 4V outputs. works.

        /*
          OK. can talk to fpga for io, or peripheral, without having to intersperse calls to mux_io() and mux_dac()
            special is asserted for io.
            ---
            but issue is the spi parameters might change for ice40 versus peripheral.
            use a second channel. and it would work.
        */
        //////////////////////////////////
        // set up clamps

        mux_io(spi);
        clamps_set_source_pve(spi);

        // WE DO need the mux() calls. to setup the spi parameters which may differ.
        // sometimes it looks like we don't because they use the *same* clock polarity.

        // voltage
        mux_dac(spi);
        spi_dac_write_register(spi, REG_DAC_VSET, voltage_to_dac( 1.f ) ); // 10V

        mux_io(spi);
        // range_voltage_set_100V(spi);       // ie. 1.2  = 12V, 1.5=15V etc
        range_voltage_set_10V(spi);           // ie 1.2 = 1.2V
        // range_voltage_set_1V(spi);         // ie 1.2 = 0.12V

        // current
        mux_dac(spi);
        spi_dac_write_register(spi, REG_DAC_ISET, voltage_to_dac( 1.f ) );  // 5.f

        mux_io(spi);
        // range_current_set_10A(spi);           // ie 1=1A, 0.5=0.5A, 0.1=0.1V
        // range_current_set_1A(spi);         // ie. 1=0.1A,10=1A
        // range_current_set_100mA(spi);      // 1=10mA, 10=100mA.
        range_current_set_10mA(spi);          // 1=1mA, 10=100mA.
        // range_current_set_none(spi);       // won't work. there's no circuit.

        // turn on output relay
        io_set(spi, REG_RELAY, RELAY_OUTCOM);


        /////////////////
        // adc init has to be done after rails are up...
        // adc init
        int ret = adc_init(spi, REG_ADC);
        if(ret != 0) {
          state = ERROR;
          return;
        }

        usart_printf("analog init ok\n" );
        // maybe change name RAILS_OK, RAILS_UP ANALOG_OK, ANALOG_UP

        // turn on power rails
        // effectively turn on output
#if 1
        ////////////////////
        // power rails
        usart_printf("turn on power rails - lp30v\n" );
        mux_io(spi);
        // io_set(spi, REG_RAILS, RAILS_LP30V );
        io_set(spi, REG_RAILS, RAILS_LP60V );  // actually 15V
        msleep(50);
#endif

        // analog and power... change name?

#endif





#if 0

// whether the value is inverse should not be a property here... i don't think.

static void clamps_set_source_pve(uint32_t spi)
{
/*
  should also have the option to regulate on vfb directly here.
  and errset?
*/
  // TODO needs to take an enum. arguemnt.


  // change name first_quadrant
  // sourcing, charging adc val 1.616501V
  // source +ve current/voltage.
  /*
    TODO change name CLAMP_MUX_MAX, or CLAMP_MAX_MUX
    actually not sure.
  */
#if 0
  // source a positive voltage. on +2mA. off +5V.
  io_write(spi, REG_CLAMP1, CLAMP1_VSET_INV | CLAMP1_ISET_INV);
  io_write(spi, REG_CLAMP2, CLAMP2_MAX);    // TODO this is terrible register naming.
#endif

#if 0
  // this works.
  // source a negative voltage. on its -2mA. off its -5V.
  io_write(spi, REG_CLAMP1, CLAMP1_VSET | CLAMP1_ISET);
  io_write(spi, REG_CLAMP2, CLAMP2_MIN  );    // TODO this is terrible register naming.
#endif

#if 1

  // When using min. it will source or sink depending on voltage.  'not less than'

  // source/ or sink positive.
  // current has to be negative to sink. otherwise it's charging.
  // now sink.
  // sinking current. with battery. on.  1.58V. -2mA.   but off reads -1.58V...

  // lower voltage == sink more current (eg. more of a short). it's confusing.
  // this is correct. limit at the min... means whatever will produce the lowest current. eg. if -2mA is limit use that.
  // io_write(spi, REG_CLAMP1, ~(CLAMP1_VSET_INV | CLAMP1_ISET));
  // io_write(spi, REG_CLAMP2, ~CLAMP2_MIN );     // min is max due to integration inverter


  io_write(spi, REG_CLAMP1, ~(CLAMP1_VSET_INV | CLAMP1_ISET));
  io_write(spi, REG_CLAMP2, ~CLAMP2_MIN );     // min is max due to integration inverter


#endif

  /*
    WE SHOULD BE USING WRITE HERE....

    and everything is active lo.  so must use ~ for all arguments.
  */

  // VERY IMPORTANT. rather than just show the adc values.
  // need to show the set values as well. in the log.

  /////////////////
  // EXTREME. when relay is off. the bad adc code. is because output is at -14V because it's trying to sink. but cannot because relay off. this is correct.
  // eg. we putting -3.2V measured into adc. BAD. rail is -2.5V... eg. ESD diodes are sinking everything. op saturated?
  // eg. 3 / (10 + 3) * 15V = 3.46
  // BUT. can fix. with attenuation? no.
  // if used +-30V output. then it would try to output -30V. and that would be even more...
  // this might be a problem.
  // OR. when sinking. we only set dac values. when relay is on.
  // also. the adc out-of-bounds flag is sticking. because we are not clearing it.
  // voltage is (hopefully) limited by the driving op.
    //
  // the alternative. is that the subtraction should work the other way. so current can be high. but it's limited if voltage goes down to 1V.
  // this could be a mistake in our mux. logic.
  // in which case we do need to invert.
  // OR. we can represent it. but need to flip signs when outputting values.
  //
  // no. think it's ok. eg. 3V.2mA will regulate on 2mA. but 1V,3A  and it won't short any more than 1V. think that is correct.
  //////////////

  /////////////////

  // sink negative. shows negative voltage. and positive current. but limits correctly.
#if 0
  io_write(spi, REG_CLAMP1, CLAMP1_VSET | CLAMP1_ISET_INV);
  io_write(spi, REG_CLAMP2, CLAMP2_MAX );  // retest.

#endif



}

#endif

#if 0
        // source pos voltage, with pos compliance current.  (current can be Q1 positive or Q2 negative to dut battery) depending on DUT and DUT polarity.
        if(false) {
          // ok. this is correct. source 2mA. with compliance of 3V.
          // alternatively can source voltage 1V with compliance of 10mA.
          // and outputs the source voltage 3V compliance when relay is off.
          // limits DUT battery in both polarities.ways
          mux_dac(app->spi);
          // voltage
          spi_dac_write_register(app->spi, DAC_VOUT0_REGISTER, voltage_to_dac( 3.f  ) ); // 3V
          // current
          spi_dac_write_register(app->spi, DAC_VOUT1_REGISTER, voltage_to_dac( 2.0f ) );  // 2mA.
          mux_io(app->spi);
          io_write(app->spi, REG_CLAMP1, ~(CLAMP1_VSET_INV | CLAMP1_ISET_INV));   // positive voltage and current.
          io_write(app->spi, REG_CLAMP2, ~CLAMP2_MAX );     // min of current or voltage
        }

        // Q3  source neg voltage, or neg current.   correct if DUT = resistive load.
        // DUT=battery.  Q4.  this is correct .... positive voltage, and negative current.
        if(false ) {
          // correct sources negative voltage. and negative current compliance or vice versa.
          // relay off shows -3V. correct.
          mux_dac(app->spi);
          // voltage
          spi_dac_write_register(app->spi, DAC_VOUT0_REGISTER, voltage_to_dac( 1.50 /*3.0f*/  ) );     // this has no effect. either below or above dut V. if DUT is battery. ...
          // current
          spi_dac_write_register(app->spi, DAC_VOUT1_REGISTER, voltage_to_dac( 5.0f ) );      // -1mA. resistor or battery


          mux_io(app->spi);
          io_write(app->spi, REG_CLAMP1, ~(CLAMP1_VSET | CLAMP1_ISET));   // positive voltage and current.
          io_write(app->spi, REG_CLAMP2, ~CLAMP2_MIN );     // min of current or voltage
        }
        // source pos voltage. and sink current. for DUT.   Q4.
        // will source neg voltage. sink current. for resistor.   Q3.
        if(true) {

          // OK. this is better for DUT sinking. compliance voltage is positive.  current is negative.
          // -1mA. regardless of DUT polarity.
          // the only way to limit voltage exercusion. is to sink more current. this is correct. why set -100mA. and +21V  compliance
          mux_dac(app->spi);
          // voltage
          spi_dac_write_register(app->spi, DAC_VOUT0_REGISTER, voltage_to_dac( 3.50 /*3.0f*/  ) );     // 1.5 is respected. it will limit voltage. by sinking more current.
          // current
          spi_dac_write_register(app->spi, DAC_VOUT1_REGISTER, voltage_to_dac( 1.0f ) );      // -1mA. resistor or battery

          mux_io(app->spi);
          io_write(app->spi, REG_CLAMP1, ~(CLAMP1_VSET_INV | CLAMP1_ISET));   // positive voltage and current.
          io_write(app->spi, REG_CLAMP2, ~CLAMP2_MAX );     // min of current or voltage

           }
#endif


#if 0
      // RULES.
      // so. if voltage is positive use clamp max.  clamp min/max follows voltage.
      // negative current. can still be source or sink. depending on polarity.

        mux_dac(app->spi);
        // voltage
        spi_dac_write_register(app->spi, DAC_VOUT0_REGISTER, voltage_to_dac( 3.f  ) ); // 3V
        // current
        spi_dac_write_register(app->spi, DAC_VOUT1_REGISTER, voltage_to_dac( 5.0f ) );  // 2mA.
        quadrant_set( app, false, false) ;


       // usart_printf(" -0.123 %f    %f \n",   -0.123,  fabs(-0.123) );
#endif

