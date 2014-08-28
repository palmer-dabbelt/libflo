#include "tempdir.bash"

cat >test.in <<EOF
Fib::io_ok = out/1 Fib::valid
Fib::io_in = out/32 Fib::range
reset = rst
T0 = add/32 Fib::cycle 1
T1 = gte/32 Fib::index Fib::range
T2 = eq Fib::valid 0
T3 = and T2 T1
T4 = mux T3 T0 Fib::cycle
Fib::cycle__update = mux reset 2 T4
Fib::cycle = reg 1 Fib::cycle__update
T5 = not/1 T2
T6 = mux T5 Fib::cycle Fib::range
Fib::range__update = mux reset 0 T6
Fib::range = reg 1 Fib::range__update
T7 = add/32 Fib::index 1
T8 = mux T2 T7 Fib::index
T9 = mux T5 1 T8
Fib::index__update = mux reset 0 T9
Fib::index = reg 1 Fib::index__update
T10 = mux T3 1 Fib::valid
T11 = mux T5 0 T10
Fib::valid__update = mux reset 1 T11
Fib::valid = reg 1 Fib::valid__update
T12 = add/32 Fib::sum0 Fib::sum1
T13 = mux T2 T12 Fib::sum1
T14 = mux T5 1 T13
Fib::sum1__update = mux reset 0 T14
Fib::sum1 = reg 1 Fib::sum1__update
T15 = mux T2 Fib::sum1 Fib::sum0
T16 = mux T5 0 T15
Fib::sum0__update = mux reset 0 T16
Fib::sum0 = reg 1 Fib::sum0__update
Fib::io_ot = out/32 Fib::sum0
EOF

cat >test.gold <<EOF
Fib::io_ok = out'1 Fib::valid
Fib::io_in = out'32 Fib::range
reset = rst'1
T0 = add'32 Fib::cycle 1'32
T1 = gte'32 Fib::index Fib::range
T2 = eq'1 Fib::valid 0'1
T3 = and'1 T2 T1
T4 = mux'32 T3 T0 Fib::cycle
Fib::cycle__update = mux'32 reset 2'32 T4
Fib::cycle = reg'32 1'32 Fib::cycle__update
T5 = not'1 T2
T6 = mux'32 T5 Fib::cycle Fib::range
Fib::range__update = mux'32 reset 0'32 T6
Fib::range = reg'32 1'32 Fib::range__update
T7 = add'32 Fib::index 1'32
T8 = mux'32 T2 T7 Fib::index
T9 = mux'32 T5 1'32 T8
Fib::index__update = mux'32 reset 0'32 T9
Fib::index = reg'32 1'32 Fib::index__update
T10 = mux'1 T3 1'1 Fib::valid
T11 = mux'1 T5 0'1 T10
Fib::valid__update = mux'1 reset 1'1 T11
Fib::valid = reg'1 1'1 Fib::valid__update
T12 = add'32 Fib::sum0 Fib::sum1
T13 = mux'32 T2 T12 Fib::sum1
T14 = mux'32 T5 1'32 T13
Fib::sum1__update = mux'32 reset 0'32 T14
Fib::sum1 = reg'32 1'32 Fib::sum1__update
T15 = mux'32 T2 Fib::sum1 Fib::sum0
T16 = mux'32 T5 0'32 T15
Fib::sum0__update = mux'32 reset 0'32 T16
Fib::sum0 = reg'32 1'32 Fib::sum0__update
Fib::io_ot = out'32 Fib::sum0
EOF

#include "harness.bash"
