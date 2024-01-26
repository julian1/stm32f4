######################################################################
#  Project Makefile
######################################################################




TOP=../../




lib2.a:  crc32.o
	${AR} rcs crc.a ${OBJS}


SRCFILES	=   crc32.c


OBJS =  crc32.o



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

TGT_CFLAGS	+= -I${TOP}/include/crc32/


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
