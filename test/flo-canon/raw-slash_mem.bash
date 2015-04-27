#include "tempdir.bash"

cat >test.in <<EOF
test::io_i = in/8
T0 = xor/8 test::r test::io_i
test::r = reg/8 1 T0
T1 = cat/8 0 test::r
test::io_r = in/1
T2 = not/1 test::io_r
T3 = wr/32 T2 test::mem test::io_i T1
test::mem = mem/32 256
T4 = rd/32 1 test::mem test::io_i
T5 = mux/32 test::io_r T4 0
test::io_o = out/32 T5
EOF

cat >test.gold <<EOF
test::io_i = in'8
T0 = xor'8 test::r test::io_i
test::r = reg'8 1'1 T0
T1 = cat'8 0'24 test::r
test::io_r = in'1
T2 = not'1 test::io_r
T3 = wr'32 T2 test::mem test::io_i T1
test::mem = mem'32 256
T4 = rd'32 1'1 test::mem test::io_i
T5 = mux'32 test::io_r T4 0'32
test::io_o = out'32 T5
EOF

#include "harness.bash"
