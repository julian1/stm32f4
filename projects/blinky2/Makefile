#####################################################################
#  Project Makefile
######################################################################



TOP=../../
RTOS=${TOP}/rtos/rtos_v9
LIBSRC=${TOP}/lib/src

BINARY		= main
SRCFILES	= main.c   \
            ${RTOS}/heap_4.c ${RTOS}/list.c ${RTOS}/port.c ${RTOS}/queue.c ${RTOS}/tasks.c ${RTOS}/opencm3.c \
            ${LIBSRC}/serial.c ${LIBSRC}/sleep.c ${LIBSRC}/usart.c ${LIBSRC}/miniprintf.c \


LDSCRIPT	= ../../cjmcu-407.ld  # JA


TGT_CFLAGS	+= -I${LIBSRC}/../include
TGT_CFLAGS	+= -I${RTOS}

include ../../Makefile.incl
include ../Makefile.rtos

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
