CPPFLAGS= @ESTAR_CPPFLAGS@
CFLAGS=   @ESTAR_CFLAGS@
CXXFLAGS= @ESTAR_CXXFLAGS@
LDFLAGS=  @ESTAR_LDFLAGS@

if ESTAR_ENABLE_GFX
  GFX_SRC= graphics.cpp
  GFX_HDR= graphics.hpp
endif

noinst_LTLIBRARIES=     libestarsub.la

libestarsub_la_SOURCES= Algorithm.cpp \
                        AlphaKernel.cpp \
                        Facade.cpp \
                        ComparisonFacade.cpp \
                        Grid.cpp \
                        Kernel.cpp \
                        LSMKernel.cpp \
                        NF1Kernel.cpp \
                        Propagator.cpp \
                        PropagatorFactory.cpp \
                        Queue.cpp \
                        Region.cpp \
                        Sprite.cpp \
                        Upwind.cpp \
                        base.cpp \
                        check.cpp \
                        dump.cpp \
                        numeric.cpp \
                        util.cpp \
                        $(GFX_SRC)

include_HEADERS=        Algorithm.hpp \
                        AlphaKernel.hpp \
                        FacadeWriteInterface.hpp \
                        FacadeReadInterface.hpp \
                        Facade.hpp \
                        ComparisonFacade.hpp \
                        Grid.hpp \
                        Kernel.hpp \
                        LSMKernel.hpp \
                        NF1Kernel.hpp \
                        Propagator.hpp \
                        PropagatorFactory.hpp \
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
                        pdebug.hpp \
                        $(GFX_HDR)

includedir=             @includedir@/estar
