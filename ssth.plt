set xlabel "Runtime (sec)"
set ylabel "CWND/SSTH Value"
set title "SSTH Compare"
set xtics 30
set size ratio 0.5625
set grid
set autoscale

# LinuxReno/TcpLinuxReno, NewReno/TcpNewReno, Vegas/TcpVegas
# DATASET='Vegas/TcpVegas';
# set title DATASET." CWND"
# plot 'LinuxReno/TcpLinuxReno-flow0-cwnd.data' title 'CWND' with lines, \
#      'LinuxReno/TcpLinuxReno-flow0-ssth.data' title 'SSTH' with lines, \
#      "linuxReno-drop" title 'DropRX' with impulses


# Compare
plot 'LinuxReno/TcpLinuxReno-flow0-ssth.data' title 'LinuxReno SSTH' with lines lc "red", \
     'LinuxReno/TcpLinuxReno-flow0-cwnd.data' title 'LinuxReno CWND' with lines lc "#ff00ff", \
     'NewReno/TcpNewReno-flow0-ssth.data' title 'NewReno SSTH' with lines lc "#228b22", \
     'NewReno/TcpNewReno-flow0-cwnd.data' title 'NewReno CWND ' with lines lc "#556b2f", \
     'Vegas/TcpVegas-flow0-ssth.data' title 'Vegas SSTH' with lines lc "blue", \
     'Vegas/TcpVegas-flow0-cwnd.data' title 'Vegas CWND' with lines lc "#00bfff"
