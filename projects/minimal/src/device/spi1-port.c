/*

  dec 2022.
    at 100MHz (instead of 50MHz) there are flip issues, using stress-test. with iso7762


*/

#include <libopencm3/stm32/gpio.h>


#include <stdio.h>
#include <assert.h>

#include <device/spi1-port.h>


/*
	// hold cs lines lo - to put fpga in reset, avoid isolator/fpga contention, because fpga wants to become spi master and drive spi lines.
  // probably ok, if just SS held lo.
  // there's a delay of about 1ms. though before ice40 samples inputs.

  // assert(0);

  // these are no longer in scope.
  // but we can enforce order  in main.c
	// spi_port_cs2_enable( SPI1);
  // spi_port_cs1_enable( SPI1);

  oct 2024.

  If/once fpga enters contention/latchup condition. we cannot get out of it. without re-power.
  even if set all pins lo / including RST . eg. if analog side, loses power and is repowered.
  There is no general way to avoid this. without having a power sense/monitor to report back to digital side.

  EXTR. this is because SS is sampled at POR, to determine if fpga should orient/act as master or peripheral, see DS, p11. configuration chart.
  But all this makes spi1 not a generalizable service that the fpga hangs off.
  the solution is move the control of/and start deefault value of SS off the cs1/cs2 assert condition.

  Actually RST is a gate in the sequence before SS is sampled to determine master/slave orientation.
  So just holding in RST. is also sufficient.

  We get correct behavior at the moment by using 'F' version of isolator that emits lo. at start if mcu/digital side unpowered.
  and presumably when mcu is in reset, emitting high-z on gpio.

  ---
  we could move RST, SS to a hc175 type latch. that can start with outputs lo. using asynch CLR.

  ---
  eg.

	// locks up.
	spi_port_cs2_disable(SPI1 );      // both gpio and cs hi. worst case. both fight.   both cs hi.  CC 2.9V / 29mA.  eg.
  spi_port_cs1_disable(SPI1 );

  // locks up
	spi_port_cs2_enable(SPI1 );			  // gpio pin lo.  but cs is hi. and fights.   but less worse CC 3.28V/29mA .
  spi_port_cs1_disable(SPI1 );

	// OK. works.
	spi_port_cs2_disable(SPI1 );			// gpio hi, and cs lo.   ok. doesn't fight. because gpio is high-z before bitstream configuration.
  spi_port_cs1_enable(SPI1 );       // this is the same state, as when, we are trying to configure the bitstream.

  summary if cs hi, we get contention/fight - regardless of state of cs2/gpio pin . probably due on the cs pin itself.  or perhaps due to clk/mosi pin.
  if cs is lo.  then ok.   regardless of gpio.

  eg. because ss is sampled at startup, to determine spi orientation.
  so SS should be held lo. at startup.  when configured as slave.
  */





void spi1_port_setup(void)
{
  printf("spi1 port setup\n");

  // clk PA5, miso. PA6
  gpio_mode_setup(GPIOA, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO5  | GPIO6);
  gpio_set_af(GPIOA, GPIO_AF5, GPIO5  | GPIO6);       // af 5
  gpio_set_output_options(GPIOA, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, GPIO5 );   // 100MHZ ??


  // mosi. on PB5.
  gpio_mode_setup(GPIOB, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO5 );
  gpio_set_af(GPIOB, GPIO_AF5, GPIO5 );       // af 5
  gpio_set_output_options(GPIOB, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, GPIO5 );   // 100MHZ ??



}


