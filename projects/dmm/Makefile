######################################################################
#  Project Makefile
######################################################################




TOP=../../
LIB2SRC=${TOP}/lib2/src
LIB2INC=${TOP}/lib2/include



mesch12b=${TOP}/mesch12b


# check ../../Makefile.incl,  no optimization flags are commented.
# CFLAGS = -mthumb -mcpu=cortex-m4 -ggdb -Os
# CFLAGS += -mfloat-abi=hard



BINARY		= main
#SRCFILES	= mcp3208.c  util.c \
SRCFILES	= flash.c  util.c \

# SRCFILES	= src/main.c   src/mcp3208.c src/w25.c src/dac8734.c src/ads131a04.c src/spi-ice40.c src/spi1.c src/mux.c src/util.c \

SRCFILES	= src/main.c   src/spi-port.c  src/w25.c  src/4094.c src/spi-ice40.c  src/mux.c  \
            src/test/test05.c src/test/test06.c src/test/test08.c src/test/test11.c src/test/test15.c src/test/test14.c  src/test/test16.c \
            src/mcu-temp.c \
            src/stream-flash.c \
            src/app-functions.c \
            src/cal.c \
            src/file-blob.c \
            src/measurement.c \
            src/calc.c src/mesch.c \
              ${LIB2SRC}/util.c ${LIB2SRC}/format.c \
              ${LIB2SRC}/cbuffer.c  ${LIB2SRC}/streams.c  ${LIB2SRC}/usart.c  ${LIB2SRC}/cstring.c \
              ${LIB2SRC}/fbuffer.c   ${LIB2SRC}/flash.c  ${LIB2SRC}/stats.c \
              ${mesch12b}/copy.c  ${mesch12b}/init.c ${mesch12b}/matop.c ${mesch12b}/submat.c  \
              ${mesch12b}/memory.c ${mesch12b}/meminfo.c  \
              ${mesch12b}/machine.c ${mesch12b}/ivecop.c \
              ${mesch12b}/err.c \
              ${mesch12b}/matrixio.c \
              ${mesch12b}/lufactor.c ${mesch12b}/pxop.c ${mesch12b}/solve.c ${mesch12b}/memstat.c ${mesch12b}/norm.c \



#  src/test17.c


# ${LIB2SRC}/format_float.c
# ${LIB2SRC}/format_bits.c



# ${LIB2SRC}/miniprintf2.c

#SRCFILES	= flash.c   \

# f407
#LDSCRIPT	= ${TOP}/cjmcu-407.ld
# LDSCRIPT	= ${TOP}/f410.ld
LDSCRIPT	= ${TOP}/f411ceu.ld


TGT_CFLAGS  += -Wno-unused-function   \
              -Werror-implicit-function-declaration \
              -Wno-packed-bitfield-compat

TGT_CFLAGS	+= -I${mesch12b}

#              -Werror \


TGT_CFLAGS	+= -I${LIB2INC}
TGT_CFLAGS	+= -I./include    # FIXME, should be before lib2inc

# pow()
LDLIBS		+=  -lm

# LDLIBS		+=  -specs=nosys.specs -lnosys -lm
# LDLIBS		+=  -specs=nano.specs -lnano -lm
# LDLIBS		+=  --specs=rdimon.specs -lrdimon -lm



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
