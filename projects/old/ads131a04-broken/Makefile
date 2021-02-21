######################################################################
#  Project Makefile
######################################################################

# AHHHH think we should pull the rtos back with local changes

X=../rtos/FreeRTOSv202012.00/FreeRTOS/Source/

XH=${X}/include



BINARY		= main
SRCFILES	= main.c   \
            ${X}/list.c  ${X}/tasks.c ${X}/queue.c ${X}/timers.c \
            ./rtos/port.c ./rtos/heap_4.c ./rtos/opencm3.c \
            ../lib/src/serial.c ../lib/src/sleep.c ../lib/src/usart.c \
            ../lib/src/miniprintf.c \


#LDSCRIPT	= stm32f103c8t6.ld
LDSCRIPT	= ../../cjmcu-407.ld  # JA




TGT_CFLAGS	+= -I./rtos  # in priority
TGT_CFLAGS	+= -I${XH}

TGT_CFLAGS	+= -I../lib/include



LDLIBS		+= -lm

#

include ../../Makefile.incl
#include ../Makefile.rtos

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
