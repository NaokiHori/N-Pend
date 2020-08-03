
set xlabel 'time'
set ylabel 'energy'

plot \
  'energy.txt' u 2:3       t 'kinetic'   lc '#FF0000' lw 3 w l, \
  'energy.txt' u 2:4       t 'poteitial' lc '#33AA00' lw 3 w l, \
  'energy.txt' u 2:($3+$4) t 'total'     lc '#0000FF' lw 3 w l
