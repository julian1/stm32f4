######################################################################
#  Project Makefile
######################################################################

BINARY		= main


SRCFILES	= src/main.c src/dac8734.c src/ref.c src/rails.c src/usart.c src/serial.c src/sleep.c \
             src/led.c src/mcu_adc.c src/slope_adc.c src/pvd.c  \
            ../lib/src/miniprintf.c \
            rtos/heap_4.c rtos/list.c rtos/port.c rtos/queue.c rtos/tasks.c rtos/opencm3.c

#LDSCRIPT	= stm32f103c8t6.ld
LDSCRIPT	= ../../cjmcu-407.ld  # JA

TGT_CFLAGS	+= -I./include
TGT_CFLAGS	+= -I../lib/include

# JA
CFLAGS    += -Wno-unused-function

# we want error, not warning
# to avoid progressing, and linking regardless of whether function exists.
CFLAGS    += -Werror-implicit-function-declaration




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
