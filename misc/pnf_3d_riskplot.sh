#!/bin/bash

gnuplot <<EOF
set terminal fig
set output 'cool-riskplot.fig'
set hidden3d
set contour both
set view 38, 238
set style line 1 lt 2 lw 1
splot 'risk.data' w l notitle
EOF
fig2dev -L pdf -p foo cool-riskplot.fig cool-riskplot.pdf
