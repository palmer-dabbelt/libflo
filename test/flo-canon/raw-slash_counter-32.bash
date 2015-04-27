#include "tempdir.bash"

cat > test.in <<EOF
reset = rst
T0 = add/32 Counter32::reg 1
Counter32::reg__update = mux reset 0 T0
Counter32::reg = reg 1 Counter32::reg__update
Counter32::io_count = out/32 Counter32::reg
EOF

cat > test.gold <<EOF
reset = rst'1
T0 = add'32 Counter32::reg 1'32
Counter32::reg__update = mux'32 reset 0'32 T0
Counter32::reg = reg'32 1'1 Counter32::reg__update
Counter32::io_count = out'32 Counter32::reg
EOF

#include "harness.bash"
