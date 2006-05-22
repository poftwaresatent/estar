#!/bin/bash

gnuplot -persist <<EOF
set term table
set out 'plot_pnf.table'
set cntrparam bspline
set cntrparam levels increment 0,1,30
set contour
unset surface
splot 'pnf_value.data'
set term fig
set output 'plot_pnf.fig'
#set term x11
set title 'PNF result (contour)'
set size ratio -1
plot 'plot_pnf.table' w l notitle
EOF
