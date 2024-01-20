######################################################################
#  Project Makefile
######################################################################




TOP=../../
LIB2SRC=${TOP}/lib2/src
LIB2INC=${TOP}/lib2/include

GFXSRC=./gfx/src
GFXINC=./gfx/include


AGGSRC=${TOP}/agg-svn-r138-agg-2.4/examples
AGGINC=${TOP}/agg-svn-r138-agg-2.4/include/ 




BINARY		= main
SRCFILES	= ./src/main.c ./src/lcd_spi.c ./src/rotary.c ./src/agg_test.cpp \
              ${GFXSRC}/context.c ${GFXSRC}/gfx.c \
              ${LIB2SRC}/buffer.c ${LIB2SRC}/miniprintf2.c ${LIB2SRC}/usart.c \
              ${AGGSRC}/circles.cpp

            # glcdfont.c ??

LDSCRIPT	= ${TOP}/f410.ld


TGT_CFLAGS  += -Wno-unused-function   \
              -Werror-implicit-function-declaration \
              -Werror



TGT_CFLAGS	+= -I./include
TGT_CFLAGS	+= -I${GFXINC}
TGT_CXXFLAGS += -I${GFXINC}      
TGT_CXXFLAGS += -I${AGGINC}      
TGT_CFLAGS	+= -I${LIB2INC}

LDLIBS		+= -lm  -lstdc++  #-static-libstdc++


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
