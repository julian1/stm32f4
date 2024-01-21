######################################################################
#  Project Makefile
######################################################################


TOP=../../

BINARY		= main

#SRCFILES	= mcp3208.c  util.c \

SRCFILES	= src/main.c  src/lzo_test.c src/ice40-extra.c src/ice40-bitstream.c



# f413
LDSCRIPT	= ${TOP}/link/f413rgt6.ld


TGT_CFLAGS  += -Wno-unused-function   \
              -Werror-implicit-function-declaration \
              -Wno-packed-bitfield-compat

TGT_CFLAGS  += -Werror \



# local include
TGT_CFLAGS	+= -I./include

# top level include, for assert and other libraries.
TGT_CFLAGS	+= -I${TOP}/include





LDLIBS		+= ${TOP}/lib/lib2/lib2.a
LDLIBS		+= ${TOP}/lib/mesch12b/mesch12b.a
LDLIBS		+= ${TOP}/lib/lzo/lzo.a
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
