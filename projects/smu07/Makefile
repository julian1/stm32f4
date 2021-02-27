######################################################################
#  Project Makefile
######################################################################



TOP=../../
RTOS=${TOP}/rtos/rtos_v9
LIBSRC=${TOP}/lib/src
LIBINC=${TOP}/lib/include



BINARY		= main


SRCFILES	= src/main.c src/dac8734.c src/ref.c src/rails.c   \
             src/led.c src/mcu_adc.c src/slope_adc.c src/pvd.c  \
            ${LIBSRC}/serial.c ${LIBSRC}/sleep.c ${LIBSRC}/usart.c ${LIBSRC}/miniprintf.c \
            ${RTOS}/heap_4.c ${RTOS}/list.c ${RTOS}/port.c ${RTOS}/queue.c ${RTOS}/tasks.c ${RTOS}/opencm3.c


LDSCRIPT	= ${TOP}/cjmcu-407.ld


TGT_CFLAGS	+= -I./include
TGT_CFLAGS	+= -I${LIBINC}
TGT_CFLAGS	+= -I${RTOS}


# JA
CFLAGS    += -Wno-unused-function

# we want error, not warning
# to avoid progressing, and linking regardless of whether function exists.
CFLAGS    += -Werror-implicit-function-declaration




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
