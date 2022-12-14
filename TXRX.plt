set xlabel "Runtime (sec)"
set ylabel "RTT (ms)"
set xtics 30

# 0.5625 =  9/16
# 0.4285 =  9/21
# 0.28125 = 9/32
set size ratio 0.4285
set grid
set autoscale
set xr [0:300]
# LinuxReno/TcpLinuxReno, NewReno/TcpNewReno, Vegas/TcpVegas
DATASET='LinuxReno/TcpLinuxReno';

set title "TX/RX Compare"

set key at 301,3500000
set key font ",6"

# Compare
plot 'LinuxReno/TcpLinuxReno-flow0-next-tx.data' title 'LinuxReno-f1-TX' with lines, \
     'LinuxReno/TcpLinuxReno-flow1-next-tx.data' title 'LinuxReno-f2-TX' with lines, \
     'LinuxReno/TcpLinuxReno-flow2-next-tx.data' title 'LinuxReno-f3-TX' with lines, \
     'NewReno/TcpNewReno-flow0-next-tx.data' title 'NewReno-f1-TX' with lines, \
     'NewReno/TcpNewReno-flow1-next-tx.data' title 'NewReno-f2-TX' with lines, \
     'NewReno/TcpNewReno-flow2-next-tx.data' title 'NewReno-f3-TX' with lines, \
     'Vegas/TcpVegas-flow0-next-tx.data' title 'Vegas-f1-TX' with lines, \
     'Vegas/TcpVegas-flow1-next-tx.data' title 'Vegas-f2-TX' with lines, \
     'Vegas/TcpVegas-flow2-next-tx.data' title 'Vegas-f3-TX' with lines, \
     'LinuxReno/TcpLinuxReno-flow0-next-tx.data' title 'LinuxReno-f1-RX' with lines, \
     'LinuxReno/TcpLinuxReno-flow1-next-tx.data' title 'LinuxReno-f2-RX' with lines, \
     'LinuxReno/TcpLinuxReno-flow2-next-tx.data' title 'LinuxReno-f3-RX' with lines, \
     'NewReno/TcpNewReno-flow0-next-tx.data' title 'NewReno-f1-RX' with lines, \
     'NewReno/TcpNewReno-flow1-next-tx.data' title 'NewReno-f2-RX' with lines, \
     'NewReno/TcpNewReno-flow2-next-tx.data' title 'NewReno-f3-RX' with lines, \
     'Vegas/TcpVegas-flow0-next-tx.data' title 'Vegas-f1-RX' with lines, \
     'Vegas/TcpVegas-flow1-next-tx.data' title 'Vegas-f2-RX' with lines, \
     'Vegas/TcpVegas-flow2-next-tx.data' title 'Vegas-f3-RX' with lines

