#include "tempdir.bash"

cat >test.in <<EOF
test::io_in = in'8
T0 = add'8 test::io_in 1'2
test::io_out = out'8 T0
EOF

cat >test.gold <<EOF
test::io_in = in'8
T0 = add'8 test::io_in 1'8
test::io_out = out'8 T0
EOF

export LIBFLO__ALLOW_BAD_WIDTHS="true"

#include "harness.bash"
