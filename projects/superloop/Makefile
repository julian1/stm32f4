######################################################################
#  Project Makefile
######################################################################


#X=../rtos/FreeRTOSv202012.00/FreeRTOS/Source/
#XH=${X}/include



BINARY		= main
SRCFILES	= main.c   \
              ../lib/src/miniprintf2.c
            


LDSCRIPT	= ../../cjmcu-407.ld  # JA


TGT_CFLAGS  += -Wno-unused-function

#TGT_CFLAGS	+= -I./rtos  # in priority
#TGT_CFLAGS	+= -I${XH}

TGT_CFLAGS	+= -I../lib/include
#LDLIBS		+= -lm


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
