#include "tempdir.bash"

cat >test.in <<EOF
reset = rst
test::io_i = in/1
T0 = cat/1 0 test::io_i
T1 = not/1 test::io_i
T2 = mux/2 T1 2 T0
test::r__update = mux/2 reset 0 T2
test::r = reg/2 1 test::r__update
test::io_o = out/2 test::r
EOF

cat >test.gold <<EOF
reset = rst/1
test::io_i = in/1
T0 = cat/1 0 test::io_i
T1 = not/1 test::io_i
T2 = mux/2 T1 2 T0
test::r__update = mux/2 reset 0 T2
test::r = reg/2 1 test::r__update
test::io_o = out/2 test::r
EOF

#include "harness.bash"
