######################################################################
#  Makefile for top level rtos subdirectory
######################################################################

PROJS = blinky2  smu07

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
