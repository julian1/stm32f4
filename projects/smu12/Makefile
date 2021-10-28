######################################################################
#  Project Makefile
######################################################################




TOP=../../
LIB2SRC=${TOP}/lib2/src
LIB2INC=${TOP}/lib2/include


BINARY		= main
#SRCFILES	= mcp3208.c  util.c \
SRCFILES	= flash.c  util.c \

SRCFILES	= src/main.c  src/mcp3208.c src/w25.c src/dac8734.c src/ads131a04.c src/ice40.c src/spi1.c src/core.c src/util.c \
              src/str.c \
              ${LIB2SRC}/assert.c ${LIB2SRC}/cbuffer.c  ${LIB2SRC}/fbuffer.c ${LIB2SRC}/usart2.c  ${LIB2SRC}/flash.c  ${LIB2SRC}/stats.c


# ${LIB2SRC}/format_float.c
# ${LIB2SRC}/format_bits.c



# ${LIB2SRC}/miniprintf2.c

#SRCFILES	= flash.c   \

# f407
#LDSCRIPT	= ${TOP}/cjmcu-407.ld
LDSCRIPT	= ${TOP}/f410.ld


TGT_CFLAGS  += -Wno-unused-function   \
              -Werror-implicit-function-declaration \
              -Werror


TGT_CFLAGS	+= -I${LIB2INC}
TGT_CFLAGS	+= -I./include    # FIXME, should be before lib2inc

# pow()
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
