AUTOMAKE_OPTIONS = subdir-objects

ACLOCAL_AMFLAGS = -I m4

lib_LTLIBRARIES = libiqrf.la

libiqrf_la_SOURCES = src/iqrf.c \
		    src/spi.c \
		    src/usb.c

include_HEADERS = src/iqrf.h

libiqrf_la_LDFLAGS = -version-info 1:0:0

libiqrf_la_CPPFLAGS = $(USB_CFLAGS)
libiqrf_la_LIBADD = $(USB_LIBS)

.PHONY: ChangeLog
ChangeLog:
	git --git-dir $(top_srcdir)/.git log > ChangeLog || touch ChangeLog


#pkgconfigdir = $(libdir)/pkgconfig
#pkgconfig_DATA = libiqrf.pc