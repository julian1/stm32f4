######################################################################
#  Project Makefile
######################################################################




TOP=../../
LIB2SRC=${TOP}/lib2/src
LIB2INC=${TOP}/lib2/include

mesch12b=${TOP}/mesch12b

# can probably manage mesch as a library. check other Makefile examples here.

BINARY		= main
SRCFILES	= ./src/main.c src/util.c  \
             ${LIB2SRC}/cstring.c  ${LIB2SRC}/cbuffer.c  ${LIB2SRC}/usart2.c \
              ${LIB2SRC}/streams.c  \



# LDSCRIPT	= ${TOP}/f407.ld
# LDSCRIPT	= ${TOP}/f410.ld
LDSCRIPT	= link.ld


TGT_CFLAGS  += -Wno-unused-function   \
              -Werror-implicit-function-declaration \
#              -Werror




TGT_CFLAGS	+= -I./include
TGT_CFLAGS	+= -I${LIB2INC}
TGT_CFLAGS	+= -I${mesch12b}

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
