######################################################################
#  Project Makefile
######################################################################

BINARY		= main
#SRCFILES	= main.c    rtos/heap_4.c rtos/list.c rtos/port.c rtos/tasks.c rtos/opencm3.c
SRCFILES	= src/main.c src/dac8734.c src/ref.c src/rails.c src/usart.c src/sleep.c src/led.c \
            rtos/heap_4.c rtos/list.c rtos/port.c rtos/queue.c rtos/tasks.c rtos/opencm3.c

#LDSCRIPT	= stm32f103c8t6.ld
LDSCRIPT	= ../../cjmcu-407.ld  # JA

TGT_CFLAGS	+= -I./include

# JA
CFLAGS    += -Wno-unused-function


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
