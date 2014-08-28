#include "tempdir.bash"

cat >test.in <<EOF
reset = rst
T0 = add/16 Counter16::reg 1
Counter16::reg__update = mux reset 0 T0
Counter16::reg = reg 1 Counter16::reg__update
Counter16::io_count = out/16 Counter16::reg
EOF

cat >test.gold <<EOF
reset = rst'1
T0 = add'16 Counter16::reg 1'16
Counter16::reg__update = mux'16 reset 0'16 T0
Counter16::reg = reg'16 1'16 Counter16::reg__update
Counter16::io_count = out'16 Counter16::reg
EOF

#include "harness.bash"
