######################################################################
#  Top Level: STM32F103C8T6 Projects
######################################################################


.PHONY = libopencm3 clobber_libopencm3 clean_libopencm3 #libwwg

all:	libopencm3 #libwwg
	$(MAKE) -$(MAKEFLAGS) -C ./projects

clean:	clean_libopencm3
	$(MAKE) -$(MAKEFLAGS) -C ./projects clean
#	$(MAKE) -$(MAKEFLAGS) -C ./rtos/libwwg clean

clobber: clobber_libopencm3
	$(MAKE) -$(MAKEFLAGS) -C ./projects clobber
#	$(MAKE) -$(MAKEFLAGS) -C ./rtos/libwwg clobber

clean_libopencm3: clobber_libopencm3

clobber_libopencm3:
	rm -f libopencm3/lib/libopencm3_stm32f4.a
	-$(MAKE) -$(MAKEFLAGS) -C ./libopencm3 clean

libopencm3: libopencm3/lib/libopencm3_stm32f4.a

libopencm3/lib/libopencm3_stm32f4.a:
	$(MAKE) -C libopencm3 TARGETS=stm32/f4

#libwwg:
#				$(MAKE) -C rtos/libwwg


# Uncomment if necessary:
# MAKE	= gmake

# End
