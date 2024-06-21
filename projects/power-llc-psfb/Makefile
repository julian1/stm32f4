


BINARY		= main


SRCFILES	=  src/main.c  \
               src/syscalls.o \


LDSCRIPT +=  ${TOP_DIR}/link/f410.ld



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

# EXTR.   cortex-m-generic.ld is picked using the -L flag to libopencm3
# and not from $TOP/link/
LDLIBS		+= -L${TOP_DIR}/lib/libopencm3/lib  -lopencm3_stm32f4


LDLIBS		+= -lm



include ${TOP_DIR}../../Makefile.incl

