######################################################################
#  Project Makefile
######################################################################




TOP=../../
LIB2SRC=${TOP}/lib2/src
LIB2INC=${TOP}/lib2/include

mesch12b=${TOP}/mesch12b


BINARY		= main
SRCFILES	= ./src/main.c src/util.c   src/ice40.c src/spi1.c src/regression.c \
              ${LIB2SRC}/assert.c ${LIB2SRC}/cbuffer.c  ${LIB2SRC}/usart2.c \
              ${LIB2SRC}/fbuffer.c ${LIB2SRC}/stats.c \
              ${mesch12b}/copy.c  ${mesch12b}/init.c ${mesch12b}/matop.c ${mesch12b}/submat.c  \
              ${mesch12b}/memory.c ${mesch12b}/meminfo.c  \
              ${mesch12b}/machine.c ${mesch12b}/ivecop.c  

              # ${mesch12b}/err.c




# mensch12b from http://homepage.divms.uiowa.edu/~dstewart/meschach/



#              ${LIB2SRC}/spi1.c
# it's messy having spi1 in lib2... and other code locally?
# actually we don't need the core. we can just write registers directly...


LDSCRIPT	= ${TOP}/f410.ld


TGT_CFLAGS  += -Wno-unused-function   \
              -Werror-implicit-function-declaration \
#              -Werror




TGT_CFLAGS	+= -I./include    # FIXME, should be before lib2inc
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
