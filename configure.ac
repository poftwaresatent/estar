AC_REVISION($Id$)
AC_INIT(estar, 0.1pre, estar-devel AT lists DOT sourceforge DOT net)
AC_PREREQ(2.57)
AM_INIT_AUTOMAKE([-Wall -Werror foreign])
AC_CANONICAL_HOST

AC_LANG(C++)
AC_PROG_CC
AC_PROG_CXX

ESTAR_CFLAGS="-Wall -pipe"

AC_ARG_ENABLE(debug,
  AC_HELP_STRING([--enable-debug], [GCC options -g -O0 (else -O3)]),
  [ ESTAR_CPPFLAGS="-DESTAR_DEBUG"
    ESTAR_CFLAGS="$ESTAR_CFLAGS -g -O0" ],
  [ ESTAR_CPPFLAGS=""
    ESTAR_CFLAGS="$ESTAR_CFLAGS -O3" ])

case "$host_os" in
  linux*)  AC_MSG_RESULT([detected Linux])
           ESTAR_CPPFLAGS="$ESTAR_CPPFLAGS -DLINUX"
           GFXLIBS="-L/usr/X11R6/lib -lglut -lGLU -lGL -lXi -lXmu"
           AC_CHECK_HEADER([GL/glut.h],
             [ AM_CONDITIONAL([ENABLE_GFX], [true]) ],
	     [ AM_CONDITIONAL([ENABLE_GFX], [false]) ]);;
  darwin*) AC_MSG_RESULT([detected OSX])
           ESTAR_CPPFLAGS="$ESTAR_CPPFLAGS -DOSX"
           ESTAR_CFLAGS="$ESTAR_CFLAGS -Wno-long-double"
           GFXLIBS="-framework OpenGL -framework GLUT -lobjc"
           AC_CHECK_HEADER([GLUT/glut.h],
             [ AM_CONDITIONAL([ENABLE_GFX], [true]) ],
	     [ AM_CONDITIONAL([ENABLE_GFX], [false]) ]);;
  *)       AC_MSG_ERROR([unknown host_os $host_os, update configure.ac]);;
esac
AC_SUBST(GFXLIBS)

AC_ARG_WITH(boost,
  AC_HELP_STRING([--with-boost=PATH], [specify boost install dir]),
  [ if test "x$withval" != "x" ; then
      ESTAR_CPPFLAGS="$ESTAR_CPPFLAGS -I${withval}/include -I${withval}"
      ESTAR_LDFLAGS="-L${withval}/lib -I${withval} $ESTAR_LDFLAGS"
    fi ], [])
oldCPPFLAGS="$CPPFLAGS"
test "$prefix" != "NONE" && CPPFLAGS="$ESTAR_CPPFLAGS -I$prefix/include"
AC_CHECK_HEADER([boost/shared_ptr.hpp], [], [
  AC_MSG_ERROR([cannot compile boost/shared_ptr.hpp (CPPFLAGS=$CPPFLAGS)])])
CPPFLAGS="$oldCPPFLAGS"

AC_DISABLE_STATIC
case $host_os in
  darwin*) AC_DISABLE_SHARED;;
  *)       AC_DISABLE_STATIC;;
esac
AC_PROG_LIBTOOL
AC_PROG_INSTALL
AC_PROG_AWK

ABSTOPBUILDDIR=`pwd`
if test -z "$ABSTOPBUILDDIR" ; then
  AC_MSG_ERROR([could not determine current working directory])
fi
if test "$prefix" = "NONE" ; then
  ESTAR_CPPFLAGS="-I${ABSTOPBUILDDIR}/include $ESTAR_CPPFLAGS"
else
  ESTAR_CPPFLAGS="-I${ABSTOPBUILDDIR}/include $ESTAR_CPPFLAGS -I${prefix}/include"
  ESTAR_LDFLAGS="$ESTAR_LDFLAGS -L$prefix/lib"
fi
ESTAR_CXXFLAGS="$ESTAR_CFLAGS"

AC_SUBST(PACKAGE_VERSION)
AC_SUBST(ESTAR_CPPFLAGS)
AC_SUBST(ESTAR_CFLAGS)
AC_SUBST(ESTAR_CXXFLAGS)
AC_SUBST(ESTAR_LDFLAGS)
AC_CONFIG_HEADERS([config.h])
AC_CONFIG_FILES([Makefile
                 estar.pc
		 Doxyfile
		 bin/Makefile
		 estar/Makefile
		 gfx/Makefile
		 pnf/Makefile])
AC_OUTPUT
