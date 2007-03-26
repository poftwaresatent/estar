CPPFLAGS= @ESTAR_CPPFLAGS@
CFLAGS=   $(CPPFLAGS) @ESTAR_CFLAGS@
CXXFLAGS= $(CPPFLAGS) @ESTAR_CXXFLAGS@
LDFLAGS=  @ESTAR_LDFLAGS@

if ENABLE_GFX
  GFX_DIR= gfx
  GFX_LIB= gfx/libgfx.la
endif

SUBDIRS= estar pnf $(GFX_DIR) bin

lib_LTLIBRARIES=     libestar.la
libestar_la_SOURCES= 
libestar_la_LIBADD=  estar/libestarsub.la pnf/libpnf.la $(GFX_LIB)

SRCDIR=              @abs_top_srcdir@
BUILDDIR=            @abs_top_builddir@

BUILT_SOURCES=       incsym

incsym:
	test -d $(BUILDDIR)/include || mkdir -p $(BUILDDIR)/include
	test -L $(BUILDDIR)/include/estar || ln -s $(SRCDIR)/estar $(BUILDDIR)/include/estar
	test -L $(BUILDDIR)/include/gfx || ln -s $(SRCDIR)/gfx $(BUILDDIR)/include/gfx
	test -L $(BUILDDIR)/include/pnf || ln -s $(SRCDIR)/pnf $(BUILDDIR)/include/pnf

libs: $(lib_LTLIBRARIES)
