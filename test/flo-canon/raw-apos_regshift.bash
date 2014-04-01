#include "tempdir.bash"

cat >test.in <<EOF
test::io_in = in'16
T0 = lsh test::io_in 5'3
T1 = cat'21 0'11 T0
test::io_out_const = out'32 T1
T2 = rsh'5 test::io_in 0'1
test::in = mov T2
T3 = lsh test::io_in test::in
T4 = rsh'32 T3 0'1
test::io_out_var = out'32 T4
EOF

cat >test.gold <<EOF
test::io_in = in'16
T0 = lsh'21 test::io_in 5'3
T1 = cat'21 0'11 T0
test::io_out_const = out'32 T1
T2 = rsh'5 test::io_in 0'1
test::in = mov'5 T2
T3 = lsh'47 test::io_in test::in
T4 = rsh'32 T3 0'1
test::io_out_var = out'32 T4
EOF

#include "harness.bash"
