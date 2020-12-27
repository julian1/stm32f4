

#include "FreeRTOS.h"
// #include "task.h"
// #include "queue.h"


#include <libopencm3/stm32/rcc.h>
// #include <libopencm3/stm32/gpio.h>


#include <libopencm3/cm3/nvic.h>
#include <libopencm3/stm32/exti.h>



#include "pvd.h"

#include "usart.h"  // to try and capture interrupt



void pvd_isr (void)
{
  // TODO - change this.
  // WE CANNOT PRINT FROM ISR...!!!!!!
  // most we can do is enqueue an output character. something.

  usart_enqueue_tx_test('x');
  usart_enqueue_tx_test('\n');
}



void power_voltage_detect_setup(void)
{

  // https://community.st.com/s/question/0D50X00009XkbAJ/how-to-get-pvd-programmable-voltage-detector-interrupt-working-

  // https://github.com/MaJerle/stm32fxxx-hal-libraries/tree/master/26-STM32Fxxx_PVD/User

  // see beginnging stm32... p191. for RTC / exti17. setup should be the same.

  // probably working, example
  // https://github.com/MaJerle/stm32fxxx-hal-libraries/blob/master/26-STM32Fxxx_PVD/User/main.c

  // TM_PVD_Enable()
  // https://github.com/MaJerle/stm32fxxx-hal-libraries/blob/89bc743fa9b9766ff5e86add8e108c9f784317fa/00-STM32_LIBRARIES/tm_stm32_pvd.h
  // https://github.com/MaJerle/stm32fxxx-hal-libraries/blob/3e68dfadc34e24ed89612b9fdf9cad7b60a0e69c/00-STM32_LIBRARIES/tm_stm32_pvd.c

  // guy does it,
  // https://community.st.com/s/question/0D50X00009XkhnSSAR/monitoring-vdd-using-the-pvd-feature

    // NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = BKPDEV_PVD_PRIORITY;

  // USES RISING...!!!!. eg. because it's different than the register?
  // EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising; // trigger power rising edge (voltage drop)

  // https://community.st.com/s/question/0D50X00009XkYe3SAF/pvd-through-exti-line-detection-not-triggered


  // in main.c
	// rcc_periph_clock_enable(RCC_PWR);


  // PWR_CR_PLS_2V9  is highest voltage
  pwr_enable_power_voltage_detect(PWR_CR_PLS_2V9);



  exti_set_trigger(EXTI16, EXTI_TRIGGER_RISING);      // other code uses rising...
  // exti_set_trigger(EXTI16, EXTI_TRIGGER_FALLING);   // think we want falling.
                                                    // pwr_voltage_high() eg. goes from high to lo.
  exti_enable_request(EXTI16);

  // defined 1 for line 16.
  // #define NVIC_PVD_IRQ 1
  nvic_enable_irq( NVIC_PVD_IRQ );



  /*
    1) see if can get the pwr_voltage_high() to change. first. by powering with a bench supply.
        could log to gpio.
        eg. ignoring the interrupt.
        ok. works. so we know the register value changes.
    2) then try setting gpio pin - which we can see with a scope.
  */
}


void report_pvd_test_task(void *args __attribute((unused)))
{
  // just monitor pwr_voltage_high() to catch the register flip when power disconnected...
  while(1) {
    uart_printf("%c\n", pwr_voltage_high() ? 't' : 'f' );
  }
}


////////////////////////
  /*

  old comments

    its far more important - to turn rails off. if digital power goes. in order to protect the dac.
    it will turn off automatically - with the optos, but would be good to pre-empt as well.

    https://stm32f4-discovery.net/2015/08/hal-library-22-bor-for-stm32fxxx/

    BO brownout
    BOR == brownout reset
    BOD == brownout detect.

    we don't want BOR - we want BO interupt.

   ///////

    Programmable voltage detector (PVD)

    ../../libopencm3/include/libopencm3/stm32/f4/nvic.h:void pvd_isr(void);

    NVIC_PVD_IRQ
    void pvd_isr(void);


    void pwr_enable_power_voltage_detect(uint32_t pvd_level);
    void pwr_disable_power_voltage_detect(void)

    p209 beginning stm32.

    http://libopencm3.org/docs/latest/stm32f4/html/group__pwr__file.html

      Enable Power Voltage Detector.

      This provides voltage level threshold detection. The result of detection is
      provided in the power voltage detector output flag (see pwr_voltage_high) or by
      setting the EXTI16 interrupt (see datasheet for configuration details).

    ext16 would correspond to the pvd_isr()

    void pwr_voltage_high()

        Get Voltage Detector Output.

        The voltage detector threshold must be set when the power voltage detector is
        enabled, see pwr_enable_power_voltage_detect.

        Returns boolean: TRUE if the power voltage is above the preset voltage
        threshold.


    levels...
    http://libopencm3.org/docs/latest/stm32f4/html/group__pwr__pls.html

    https://community.st.com/s/question/0D50X00009XkbAJ/how-to-get-pvd-programmable-voltage-detector-interrupt-working-

  */



