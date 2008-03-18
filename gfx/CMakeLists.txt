ADD_LIBRARY (gfx
             Mousehandler.cpp
             Subwindow.cpp
             Viewport.cpp
             wrap_glu.cpp)

INCLUDE_DIRECTORIES (.
                     ..
		     ${PREFIX}/include)
