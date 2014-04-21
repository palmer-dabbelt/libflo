#include "tempdir.bash"

cat >test.in <<EOF
test::io_in = in/8
test::io_out = out/8 test::io_in
test::io_in1 = in/8
test::io_out = out/8 test::io_in1
EOF

cat >test.gold <<EOF
Depth: 2
Width: 2
EOF

#include "harness.bash"
