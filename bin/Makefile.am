CPPFLAGS= @ESTAR_CPPFLAGS@
CFLAGS=   @ESTAR_CFLAGS@
CXXFLAGS= @ESTAR_CXXFLAGS@
LDFLAGS=  @ESTAR_LDFLAGS@

if ESTAR_ENABLE_GFX
  GFX_PROGS=  test_estar_gfx estar_compare_gfx test_pnf_gfx
  GFX_LIBADD= ../gfx/libgfx.la @ESTAR_GFXLIBS@
endif

bin_PROGRAMS= test_dbg_opt \
              test_estar \
              test_estar_queue \
              test_fake_os \
              test_pnf_cooc \
              test_pnf_cooc3d \
              test_pnf_riskmap \
              test_shape \
              pgm2ascii \
              $(GFX_PROGS)

test_dbg_opt_SOURCES=     test_dbg_opt.cpp
test_dbg_opt_LDADD=       ../libestar.la
test_estar_SOURCES=       test_estar.cpp
test_estar_LDADD=         ../libestar.la
test_estar_queue_SOURCES= test_estar_queue.cpp
test_estar_queue_LDADD=   ../libestar.la
test_fake_os_SOURCES=     test_fake_os.cpp
test_fake_os_LDADD=       ../libestar.la
test_pnf_cooc_SOURCES=    test_pnf_cooc.cpp
test_pnf_cooc_LDADD=       ../libestar.la
test_pnf_cooc3d_SOURCES=  test_pnf_cooc3d.c
test_pnf_cooc3d_LDADD=    ../libestar.la
test_pnf_riskmap_SOURCES= test_pnf_riskmap.cpp
test_pnf_riskmap_LDADD=   ../libestar.la
test_shape_SOURCES=       test_shape.cpp
test_shape_LDADD=         ../libestar.la
pgm2ascii_SOURCES=        pgm2ascii.cpp
pgm2ascii_LDADD=          -lnetpbm

if ESTAR_ENABLE_GFX
  test_estar_gfx_SOURCES= test_estar_gfx.cpp Getopt.cpp
  test_estar_gfx_LDADD=   ../libestar.la $(GFX_LIBADD)
  estar_compare_gfx_SOURCES= estar_compare_gfx.cpp Getopt.cpp
  estar_compare_gfx_LDADD=   ../libestar.la $(GFX_LIBADD)
  test_pnf_gfx_SOURCES=   test_pnf_gfx.cpp
  test_pnf_gfx_LDADD=     ../libestar.la $(GFX_LIBADD)
endif

dist_bin_SCRIPTS= plot_pnf.sh pnf_3d_riskplot.sh

../libestar.la:
	$(MAKE) -C .. libs
