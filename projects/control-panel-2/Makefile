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

#

BINARY		= main
SRCFILES	=   src/main.cpp src/util.c src/fsmc.c src/ssd1963.c src/xpt2046.c src/rotary.c src/ui_events.c src/agg.cpp src/curses.cpp src/menu.cpp src/value.cpp \
              src/agg_test2.cpp src/agg_test3.cpp src/agg_test4.cpp src/agg_test5.cpp  src/agg_test6.cpp src/agg_test7.cpp src/agg_test8.cpp  \
              src/arial-outline.cpp   src/arial-span-18.cpp  src/arial-span-72.cpp   \
              ${LIB2SRC}/cbuffer.c  ${LIB2SRC}/streams.c  ${LIB2SRC}/usart.c  \
              ${LIB2SRC}/cstring.c ${LIB2SRC}/format.c    ${LIB2SRC}/gpio.c \
              ${LIB2SRC}/cdcacm.c ${LIB2SRC}/usb_f107.c \
              ${AGGSRC}/agg_trans_affine.cpp  ${AGGSRC}/agg_curves.cpp ${AGGSRC}/agg_rounded_rect.cpp  ${AGGSRC}/agg_arc.cpp



# TODO - update to use 512k.
LDSCRIPT	= ${TOP}/f407.ld


TGT_CFLAGS  += -Wno-unused-function   \
              -Werror-implicit-function-declaration \
              -Werror

# ../../libopencm3/lib/usb/usb_private.h

# for usb_private.h, required by usb_f107.c
# TODO review
TGT_CFLAGS	+= -I../../libopencm3/lib/usb/
TGT_CFLAGS	+= -I${LIB2INC}  -I./include    

#TGT_CXXFLAGS +=  -std=c++17  -I${LIB2INC} -I./include
TGT_CXXFLAGS +=   -I${LIB2INC} -I./include
TGT_CXXFLAGS += -I${AGGINC}

# stop at first error
TGT_CXXFLAGS += -Wfatal-errors


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
