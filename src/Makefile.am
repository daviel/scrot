# A list of all the files in the current directory which can be regenerated
MAINTAINERCLEANFILES = Makefile.in				

LDFLAGS           = -L/usr/X11R6/lib
INCLUDES          = -g -O3 -Wall -I/usr/X11R6/include \
$(X_CFLAGS) -I$(prefix)/include -I$(includedir) -I. \
-DPREFIX=\""$(prefix)"\" @GIBLIB_CFLAGS@
LIBOBJS = @LIBOBJS@

bin_PROGRAMS      = scrot
scrot_SOURCES       = main.c scrot.h

scrot_LDADD         = -lX11 @GIBLIB_LIBS@
