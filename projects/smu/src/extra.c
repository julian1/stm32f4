


#if 0
      // ... ok.
      // how to return. pass by reference...
      float ar[4];
      spi_adc_do_read(spi, ar, 4);

      // current in amps.
      // til, we format it better
      // %f formatter, doesn't pad with zeros properly...
      // why is the voltage *10?
      // Force=Potential=3V, etc.
      usart1_printf("adc %fV    %fA\n",
        ar[0] / 1.64640 * vmultiplier,
        ar[1] / 1.64640 * imultiplier
      );

      // we want to go to another state here... and bring up POWER_UP...
#endif



#if 0
  // toggle led2
  if(count % 2 == 0) {
    ice40_reg_set(spi, LED_REGISTER, LED2);
    // ice40_reg_set(spi, ADC_REGISTER, ADC_RST);
  }
  else {
    ice40_reg_clear(spi, LED_REGISTER, LED2);
    // ice40_reg_clear(spi, ADC_REGISTER, ADC_RST);
  }
#endif

        // 100mA*15V=1.5W.   think mje15034g is rated at 2W without heatsink.

        // can probably increase current. 12V out. 15-12V=  3V* 2A=6W... no thats even more lot.
        // needs a heatsink.


        /*  none of this works.
            Because, V MON pin output impedance is too low. and needs a buffer. (approximately 2.2kΩ).
        */
        // spi_dac_write_register(spi, DAC_MON_REGISTER, 0 );
        // spi_dac_write_register(spi, DAC_MON_REGISTER, 0 DAC_MON_MDAC0  );
        // spi_dac_write_register(spi, DAC_MON_REGISTER, DAC_MON_AIN );
        // spi_dac_write_register(spi, DAC_MON_REGISTER, DAC_MON_MDAC0  );
        // spi_dac_write_register(spi, DAC_MON_REGISTER, DAC_MON_MDAC1  );



    #if 0
        // source -ve current/voltage.
        // should be write(  ~CLAMP1_VSET | CLAMP1_ISET) etc.
        ice40_reg_clear(spi, CLAMP1_REGISTER, CLAMP1_VSET | CLAMP1_ISET);    // inv for +ve voltage, non invert for negative
        ice40_reg_clear(spi, CLAMP2_REGISTER, CLAMP2_MIN);             // max.   for source +ve voltage, min for source -ve voltage
    #endif

    #if 0
        // sourcing, charging adc val 1.616501V
        // source +ve current/voltage.
        ice40_reg_clear(spi, CLAMP1_REGISTER, CLAMP1_VSET_INV | CLAMP1_ISET_INV);
        ice40_reg_clear(spi, CLAMP2_REGISTER, CLAMP2_MAX);
    #endif


#if 0
        // OK. think its sinking current. 2V over 1k = i=v/r = 2mA. eg. battery discharging.  1.591694V
        // but I don't think V is set correctly
        // except I think V set would have to be negative as well to hold.
        // not sure. set it to 1V and it works. but it goes out of range?
        ice40_reg_clear(spi, CLAMP1_REGISTER, CLAMP1_VSET_INV | CLAMP1_ISET);
        ice40_reg_clear(spi, CLAMP2_REGISTER, CLAMP2_MAX);
#endif


        // ice40_reg_set(spi, RELAY_REGISTER, RELAY_VRANGE ); // turn on vrange register

        // should ok for 12V. perhaps we're running into a limit of voltage drops on output

        // so. should try connecting a higher voltage output. or add 7815 regulation. and turn up bench?

        // 100V range (eg. using 221k resisters
        //  8V output.
        //                  unloaded  8.0861   disconnected.
        //                  loaded    8.0859     OK.
        //   11V output       11.092 loaded  or unloaded. good.
        // 10V
        // 8V range
        //                  unloaded    8.0226  disconnected.
        //                  loaded      8.0225V  great.

        // issue - set and measuring 11V.    but meter reads 10V? 



        // new approach where fb is always active/routed through.
        // 3V sig-gen. vset=4V, Iset=2V.
        // ice40_reg_clear(spi, CLAMP1_REGISTER, CLAMP1_VSET_INV); // active lo. works. 3V -4V = -1V  source a positive voltage.
        ice40_reg_clear(spi, CLAMP1_REGISTER, CLAMP1_VSET);     // active lo. works. 3V +4V = 7V.  source a negative voltage.
        // nothing.                                                              3V * 2= 6V

        // ice40_reg_clear(spi, CLAMP1_REGISTER, CLAMP1_ISET_INV); // active lo. works. 3V -2V = 1V
        ice40_reg_clear(spi, CLAMP1_REGISTER, CLAMP1_ISET);     // active lo. works. 3V +2V = 5V
        // nothing.                                                              3V * 2 = 6V.
        // with no clamps open. we get VERR and IERR = 0. GOOD!!!. makes it easy, to test/use comparison
        // this also means can pass vfb (or vfb_inv) straight through to hold output at 0. (x2 doesn't matter, on integrator).

        // ok. the selection has a 0.7V offset. however. without a resistor.
        // ok. 1M. works. have hard knee.



/*

  GPIOx_BSRR
    This is GPIO Bit Set/Reset Register. When you want to set or reset the
    particular bit or pin, you can use this register.

  GPIOx_ODR
    This is the Output Data Register. [...] this register is used to set the value
    to the GPIO pin.
  https://embetronicx.com/tutorials/microcontrollers/stm32/stm32-gpio-tutorial/#GPIOx_BSRR


void gpio_set(uint32_t gpioport, uint16_t gpios)
 {
         GPIO_BSRR(gpioport) = gpios;
 }

void  gpio_clear(uint32_t gpioport, uint16_t gpios)
 {
         GPIO_BSRR(gpioport) = (gpios << 16);
 }

  So BSRR. doesn't need bit masks, or to read values.  to set individual values.
    instead it uses low bytes to set.
    and high bytes to clear

  #define GPIO_BSRR  (     port )     MMIO32((port) + 0x18)
  #define MMIO32  (     addr )     (*(volatile uint32_t *)(addr))

toggle does a read eg. stores in port ..

void gpio_toggle(uint32_t gpioport, uint16_t gpios)
 {
         uint32_t port = GPIO_ODR(gpioport);
         GPIO_BSRR(gpioport) = ((port & gpios) << 16) | (~port & gpios);
 }


  so for setting it's just    current | val
     and clear                ~(~current | val)

   0b01111
  ~0b10010
val     1
  |  10010
  ~  01101

  but then how are they combined?
    yes. apply set(), take output, then apply clear()
  -----------------------

  we want to get analog power - to test dac. i think. in priority.


void gpio_port_write(uint32_t gpioport, uint16_t data)
 {
         GPIO_ODR(gpioport) = data;
 }
  write uses a different register.
*/
