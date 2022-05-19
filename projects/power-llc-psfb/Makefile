######################################################################
#  Project Makefile
######################################################################




TOP=../../
LIB2SRC=${TOP}/lib2/src
LIB2INC=${TOP}/lib2/include

AGGSRC=${TOP}/agg-svn-r138-agg-2.4/src
AGGINC=${TOP}/agg-svn-r138-agg-2.4/include/


# there's a weird drawing regression when agg_test2.cpp is inlcluded
# src/agg_test2.cpp

BINARY		= main
SRCFILES	= ./src/main.c  \
              ${LIB2SRC}/util.c \
              ${LIB2SRC}/cbuffer.c  ${LIB2SRC}/streams.c  ${LIB2SRC}/usart.c  ${LIB2SRC}/cstring.c \
              ${LIB2SRC}/gpio.c \


# TODO - update to use 512k.
# LDSCRIPT	= ${TOP}/f407.ld
LDSCRIPT	= ${TOP}/f410.ld


TGT_CFLAGS  += -Wno-unused-function   \
              -Werror-implicit-function-declaration \
              -Werror

# ../../libopencm3/lib/usb/usb_private.h

# for usb_private.h, required by usb_f107.c
TGT_CFLAGS	+= -I../../libopencm3/lib/usb/

TGT_CFLAGS	+= -I${LIB2INC}
TGT_CFLAGS	+= -I./include    # FIXME, should be before lib2inc

TGT_CXXFLAGS += -I${LIB2INC} -I./include
TGT_CXXFLAGS += -I${AGGINC}



LDLIBS		+= -lm -lstdc++
# LDLIBS		+= -lm  -lstdc++  #-static-libstdc++

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
