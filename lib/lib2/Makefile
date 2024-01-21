######################################################################
#  Project Makefile
######################################################################




TOP=../../




lib2.a: cbuffer.o  cdcacm.o  cstring.o  fbuffer.o  flash.o  format.o    stats.o  streams.o  usart.o    util.o  stream-flash.o assert.o
	${AR} rcs lib2.a ${OBJS}


SRCFILES	=   cbuffer.c  cdcacm.c  cstring.c  fbuffer.c  flash.c  format.c    stats.c  streams.c  usart.c    util.c  stream-flash.c  assert.c


OBJS = cbuffer.o  cdcacm.o  cstring.o  fbuffer.o  flash.o  format.o    stats.o  streams.o  usart.o    util.o  stream-flash.o  assert.o


# items not included
#assert.c
# format_float.o
# usb_f107.o




TGT_CFLAGS  += -Wno-unused-function   \
              -Werror-implicit-function-declaration \
              -Wno-packed-bitfield-compat


#              -Werror \



# needed for <assert.h> in priority to pickup the function prototype
TGT_CFLAGS	+= -I${TOP}/include

TGT_CFLAGS	+= -I${TOP}/include/lib2/


#

include ${TOP}/Makefile.incl

######################################################################
#  NOTES:
#
#	1. remove any modules you don't need from SRCFILES
#	2. "make clean" will remove *.o etc., but leaves *.elf, *.bin
#	3. "make clobber" will "clean" and remove *.elf, *.bin etc.
#	4. "make flash" will perform:
#	   st-flash write main.bin 0x8000000
#
######################################################################
