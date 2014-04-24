#include "tempdir.bash"

cat >test.in <<EOF
test::i = in'8
T0 = eq'8 test::i -1'8
test::o = out'1 T0
EOF

cat >test.gold <<EOF
test::i = in'8
T0 = eq'8 test::i 255'8
test::o = out'1 T0
EOF

#include "harness.bash"
