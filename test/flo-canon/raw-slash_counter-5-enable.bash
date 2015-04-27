#include "tempdir.bash"

cat >test.in <<EOF
reset = rst
T0 = add/5 Counter5::reg 1
Counter5::reg = reg reset T0
Counter5::io_count = out/5 Counter5::reg
EOF

cat >test.gold <<EOF
reset = rst'1
T0 = add'5 Counter5::reg 1'5
Counter5::reg = reg'5 reset T0
Counter5::io_count = out'5 Counter5::reg
EOF

#include "harness.bash"
