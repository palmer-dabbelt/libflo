#include "tempdir.bash"

cat >test.in <<EOF
test::io_in = in/8
test::io_out = out/8 test::io_in
EOF

cat >test.gold <<EOF
Depth: 2
Width: 1
EOF

#include "harness.bash"
