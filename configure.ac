AC_INIT([LibIqrf], [1.0.0], [marek.belisko@gmail.com], [iqrf],
        [http://open-nandra.com])

AM_INIT_AUTOMAKE(libiqrf,0.0.1)
AM_PROG_LIBTOOL

AC_PROG_INSTALL
AC_PROG_LIBTOOL

AC_CONFIG_MACRO_DIR([m4])

AC_LANG_C
AC_PROG_CC
AC_PROG_MAKE_SET

AC_HEADER_STDC

#AC_CHECK_HEADERS(usb.h,,AC_MSG_ERROR([
#required header file missing]))

#AC_CHECK_FUNCS(gethostbyname socket htons connect shutdown,,AC_MSG_ERROR([
#required standard library function missing]))

# Get libsub library and include locations
#AC_ARG_WITH([libusb-include-path],
#  [AS_HELP_STRING([--with-libusb-include-path],
#    [location of the libusb headers, defaults to /usr/include/])],
#  [USB_CFLAGS="-I$withval"],
#  [USB_CFLAGS='-I/usr/include/libusb-1.0'])
#AC_SUBST([USB_CFLAGS])

#AC_ARG_WITH([libusb-lib-path],
#  [AS_HELP_STRING([--with-libusb-lib-path], [location of the libsub libraries])],
#  [USB_LIBS="-L$withval -lusb"],
#  [USB_LIBS='-L/usr/lib/-lusb'])
#AC_SUBST([USB_LIBS])
PKG_CHECK_MODULES(LIBUSB, libusb-1.0 >= 1.0.0, [ac_libusb=yes], [ac_libusb=no])
AC_SUBST(LIBUSB_LIBS)
AC_SUBST(LIBUSB_CFLAGS)
if test x"$ac_libusb" = x"yes" ; then
        AC_DEFINE([HAVE_LIBUSB], 1, [libusb support])
fi

AC_CONFIG_FILES([libiqrf.pc])
AC_OUTPUT(Makefile)
