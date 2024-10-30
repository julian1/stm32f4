######################################################################
#  Project Makefile
######################################################################





BINARY		= main


SRCFILES	= src/peripheral/trigger.c     \
            src/support.c \
            src/peripheral/spi-ice40.c src/peripheral/spi-ice40-bitstream.c \
            src/peripheral/spi-4094.c src/peripheral/spi-dac8811.c src/peripheral/spi-ad5446.c \
            src/peripheral/vfd.c \
            src/device/spi1-port.c src/device/spi2-port.c  \
            src/device/u202.c src/device/fsmc.c src/device/led0.c src/device/u102.c  \
            src/main.c  \
            src/app.c  \
            src/syscalls.o \
            src/mode.o \
            src/lzo_test.c \
            src/util.c \
            src/data/data.c src/data/cal.c src/data/matrix.c src/data/mesch-support.c src/data/regression.c src/data/buffer.c \
            src/test/test01.c src/test/test02.c  src/test/test03.c  \
            src/test/test10.c  src/test/test11.c \
            src/test/test12.c  src/test/test14.c src/test/test15.c \
            src/test/test19.c \
            src/test/test20.c src/test/test40.c src/test/test41.c  src/test/test42.c   \
            src/flash/file-blob.c \
            src/data/cal-flash.c \
            src/vfd.c \



#            src/peripheral/spi-dac8734.c \




# f413
# this uses ,
# TGT_LDFLAGS	+= -T$(LDSCRIPT)
# but there is an flag order issue, so rely on toplevel Makefile.incl LDSCRIPT variable

# LDSCRIPT +=  ${TOP_DIR}/link/f413rgt6.ld
LDSCRIPT +=  ${TOP_DIR}/link/f429.ld



TGT_CFLAGS  += -Wno-unused-function   \
              -Werror-implicit-function-declaration \
              -Wno-packed-bitfield-compat

TGT_CFLAGS  += -Werror \



# local include
TGT_CFLAGS	+= -I./include

# top level include, for assert and other libraries.
TGT_CFLAGS	+= -I${TOP_DIR}/include
TGT_CFLAGS	+= -I${TOP_DIR}/lib/libopencm3/include


# libs
LDLIBS		+= ${TOP_DIR}/lib/lib2/lib2.a
LDLIBS		+= ${TOP_DIR}/lib/mesch12b/mesch12b.a
LDLIBS		+= ${TOP_DIR}/lib/lzo/lzo.a

# EXTR.   cortex-m-generic.ld is picked using the -L flag to libopencm3
# and not from $TOP/link/
LDLIBS		+= -L${TOP_DIR}/lib/libopencm3/lib  -lopencm3_stm32f4


LDLIBS		+= -lm

# LDLIBS		+=  -specs=nosys.specs -lnosys
# LDLIBS		+=  -specs=nano.specs -lnano
# LDLIBS		+=  --specs=rdimon.specs -lrdimon



# include ${TOP}/Makefile.incl
include ../..//Makefile.incl

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
