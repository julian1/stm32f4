

mosfet driver. low side. 

  can use the 3.3V or the output rail 0 - 24V
  use diode to prevent output rail over-powering the the 3.3V line.

  use zener to protect from over voltage of the gate.
  use a current source for the push-pull switch - instead of resistor - so that current is independent.

  issue is the charging of output caps.

  issue is - current source is 1 transistor voltage drop, and push-pull is 1 transistor voltage drop.

  eg. 0.7V * 2 = 1.4.
  3.3 - 1.4 = 2V to play with.
  
  issue - must be boost converter. because mosfet gating relies on good voltages.
  ---

  forget current source.
    instead - use simple zener voltage regulator . eg. zener + resistor - for regulated voltage
    driving a load transistor. then 

    then use that output - for the open collector - and push pull stages.

  ----
  no current source + sinking zener voltage protection.
    - zener just sinks any higher voltage. which should work very well - because current is always limited.

  no current source is for the open-collector push-pull driver only.
  and zener should limit input voltage to the push-pull circuit. 

  should have a diode and a big cap  - so that it gate driver has own supply.
  ----

  extreme - the 3.3V starter voltage - doesn't have to go through all these stages.

  EXTREME
  instead it can be a *separate* diode + resistor to the open collector 
    therefore 0 voltage drop.

  EXTREME
    don't use current source - *because* we use zener + trx for voltage regulation - . 
    very GOOD.

  So the open collector is powered by two sources - stage output .  or direct from 3.3V rail.

  EXTREME
    have open collector - zener voltage reg - current limit resistor all in-line . 
      no zener is in parallel to the open-collector.  both tx and zener.


    zener in parallel - its a basic linear voltage regulator. for open-collector. 
      but then the push-pull stage can have full line voltage.  high-voltage drop - but ok - since only driving capacitor.

    zener must be in parallel - because voltage regulation works by shunting.

    current source + zener shunt - in parallel could work. also. 
      eg. when transistor is off - then the zener just shunts the excess voltage away. 


  jfet constant current source?


    - voltage source - zener + transistor  + resistor for current limit - open-collector current regulation.
    versus
    - current source (jfet) - with zener shunt for over voltage.
        jfet switch speed ok?
        expensive - need higher voltages 
    ----------------------------  

    OK - hang on.   a capacitor - blocks AC - but allows DC.
      So should be able to gate drive through a series capacitor - (must be very small and very high power).
      much the same as gate drive - with a transformer.

  


  ---
    or dickson charge pump.

  ---

  else
    use capacitor charge pump. voltage doubler.   can be independent circuit. driven by gpio.
      - advantage - don't need the current source.
      - or zener gate over voltage protection.
      - or the 3.3V diode reverse flow.
    use gate drive transformer.









static void uart_task(void *args) {
  int gc;
  char kbuf[256], ch;

  (void)args;

  uart_puts("\n\ruart_task() has begun:\n\r");

  // puts_uart(1,"\n\ruart_task() has begun:\n\r");

  for (;;) {
    if ( (gc = getc_uart_nb(1)) != -1 ) {
      puts_uart(1,"\r\n\nENTER INPUT: ");

      ch = (char)gc;
      if ( ch != '\r' && ch != '\n' ) {
        /* Already received first character */
        kbuf[0] = ch;
        putc_uart(1,ch);
        getline_uart(1,kbuf+1,sizeof kbuf-1);
      } else  {
        /* Read the entire line */
        getline_uart(1,kbuf,sizeof kbuf);
      }

      puts_uart(1,"\r\nReceived input '");
      puts_uart(1,kbuf);
      puts_uart(1,"'\n\r\nResuming prints...\n\r");
    }

    /* Receive char to be TX */
    if ( xQueueReceive(uart_txq,&ch,10) == pdPASS )
      putc_uart(1,ch);
    /* Toggle LED to show signs of life */
    // gpio_toggle(GPIOC,GPIO13);
    gpio_toggle(GPIOE,GPIO0); // JA
  }
}

static void demo_task(void *args) {

  (void)args;

  for (;;) {
    uart_puts("Now this is a message..\n\r");
    uart_puts("  sent via FreeRTOS queues.\n\n\r");
    vTaskDelay(pdMS_TO_TICKS(1000));
    uart_puts("Just start typing to enter a line, or..\n\r"
      "hit Enter first, then enter your input.\n\n\r");
    vTaskDelay(pdMS_TO_TICKS(1500));
  }
}


// see the getline function... just needs to have get() implemented...
// should be possible to implement gets using just get.
// and we just need the producer consumer setup. again. I think...
// use same non blocking approach...
// eg.
// most of the time - that we want to receive - will be at a prompt.
// so we really only need to do it once. and we can perhaps directly block...
// so dont need the queue mechanism.
// is this blocking or non blocking...
// int16_t usart_recv(uint32_t usart);
// Actually not sure - we want blocking... for character. So we don't miss a character.

// So just use - an interrupt. and then push value onto the queue...
// if the queue blocks its ok...
// for getline ... we clear queue - then just keep processing the receive queue until we hit a \r\n

/*
333   if ( !uptr )
334     return -1;  // No known uart
335   while ( (rch = get_char(uptr)) == -1 )
336     taskYIELD();
337   return (char)rch;
*/


