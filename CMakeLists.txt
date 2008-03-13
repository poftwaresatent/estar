### Simplified CMake build file for E-Star.
###
### We only build the core library with CMake, no PNF or fancy test
### programs. If you want any of those, use the GNU Autoconf /
### Automake build system.

PROJECT (E-Star)
SET (CMAKE_VERBOSE_MAKEFILE ON)

MESSAGE ("System: ${CMAKE_SYSTEM_NAME}")



### Some compiler flags are not understood by all...

INCLUDE (CheckCXXCompilerFlag)

CHECK_CXX_COMPILER_FLAG (-O0 ESTAR_CXX_FLAG_O)
IF (ESTAR_CXX_FLAG_O)
  ADD_DEFINITIONS (-O0)
ENDIF (ESTAR_CXX_FLAG_O)

CHECK_CXX_COMPILER_FLAG (-g ESTAR_CXX_FLAG_g)
IF (ESTAR_CXX_FLAG_g)
  ADD_DEFINITIONS (-g)
ENDIF (ESTAR_CXX_FLAG_g)

CHECK_CXX_COMPILER_FLAG (-Wall ESTAR_CXX_FLAG_Wall)
IF (ESTAR_CXX_FLAG_Wall)
  ADD_DEFINITIONS (-Wall)
ENDIF (ESTAR_CXX_FLAG_Wall)

CHECK_CXX_COMPILER_FLAG (-Wno-long-double ESTAR_CXX_FLAG_Wnld)
IF (ESTAR_CXX_FLAG_Wnld)
  ADD_DEFINITIONS (-Wno-long-double)
ENDIF (ESTAR_CXX_FLAG_Wnld)

CHECK_CXX_COMPILER_FLAG (-Wundef ESTAR_CXX_FLAG_Wundef)
IF (ESTAR_CXX_FLAG_Wundef)
  ADD_DEFINITIONS (-Wundef)
ENDIF (ESTAR_CXX_FLAG_Wundef)


### Find required Boost libraries: Smart Pointers and Graph.

SET (PREFIX /usr/local          CACHE PATH "project installation prefix")
SET (BOOST_PATH ${PREFIX}/boost CACHE PATH "Boost installation prefix")

SET (CMAKE_REQUIRED_INCLUDES ${BOOST_PATH}/include ${PREFIX}/include)
INCLUDE_DIRECTORIES (${BOOST_PATH}/include ${PREFIX}/include)

INCLUDE (CheckIncludeFileCXX)

CHECK_INCLUDE_FILE_CXX (boost/shared_ptr.hpp ESTAR_HAVE_SHARED_PTR)
IF (${ESTAR_HAVE_SHARED_PTR})
ELSE (${ESTAR_HAVE_SHARED_PTR})
      MESSAGE(FATAL_ERROR "boost/shared_ptr.hpp not found")
ENDIF (${ESTAR_HAVE_SHARED_PTR})

CHECK_INCLUDE_FILE_CXX (boost/graph/adjacency_list.hpp ESTAR_HAVE_ADJLIST)
IF (${ESTAR_HAVE_ADJLIST})
ELSE (${ESTAR_HAVE_ADJLIST})
      MESSAGE(FATAL_ERROR "boost/graph/adjacency_list.hpp not found")
ENDIF (${ESTAR_HAVE_ADJLIST})



### Some OS-dependent flags.

IF (WIN32)
  MESSAGE ("Detected Microsoft Windows")
  ADD_DEFINITIONS (-DWIN32)
ELSE (WIN32)
  IF (APPLE)
    MESSAGE ("Detected Mac OS X")
    ADD_DEFINITIONS (-DOSX)
  ENDIF (APPLE)
  
  IF (CMAKE_SYSTEM_NAME MATCHES Linux)
    MESSAGE ("Detected Linux")
    ADD_DEFINITIONS (-DLINUX)
  ENDIF (CMAKE_SYSTEM_NAME MATCHES Linux)
ENDIF(WIN32)



SUBDIRS (estar gfx)
