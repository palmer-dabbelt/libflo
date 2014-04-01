#include "tempdir.bash"

cat > test.in <<EOF
reset = rst
T0 = add'32 test::r 1
test::r__update = mux'32 reset 0 T0
test::r = reg'32 1 test::r__update
test::io_o = out'32 test::r
EOF

cat > test.gold <<EOF
reset = rst'1
T0 = add'32 test::r 1'32
test::r__update = mux'32 reset 0'32 T0
test::r = reg'32 1'32 test::r__update
test::io_o = out'32 test::r
EOF

#include "harness.bash"
