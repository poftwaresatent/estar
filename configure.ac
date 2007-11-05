AC_REVISION($Id$)
AC_INIT([estar],[0.3pre],[estar-devel AT lists DOT sourceforge DOT net])
AC_CONFIG_AUX_DIR([.])
AC_PREREQ(2.57)
AM_INIT_AUTOMAKE([-Wall -Werror foreign])
AC_CANONICAL_HOST

AC_LANG(C++)
AC_PROG_CC
AC_PROG_CXX

ESTAR_CFLAGS="-Wall -pipe"

AC_ARG_ENABLE(debug,
  AS_HELP_STRING([--enable-debug], [GCC options -g -O0 (else -O3)]),
  [ ESTAR_CPPFLAGS="-DESTAR_DEBUG"
    ESTAR_CFLAGS="$ESTAR_CFLAGS -g -O0" ],
  [ ESTAR_CPPFLAGS=""
    ESTAR_CFLAGS="$ESTAR_CFLAGS -O3" ])

AC_ARG_ENABLE(pedantic,
  AS_HELP_STRING([--enable-pedantic], [GCC options -pedantic (else -Wall)]),
  [ ESTAR_CFLAGS="$ESTAR_CFLAGS -pedantic" ],
  [ ESTAR_CFLAGS="$ESTAR_CFLAGS -Wall" ])

case "$host_os" in
  linux*)  AC_MSG_NOTICE([detected Linux])
           ESTAR_CPPFLAGS="$ESTAR_CPPFLAGS -DLINUX"
           ESTAR_GFXLIBS="-L/usr/X11R6/lib -lglut -lGLU -lGL -lXi -lXmu"
           AC_CHECK_HEADER([GL/glut.h],
             [ AM_CONDITIONAL([ESTAR_ENABLE_GFX], [true]) ],
	     [ AM_CONDITIONAL([ESTAR_ENABLE_GFX], [false]) ]);;
  darwin*) AC_MSG_NOTICE([detected OSX])
           ESTAR_CPPFLAGS="$ESTAR_CPPFLAGS -DOSX"
           ESTAR_CFLAGS="$ESTAR_CFLAGS -Wno-long-double"
           ESTAR_GFXLIBS="-framework OpenGL -framework GLUT -lobjc"
           AC_CHECK_HEADER([GLUT/glut.h],
             [ AM_CONDITIONAL([ESTAR_ENABLE_GFX], [true]) ],
	     [ AM_CONDITIONAL([ESTAR_ENABLE_GFX], [false]) ]);;
  *openbsd*) AC_MSG_NOTICE([detected OpenBSD])
             ESTAR_CPPFLAGS="$ESTAR_CPPFLAGS -DOPENBSD -I/usr/local/include -I/usr/X11R6/include"
	     ESTAR_LDFLAGS="-lpthread $ESTAR_LDFLAGS -L/usr/local/lib -L/usr/X11R6/lib"
             ESTAR_GFXLIBS="-L/usr/X11R6/lib -lX11 -lXi -lXmu -lglut -lGLU -lGL"
             oldCPPFLAGS="$CPPFLAGS"
             if test "$prefix" = "NONE"; then
               CPPFLAGS="$ESTAR_CPPFLAGS"
             else
               CPPFLAGS="$ESTAR_CPPFLAGS -I$prefix/include"
             fi
             AC_CHECK_HEADER([GL/glut.h],
               [ AM_CONDITIONAL([ESTAR_ENABLE_GFX], [true]) ],
	       [ AM_CONDITIONAL([ESTAR_ENABLE_GFX], [false]) ])
             CPPFLAGS="$oldCPPFLAGS";;
  *)       AC_MSG_ERROR([unknown host_os $host_os, update configure.ac]);;
esac
AC_SUBST(ESTAR_GFXLIBS)

AC_ARG_WITH(boost,
  AS_HELP_STRING([--with-boost=PATH], [specify boost install dir]),
  [ if test "x$withval" != "x" ; then
      ESTAR_CPPFLAGS="$ESTAR_CPPFLAGS -I${withval}/include -I${withval}"
      ESTAR_LDFLAGS="-L${withval}/lib $ESTAR_LDFLAGS"
    fi ], [])
oldCPPFLAGS="$CPPFLAGS"
if test "$prefix" = "NONE"; then
  CPPFLAGS="$ESTAR_CPPFLAGS"
else
  CPPFLAGS="$ESTAR_CPPFLAGS -I$prefix/include"
fi
AC_CHECK_HEADER([boost/shared_ptr.hpp], [], [
  AC_MSG_NOTICE([failed to find boost smart pointer library:])
  AC_MSG_NOTICE([  cannot compile boost/shared_ptr.hpp])
  AC_MSG_NOTICE([  CPPFLAGS=$CPPFLAGS])
  AC_MSG_ERROR([  specify boost library install path (--with-boost=PATH)]) ])
AC_CHECK_HEADER([boost/graph/adjacency_list.hpp], [], [
  AC_MSG_NOTICE([failed to find boost graph library:])
  AC_MSG_NOTICE([  cannot compile boost/graph/adjacency_list.hpp])
  AC_MSG_NOTICE([  CPPFLAGS=$CPPFLAGS])
  AC_MSG_ERROR([  specify boost library install path (--with-boost=PATH)]) ])
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

AC_MSG_NOTICE([==================================================])
AC_MSG_NOTICE([Configuration Summary:])
if test -z "$ESTAR_ENABLE_GFX_TRUE"; then
  AC_MSG_NOTICE([  ESTAR_ENABLE_GFX: true - graphical components will be built])
  AC_MSG_NOTICE([  ESTAR_GFXLIBS: $ESTAR_GFXLIBS])
else
  AC_MSG_NOTICE([  ESTAR_ENABLE_GFX: false - skip graphical components])
  AC_MSG_NOTICE([  ESTAR_GFXLIBS: $ESTAR_GFXLIBS])
fi
AC_MSG_NOTICE([  ESTAR_CPPFLAGS: $ESTAR_CPPFLAGS])
AC_MSG_NOTICE([  ESTAR_CFLAGS: $ESTAR_CFLAGS])
AC_MSG_NOTICE([  ESTAR_CXXFLAGS: $ESTAR_CXXFLAGS])
AC_MSG_NOTICE([  ESTAR_LDFLAGS: $ESTAR_LDFLAGS])
AC_MSG_NOTICE([==================================================])
