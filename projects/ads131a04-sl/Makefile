######################################################################
#  Project Makefile
######################################################################



BINARY		= main
SRCFILES	= main.c ads131a04.c  \
              ../lib2/src/stats.c \
              ../lib2/src/buffer.c \
              ../lib2/src/miniprintf2.c ../lib2/src/usart2.c



LDSCRIPT	= ../../cjmcu-407.ld  # JA


TGT_CFLAGS  += -Wno-unused-function   \
              -Werror-implicit-function-declaration \
              -Werror

#              -Wno-missing-prototypes


TGT_CFLAGS	+= -I../lib2/include

LDLIBS		+= -lm


include ../../Makefile.incl

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
