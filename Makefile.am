CPPFLAGS= @ESTAR_CPPFLAGS@
CFLAGS=   @ESTAR_CFLAGS@
CXXFLAGS= @ESTAR_CXXFLAGS@
LDFLAGS=  @ESTAR_LDFLAGS@

if ESTAR_ENABLE_GFX
  GFX_DIR= gfx
  GFX_LIB= gfx/libgfx.la
endif

SUBDIRS= estar pnf $(GFX_DIR) bin

lib_LTLIBRARIES=     libestar.la
libestar_la_SOURCES= 
libestar_la_LIBADD=  estar/libestarsub.la pnf/libpnf.la $(GFX_LIB)

SRCDIR=              @abs_top_srcdir@
BUILDDIR=            @abs_top_builddir@

EXTRA_DIST=          ABOUT \
                     LICENSE.BSD \
                     LICENSE.GPL \
                     CHANGELOG \
                     bootstrap-buildsystem.sh \
                     build-stage.sh \
                     doxylogo.png \
                     footer.html \
                     mainpage.dox \
                     estar/cwrap.h \
                     bin/Getopt.hpp \
                     misc/example-testqorder-hex.log \
                     misc/example-testqorder-hex.results \
                     misc/grid-hex-small.txt \
                     misc/grid-nhp-1.txt \
                     misc/grid-nhp-2.txt \
                     misc/grid-nhp-3.txt \
                     misc/grid-nhp-4.txt \
                     misc/grid-small.txt \
                     misc/pnf-dynamic-impulse.pnf \
                     misc/pnf-setup-iros06-a.pnf \
                     misc/pnf-setup-iros06-b.pnf \
                     misc/pnf-setup-iros06-buffered-b.pnf \
                     misc/pnf-setup-iros06-c.pnf \
                     misc/pnf-setup-iros06-d.pnf \
                     misc/pnf-setup-iros06-e.pnf \
                     misc/pnf-setup-iros06-f.pnf \
                     misc/pnf-setup-star06-a.pnf \
                     misc/pnf-setup-star06-b.pnf \
                     misc/pnf-static-impulse.pnf

pkgconfig_DATA= estar.pc
pkgconfigdir=   $(libdir)/pkgconfig

BUILT_SOURCES=       incsym

incsym:
	test -d $(BUILDDIR)/include || mkdir -p $(BUILDDIR)/include
	test -L $(BUILDDIR)/include/estar || ln -s $(SRCDIR)/estar $(BUILDDIR)/include/estar
	test -L $(BUILDDIR)/include/gfx || ln -s $(SRCDIR)/gfx $(BUILDDIR)/include/gfx
	test -L $(BUILDDIR)/include/pnf || ln -s $(SRCDIR)/pnf $(BUILDDIR)/include/pnf

libs: $(lib_LTLIBRARIES)

doc:
	doxygen Doxyfile

ChangeLog:
	test -f Changelog || echo "see http://estar.svn.sourceforge.net/viewvc/estar/" > ChangeLog

CHANGELOG:
	test -f CHANGELOG || echo "see http://estar.svn.sourceforge.net/viewvc/estar/" > CHANGELOG
