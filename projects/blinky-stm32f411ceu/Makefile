######################################################################
#  Project Makefile
######################################################################




TOP=../../
LIB2SRC=${TOP}/lib2/src
LIB2INC=${TOP}/lib2/include




BINARY		= main
SRCFILES	= ./src/main.c src/util.c  \
              ${LIB2SRC}/cbuffer.c   ${LIB2SRC}/usart.c  \
              ${LIB2SRC}/cdcacm.c ${LIB2SRC}/usb_f107.c



# TODO - update to use 512k. 
LDSCRIPT	= ${TOP}/f411ceu.ld


TGT_CFLAGS  += -Wno-unused-function   \
              -Werror-implicit-function-declaration \
              -Werror

# ../../libopencm3/lib/usb/usb_private.h

# for usb_private.h, required by usb_f107.c
TGT_CFLAGS	+= -I../../libopencm3/lib/usb/

TGT_CFLAGS	+= -I${LIB2INC}
TGT_CFLAGS	+= -I./include    # FIXME, should be before lib2inc

LDLIBS		+= -lm


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
