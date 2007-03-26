CPPFLAGS= @ESTAR_CPPFLAGS@
CFLAGS=   $(CPPFLAGS) @ESTAR_CFLAGS@
CXXFLAGS= $(CPPFLAGS) @ESTAR_CXXFLAGS@
LDFLAGS=  @ESTAR_LDFLAGS@

if ENABLE_GFX
  GFX_SRC= graphics.cpp
  GFX_HDR= graphics.hpp
endif

noinst_LTLIBRARIES=     libestarsub.la

libestarsub_la_SOURCES= Algorithm.cpp \
                        AlphaKernel.cpp \
                        Facade.cpp \
                        Grid.cpp \
                        Kernel.cpp \
                        LSMKernel.cpp \
                        NF1Kernel.cpp \
                        Propagator.cpp \
                        Queue.cpp \
                        Region.cpp \
                        Sprite.cpp \
                        Upwind.cpp \
                        base.cpp \
                        check.cpp \
                        cwrap.cpp \
                        dump.cpp \
                        numeric.cpp \
                        util.cpp \
                        $(GFX_SRC)

include_HEADERS=        Algorithm.hpp \
                        AlphaKernel.hpp \
                        Facade.hpp \
                        Grid.hpp \
                        Kernel.hpp \
                        LSMKernel.hpp \
                        NF1Kernel.hpp \
                        Propagator.hpp \
                        Queue.hpp \
                        Region.hpp \
                        RiskMap.hpp \
                        Sprite.hpp \
                        Upwind.hpp \
                        base.hpp \
                        check.hpp \
                        dump.hpp \
                        numeric.hpp \
                        util.hpp \
                        $(GFX_HDR)

includedir=             @includedir@/estar
