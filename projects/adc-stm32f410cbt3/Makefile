######################################################################
#  Project Makefile
######################################################################




TOP=../../
LIB2SRC=${TOP}/lib2/src
LIB2INC=${TOP}/lib2/include




BINARY		= main
SRCFILES	= ./src/main.c src/util.c   src/ice40.c \
              ${LIB2SRC}/assert.c ${LIB2SRC}/cbuffer.c   ${LIB2SRC}/usart2.c \
              ${LIB2SRC}/spi1.c

# it's messy having spi1 in lib2... and other code locally?
# actually we don't need the core. we can just write registers directly...


LDSCRIPT	= ${TOP}/f410.ld


TGT_CFLAGS  += -Wno-unused-function   \
              -Werror-implicit-function-declaration \
              -Werror




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
