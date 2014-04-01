#include "tempdir.bash"

cat >test.in <<EOF
reset = rst
T0 = add/5 Counter5::reg 1
Counter5::reg__update = mux reset 0 T0
Counter5::reg = reg T0 Counter5::reg__update
Counter5::io_count = out/5 Counter5::reg
EOF

cat >test.gold <<EOF
reset = rst'1
T0 = add'5 Counter5::reg 1'5
Counter5::reg__update = mux'5 reset 0'5 T0
Counter5::reg = reg'5 T0 Counter5::reg__update
Counter5::io_count = out'5 Counter5::reg
EOF

#include "harness.bash"
