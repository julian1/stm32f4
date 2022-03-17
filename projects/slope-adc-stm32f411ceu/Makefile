######################################################################
#  Project Makefile
######################################################################




TOP=../../
LIB2SRC=${TOP}/lib2/src
LIB2INC=${TOP}/lib2/include

mesch12b=${TOP}/mesch12b

# can probably manage mesch as a library. check other Makefile examples here.

BINARY		= main
SRCFILES	= ./src/main.c src/util.c   src/ice40.c src/spi1.c src/regression.c src/mesch.c \
             ./src/app.c ./src/loop1.c ./src/loop2.c     ./src/voltage-source.c ./src/flash.c ./src/config.c \
              ${LIB2SRC}/cbuffer.c  ${LIB2SRC}/usart.c \
              ${LIB2SRC}/fbuffer.c ${LIB2SRC}/stats.c  ${LIB2SRC}/format.c \
              ${LIB2SRC}/streams.c  \
              ${mesch12b}/copy.c  ${mesch12b}/init.c ${mesch12b}/matop.c ${mesch12b}/submat.c  \
              ${mesch12b}/memory.c ${mesch12b}/meminfo.c  \
              ${mesch12b}/machine.c ${mesch12b}/ivecop.c \
              ${mesch12b}/err.c \
              ${mesch12b}/matrixio.c \
              ${mesch12b}/lufactor.c ${mesch12b}/pxop.c ${mesch12b}/solve.c ${mesch12b}/memstat.c ${mesch12b}/norm.c \





# mensch12b from http://homepage.divms.uiowa.edu/~dstewart/meschach/



#              ${LIB2SRC}/spi1.c
# it's messy having spi1 in lib2... and other code locally?
# actually we don't need the core. we can just write registers directly...


LDSCRIPT	= ${TOP}/f411ceu.ld


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
