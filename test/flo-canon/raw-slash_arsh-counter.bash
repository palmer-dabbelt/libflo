#include "tempdir.bash"

cat >test.in <<EOF
reset = rst
T0 = add/32 ModCounter::reg 4
T1 = gte/32 ModCounter::reg 5
T2 = mux T1 0 T0
T3 = rsh/16 T2 0
T4 = arsh/32 T3 0
ModCounter::reg__update = mux reset 0 T4
ModCounter::reg = reg 1 ModCounter::reg__update
ModCounter::io_count = out/32 ModCounter::reg
EOF

cat >test.gold <<EOF
reset = rst'1
T0 = add'32 ModCounter::reg 4'32
T1 = gte'32 ModCounter::reg 5'32
T2 = mux'32 T1 0'32 T0
T3 = rsh'16 T2 0'1
T4 = arsh'32 T3 0'1
ModCounter::reg__update = mux'32 reset 0'32 T4
ModCounter::reg = reg'32 1'1 ModCounter::reg__update
ModCounter::io_count = out'32 ModCounter::reg
EOF

#include "harness.bash"
