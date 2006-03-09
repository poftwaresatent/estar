#!/bin/bash

for file in `find estar gfx tools pnf -name '*pp'`
  do
#  echo ==================================================
#  echo $file
#  for header in \
#      MetaMousehandler.hpp \
#      Mousehandler.hpp \
#      Subwindow.hpp \
#      Viewport.hpp \
#      graphics.hpp \
#      wrap_gl.hpp wrap_glu.h wrap_glut.hpp
#    do
    BEFORE="cspace_vertex_t"
    AFTER="vertex_t"
    echo "cat $file | sed s:$BEFORE:$AFTER: > ....."
    cat $file | sed s:$BEFORE:$AFTER: > foo
    mv foo $file
#  done
done
