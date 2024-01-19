######################################################################
#  Project Makefile
######################################################################


TOP=../../

BINARY		= main

#SRCFILES	= mcp3208.c  util.c \

SRCFILES	= src/main.c  src/spi-port.c  src/w25.c  src/4094.c src/spi-ice40.c  src/mux.c  \
            src/test/test05.c src/test/test06.c src/test/test08.c src/test/test11.c src/test/test15.c src/test/test14.c  src/test/test16.c \
            src/mcu-temp.c \
            src/stream-flash.c src/app-functions.c src/cal.c src/file-blob.c src/measurement.c src/calc.c src/mesch.c \



# f407
#LDSCRIPT	= ${TOP}/cjmcu-407.ld
# LDSCRIPT	= ${TOP}/f410.ld
LDSCRIPT	= ${TOP}/f411ceu.ld


TGT_CFLAGS  += -Wno-unused-function   \
              -Werror-implicit-function-declaration \
              -Wno-packed-bitfield-compat

#              -Werror \



# local include
TGT_CFLAGS	+= -I./include

# top level include, for assert and other libraries.
TGT_CFLAGS	+= -I${TOP}/include

# mesch... but
TGT_CFLAGS	+= -I${TOP}/include/mesch12b    # this is wrong. should be explicit




LDLIBS		+= ${TOP}/lib/lib2/lib2.a
LDLIBS		+= ${TOP}/lib/mesch12b/mesch12b.a
LDLIBS		+= -lm
# LDLIBS		+=  -specs=nosys.specs -lnosys
# LDLIBS		+=  -specs=nano.specs -lnano
# LDLIBS		+=  --specs=rdimon.specs -lrdimon



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
