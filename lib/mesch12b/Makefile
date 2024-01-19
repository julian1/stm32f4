######################################################################
#  Project Makefile
######################################################################




TOP=../../


#              ${mesch12b}/copy.c  ${mesch12b}/init.c ${mesch12b}/matop.c ${mesch12b}/submat.c  \
#              ${mesch12b}/memory.c ${mesch12b}/meminfo.c  \
#              ${mesch12b}/machine.c ${mesch12b}/ivecop.c \
#              ${mesch12b}/err.c \
#              ${mesch12b}/matrixio.c \
#              ${mesch12b}/lufactor.c ${mesch12b}/pxop.c ${mesch12b}/solve.c ${mesch12b}/memstat.c ${mesch12b}/norm.c \


# copy.c  init.c matop.c submat.c  memory.c meminfo.c  machine.c ivecop.c err.c matrixio.c lufactor.c pxop.c solve.c memstat.c norm.c 



mesch12b.a: copy.o  init.o matop.o submat.o  memory.o meminfo.o  machine.o ivecop.o err.o matrixio.o lufactor.o pxop.o solve.o memstat.o norm.o 
	${AR} rcs mesch12b.a ${OBJS}


SRCFILES	=  copy.c  init.c matop.c submat.c  memory.c meminfo.c  machine.c ivecop.c err.c matrixio.c lufactor.c pxop.c solve.c memstat.c norm.c 




OBJS = copy.o  init.o matop.o submat.o  memory.o meminfo.o  machine.o ivecop.o err.o matrixio.o lufactor.o pxop.o solve.o memstat.o norm.o 




TGT_CFLAGS  += -Wno-unused-function   \
              -Werror-implicit-function-declaration \
              -Wno-packed-bitfield-compat


#              -Werror \



# needed for <assert.h> in priority to pickup the function prototype
TGT_CFLAGS	+= -I${TOP}/include

TGT_CFLAGS	+= -I${TOP}/include/mesch12b/


#

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
