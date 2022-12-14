set xlabel "Runtime (sec)"
set ylabel "CWND/SSTH Size"
set xtics 30
set size ratio 0.5625
set autoscale

# LinuxReno/TcpLinuxReno, NewReno/TcpNewReno, Vegas/TcpVegas

DATASET='Vegas/TcpVegas';
set title DATASET." Detail"
set yr [0:5000]
plot DATASET.'-flow0-cwnd.data' title 'CWND' with lines lc "red", \
     DATASET.'-flow0-ssth.data' title 'SSTH' with lines lc "blue", \
     DATASET.'-drop' title 'DropRX' with impulses lw 0.5 lc "#006000"
