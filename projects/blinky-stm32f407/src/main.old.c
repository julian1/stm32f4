
#if 0
  while(1) {

    // also see read_ddb. a lot of serial stuff. 

    reg = 0x0A;   // == 1000

    LCD_SetAddr(reg );
    x = LCD_ReadData();   // EXTR. OK. interleaving the write, and read data with printf... slows things down enough to get the correct response.
                          // unless one changes the setup time.
    usart_printf("reg %u (%02x)  r\n", reg,  reg);
    usart_printf("%03u  %s\n", x, format_bits(buf, 16, x));


    msleep(1000);
  }
#endif




 /* 
  // LCD_TouchReg( 0x01 );  // soft reset
  LCD_TouchReg( 0x01 );  // soft reset
  msleep(1000);
*/

  // ok. its an interleaving issue. the value read. is getting the value from the previous register read ...
  // because 16/8 bit issue?

  // is our fsmc configured as 16 bit or 8 bit?

  ///////////  EXTR. 0A == 1000   correct.  read works.   but only on the second try. 
  //                 0D == 0011   correct. read works but only on second time.
  // we may need to intersperse read and write. or pad it... 
  // interleaving?
  // OR. are we not handling correctly - when expect a parameter
  // OR. ccould be fsmc setup.

  // LCD_ReadReg( 0x00 );  // nop

  /*
    EXTR. doing things in separate calls. gets a different result. due to timing differences?
    or compiler not optimizing the accesses?

  */

  /*
    there are timing issues.
    maybe related to initial setup. maybe related to the tft 1963 pll.
    see.  resets the timing 
      https://github.com/andysworkshop/stm32plus/blob/master/examples/ssd1963/ssd1963.cpp
    and here, where change pll.
      https://community.st.com/s/question/0D50X00009XkgSPSAZ/stm32f4-discovery-ssd1963-fsmc

    stm32f4 code for fsmc here.  EXTR.
      https://github.com/stm32f4/library/blob/master/SSD1963/GLCD.c

      data setup is slowed. divider == 12 initially.
      FSMC_NORSRAMTimingInitStructureRead.FSMC_DataSetupTime = 5 * divider;

      then later does a full speed reconfig with divider == 1.
  */

/*
  for(reg = 0x0A; reg < 72 ; reg++) {
    x = LCD_ReadReg( reg );
    // usart_printf("reg %u (%x)  read %u   %s\n", reg, reg, x, format_bits(buf, 16, x));
    // EXTR... there's a memory issue is this string gets too long?
    usart_printf("reg %u (%02x)  r %u  %s\n", reg,  reg, x, format_bits(buf, 16, x));
    msleep(10);
  }
*/

/*
  usart_printf("----\n");
  msleep(20);
  for(reg = 0x0A; reg < 72 ; reg++) {
    x = LCD_ReadReg( reg );
    // usart_printf("reg %u (%x)  read %u   %s\n", reg, reg, x, format_bits(buf, 16, x));
    // EXTR... there's a memory issue is this string gets too long?
    usart_printf("reg %u (%02x)  r %u  %s\n", reg,  reg, x, format_bits(buf, 16, x));
    msleep(10);
  }
*/


  /*
    OK. the bottom two bits of 0x0D  get_display_mode should be hi. according to doc.
    while our code. bottom bits of 0x0E are high.

    because the sequencing is out of alignment???
    NOP (0) and soft reset (1) take no parameters
    --------------

    bits are mixed up.  reg 0x0a should have D3. on at POR.
    instead             reg 0x0b has this bit.

                        reg 0x0d should have D0 and D1 at POR
    instead             reg 0x0e has these set.

    possible high byte of register - should be configured differently?
    manual states on 8bits used.

    TRY
    - should try a write - and see which values get written.
    - should read all registers twice. make sure nothing changes.

    - make sure we are not sending a parameter. which could throw things off.
    - need to probe everything.

    - bitbang as gpio. eg. just try to read a copule of registers.

    - does it need an entry in the linker script?

    - we need to understand how arguments are presented.
        is it a series of writes.

    - is a write with no arguments the same as a read.

    - setup on loop.  just on the d register...
        nothing else - that could be interpreted differently as a write.

    - review what other fsmc for ssd1936 uses.

first time.
  reg 10 (a)  r 0  0000000000000000
  reg 11 (b)  r 8  0000000000001000
  reg 12 (c)  r 0  0000000000000000
  reg 13 (d)  r 0  0000000000000000
  reg 14 (e)  r 3  0000000000000011
  reg 15 (f)  r 0  0000000000000000
  reg 16 (10)  r 0  0000000000000000


second time. (think that just accessing a address - may be a command,  )...
should try a loop for the same address...
  reg 10 (a)  r 0  0000000000000000
  reg 11 (b)  r 92  0000000001011100
  reg 12 (c)  r 0  0000000000000000
  reg 13 (d)  r 0  0000000000000000
  reg 14 (e)  r 35  0000000000100011
  reg 15 (f)  r 0  0000000000000000


  so reading is changing values somehow...
*/

/*
  /////////////
  usart_printf("reset cmd\n");
  LCD_WriteReg( 0x01 , 0x0 ); // soft reset. takes no parameter. but we are giving it one.
  msleep(10); // sleep is required.
*/



  // conssequtive reads and the value changes...
#if 0
  x = LCD_ReadReg( reg );
  usart_printf("read %u   %s\n", x, format_bits(buf, 8, x));

  x = LCD_ReadReg( reg );
  usart_printf("read %u   %s\n", x, format_bits(buf, 8, x));


  // any kind of write value... sets a bit...
  usart_printf("write ram \n");
  LCD_WriteReg( reg , 0x00 );

  x = LCD_ReadReg( reg );
  usart_printf("read %u   %s\n", x, format_bits(buf, 8, x));

  x = LCD_ReadReg( reg );
  usart_printf("read %u   %s\n", x, format_bits(buf, 8, x));

  /////////////
  usart_printf("reset cmd\n");
  LCD_WriteReg( 0x01 , 0x0 ); // soft reset. takes no parameter. but we are giving it one.
  msleep(10); // sleep is required.

  x = LCD_ReadReg( reg );
  usart_printf("read %u   %s\n", x, format_bits(buf, 8, x));



  // ok writing a 0, and we end up with a value...
  // no it appears the second time we read... it changes the value....

  usart_printf("write 0 \n");
  LCD_WriteReg( reg , 0x00 );
  x = LCD_ReadReg( reg );

  x = LCD_ReadReg( reg );
  usart_printf("read %u   %s\n", x, format_bits(buf, 8, x));

  x = LCD_ReadReg( reg );
  usart_printf("read %u   %s\n", x, format_bits(buf, 8, x));


#endif


  // so a write



/*
    - should try a soft reset command. and see if it clears it?

    - also need to probe the scope - to make sure all bits/ are soldered ok.
*/
