AC_REVISION($Id$)
AC_INIT(estar, 0.1pre, estar-devel AT lists DOT sourceforge DOT net)
AC_PREREQ(2.57)
AC_CANONICAL_HOST

AC_LANG(C++)
AC_PROG_CC
AC_PROG_CXX

dnl We'll synch CXXFLAGS at the end
CFLAGS="-Wall -pipe"

AC_ARG_ENABLE(debug,
  AC_HELP_STRING([--enable-debug], [GCC options -g -O0 (else -O3)]),
  [ CPPFLAGS="-DESTAR_DEBUG"
    CFLAGS="$CFLAGS -g -O0" ],
  [ CPPFLAGS=""
    CFLAGS="$CFLAGS -O3" ])

case "$host_os" in
  linux*)  AC_MSG_RESULT([detected Linux])
           CPPFLAGS="$CPPFLAGS -DLINUX"
           GFXLIBS="-L/usr/X11R6/lib -lglut -lGLU -lGL -lXi -lXmu"
           AC_CHECK_HEADER([GL/glut.h], [glut=GLUT], [glut=NOGLUT]);;
  darwin*) AC_MSG_RESULT([detected OSX])
           CPPFLAGS="$CPPFLAGS -DOSX"
           CFLAGS="$CFLAGS -Wno-long-double"
           GFXLIBS="-framework OpenGL -framework GLUT -lobjc"
           AC_CHECK_HEADER([GLUT/glut.h], [glut=GLUT], [glut=NOGLUT]);;
  *)       AC_MSG_ERROR([unknown host_os $host_os, update configure.ac]);;
esac
AC_SUBST(GFXLIBS)
AC_SUBST(glut)

AC_ARG_WITH(boost,
  AC_HELP_STRING([--with-boost=PATH], [specify boost install dir]),
  [ if test "x$withval" != "x" ; then
      CPPFLAGS="$CPPFLAGS -I${withval}/include -I${withval}"
      LDFLAGS="-L${withval}/lib -I${withval} $LDFLAGS"
    fi ], [])
oldCPPFLAGS="$CPPFLAGS"
test "$prefix" != "NONE" && CPPFLAGS="$CPPFLAGS -I$prefix/include"
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
  CPPFLAGS="-I${ABSTOPBUILDDIR}/include $CPPFLAGS"
else
  CPPFLAGS="-I${ABSTOPBUILDDIR}/include $CPPFLAGS -I${prefix}/include"
  LDFLAGS="$LDFLAGS -L$prefix/lib"
fi
CXXFLAGS="$CFLAGS"

AC_SUBST(PACKAGE_VERSION)
AC_OUTPUT(Makefile estar.pc Doxyfile)
