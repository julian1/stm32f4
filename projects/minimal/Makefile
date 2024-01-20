######################################################################
#  Project Makefile
######################################################################


TOP=../../

BINARY		= main

#SRCFILES	= mcp3208.c  util.c \

SRCFILES	= src/main.c  



# f407
#LDSCRIPT	= ${TOP}/cjmcu-407.ld
# LDSCRIPT	= ${TOP}/f410.ld
LDSCRIPT	= ${TOP}/link/f411ceu.ld


TGT_CFLAGS  += -Wno-unused-function   \
              -Werror-implicit-function-declaration \
              -Wno-packed-bitfield-compat

#              -Werror \



# local include
TGT_CFLAGS	+= -I./include

# top level include, for assert and other libraries.
TGT_CFLAGS	+= -I${TOP}/include





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
