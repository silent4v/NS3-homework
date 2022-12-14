set xlabel "Runtime (sec)"
set ylabel "RTT (ms)"
set xtics 30

# 0.5625 =  9/16
# 0.4285 =  9/21
# 0.28125 = 9/32
set size ratio 0.28125

set grid
set autoscale

# LinuxReno/TcpLinuxReno, NewReno/TcpNewReno, Vegas/TcpVegas
DATASET='LinuxReno/TcpLinuxReno';

set title DATASET." RTT"
# plot DATASET.'-flow0-rtt.data' title 'flow0' with lines, \
#      DATASET.'-flow1-rtt.data' title 'flow1' with lines, \
#      DATASET.'-flow2-rtt.data' title 'flow2' with lines

# Compare
plot 'LinuxReno/TcpLinuxReno-flow0-rtt.data' title 'LinuxReno' with lines, \
     'NewReno/TcpNewReno-flow0-rtt.data' title 'NewReno' with lines, \
     'Vegas/TcpVegas-flow0-rtt.data' title 'Vegas' with lines

