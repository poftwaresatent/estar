CPPFLAGS= @ESTAR_CPPFLAGS@
CFLAGS=   $(CPPFLAGS) @ESTAR_CFLAGS@
CXXFLAGS= $(CPPFLAGS) @ESTAR_CXXFLAGS@
LDFLAGS=  @ESTAR_LDFLAGS@

noinst_LTLIBRARIES= libpnf.la

libpnf_la_SOURCES=  BufferZone.cpp \
                    Flow.cpp \
                    PNFRiskMap.cpp \
                    pnf_cooc.c

include_HEADERS=    pnf_cooc.h \
                    BufferZone.hpp \
                    Flow.hpp \
                    PNFRiskMap.hpp

includedir=         @includedir@/pnf
