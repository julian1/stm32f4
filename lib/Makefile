######################################################################
#  Makefile for top level rtos subdirectory
######################################################################

# PROJS = blinky2  smu07 ads131a04-sl
PROJS = crc32  lib2  lzo  mesch12b  mongoose

all:	
	for proj in $(PROJS) ; do \
		$(MAKE) -$(MAKEFLAGS) -C ./$$proj ; \
	done

clean:	
	for proj in $(PROJS) ; do \
		$(MAKE) -$(MAKEFLAGS) -C ./$$proj clean ; \
	done

clobber:
	for proj in $(PROJS) ; do \
		$(MAKE) -$(MAKEFLAGS) -C ./$$proj clobber ; \
	done

# End
