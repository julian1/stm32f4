######################################################################
#  Project Makefile
######################################################################




TOP=../../



lib2.a: decompress_unlzo.o    lzo1x_decompress_safe.o  lzo1x_compress.o
	${AR} rcs lib2.a ${OBJS}


SRCFILES	=  decompress_unlzo.c    lzo1x_decompress_safe.c  lzo1x_compress.c


OBJS = decompress_unlzo.o   lzo1x_decompress_safe.o  lzo1x_compress.o




TGT_CFLAGS  += -Wno-unused-function   \
              -Werror-implicit-function-declaration \
              -Wno-packed-bitfield-compat



# needed for <assert.h> in priority to pickup the function prototype
TGT_CFLAGS	+= -I${TOP}/include

TGT_CFLAGS	+= -I${TOP}/include/lzo/


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
