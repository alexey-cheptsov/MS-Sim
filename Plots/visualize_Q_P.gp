set datafile separator ';'
set key outside

set xlabel "Time, s."
set ylabel "Q(P)"

Q=system("echo $Q_INPUT")
P=system("echo $P_INPUT")

#https://subscription.packtpub.com/book/big_data_and_business_intelligence/9781849517249/1/ch01lvl1sec13/using-two-different-y-axes
set y2tics 0, 50 # start value, step
set ytics nomirror

plot P  using 5:6 axes x1y2 with lines title columnheader, Q using 5:6:xtic(log($1)) axes x1y1 with lines title columnheader