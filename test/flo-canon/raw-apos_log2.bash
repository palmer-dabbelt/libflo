#include "tempdir.bash"

cat >test.in <<EOF
Torture::io_in0 = in'3
T0 = log2'2 Torture::io_in0

T1 = rsh'1 T0 0'1
Torture::io_out0 = out'1 T1
EOF

cat >test.gold <<EOF
Torture::io_in0 = in'3
T0 = log2'2 Torture::io_in0
T1 = rsh'1 T0 0'1
Torture::io_out0 = out'1 T1
EOF

#include "harness.bash"
