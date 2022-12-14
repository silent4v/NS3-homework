set xlabel "Runtime (sec)"
set ylabel "Cwnd Size"
set xtics 10
set size ratio 0.5625
set grid
set autoscale

# LinuxReno/TcpLinuxReno, NewReno/TcpNewReno, Vegas/TcpVegas

# DATASET='Vegas/TcpVegas';
# set title DATASET." CWND"
# plot DATASET.'-flow0-cwnd.data' title 'flow0' with lines, \
#      DATASET.'-flow1-cwnd.data' title 'flow1' with lines, \
#      DATASET.'-flow2-cwnd.data' title 'flow2' with lines

# Compare
plot 'LinuxReno/TcpLinuxReno-flow0-cwnd.data' title 'LinuxReno' with lines, \
     'NewReno/TcpNewReno-flow0-cwnd.data' title 'NewReno' with lines, \
     'Vegas/TcpVegas-flow0-cwnd.data' title 'Vegas' with lines

