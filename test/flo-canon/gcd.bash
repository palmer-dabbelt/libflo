#include "tempdir.bash"

cat >test.in <<EOF
reset = rst
GCDWrapper::gcd::reset = mov reset
T0 = add/32 GCDWrapper::cnt 1
GCDWrapper::gcd::io_bin = mov T0
T1 = not/1 GCDWrapper::gcd::busy
T2 = and T1 1
T3 = mux T2 GCDWrapper::gcd::io_bin GCDWrapper::gcd::b
T4 = sub/32 GCDWrapper::gcd::b GCDWrapper::gcd::a
T5 = lt/32 GCDWrapper::gcd::b GCDWrapper::gcd::a
T6 = eq GCDWrapper::gcd::a GCDWrapper::gcd::b
T7 = or T6 T5
T8 = not/1 T7
T9 = not/1 GCDWrapper::gcd::done
T10 = and GCDWrapper::gcd::busy T9
T11 = and T10 T8
T12 = mux T11 T4 T3
GCDWrapper::gcd::b__update = mux GCDWrapper::gcd::reset 0 T12
GCDWrapper::gcd::b = reg T12 GCDWrapper::gcd::b__update
T13 = mux T2 0 GCDWrapper::gcd::done
T14 = and T10 T6
T15 = mux T14 1 T13
T16 = and GCDWrapper::gcd::busy GCDWrapper::gcd::done
T17 = mux T16 0 T15
GCDWrapper::gcd::done__update = mux GCDWrapper::gcd::reset 0 T17
GCDWrapper::gcd::done = reg T17 GCDWrapper::gcd::done__update
T18 = mux T2 1 GCDWrapper::gcd::busy
T19 = mux T16 0 T18
GCDWrapper::gcd::busy__update = mux GCDWrapper::gcd::reset 0 T19
GCDWrapper::gcd::busy = reg T19 GCDWrapper::gcd::busy__update
GCDWrapper::gcd::io_ain = mov GCDWrapper::cnt
T20 = mux T2 GCDWrapper::gcd::io_ain GCDWrapper::gcd::a
T21 = sub/32 GCDWrapper::gcd::a GCDWrapper::gcd::b
T22 = not/1 T6
T23 = and T22 T5
T24 = and T10 T23
T25 = mux T24 T21 T20
GCDWrapper::gcd::a__update = mux GCDWrapper::gcd::reset 0 T25
GCDWrapper::gcd::a = reg T25 GCDWrapper::gcd::a__update
GCDWrapper::gcd::io_vld = mov T16
GCDWrapper::io_valid = out/1 GCDWrapper::gcd::io_vld
T26 = add/32 GCDWrapper::cnt 1
GCDWrapper::cnt__update = mux reset 0 T26
GCDWrapper::cnt = reg T26 GCDWrapper::cnt__update
GCDWrapper::gcd::io_out = mov GCDWrapper::gcd::a
GCDWrapper::io_data = out/32 GCDWrapper::gcd::io_out
EOF

cat >test.gold <<EOF
reset = rst/1
GCDWrapper::gcd::reset = mov/1 reset
T0 = add/32 GCDWrapper::cnt 1
GCDWrapper::gcd::io_bin = mov/32 T0
T1 = not/1 GCDWrapper::gcd::busy
T2 = and/1 T1 1
T3 = mux/32 T2 GCDWrapper::gcd::io_bin GCDWrapper::gcd::b
T4 = sub/32 GCDWrapper::gcd::b GCDWrapper::gcd::a
T5 = lt/32 GCDWrapper::gcd::b GCDWrapper::gcd::a
T6 = eq/32 GCDWrapper::gcd::a GCDWrapper::gcd::b
T7 = or/1 T6 T5
T8 = not/1 T7
T9 = not/1 GCDWrapper::gcd::done
T10 = and/1 GCDWrapper::gcd::busy T9
T11 = and/1 T10 T8
T12 = mux/32 T11 T4 T3
GCDWrapper::gcd::b__update = mux/32 GCDWrapper::gcd::reset 0 T12
GCDWrapper::gcd::b = reg/32 T12 GCDWrapper::gcd::b__update
T13 = mux/1 T2 0 GCDWrapper::gcd::done
T14 = and/1 T10 T6
T15 = mux/1 T14 1 T13
T16 = and/1 GCDWrapper::gcd::busy GCDWrapper::gcd::done
T17 = mux/1 T16 0 T15
GCDWrapper::gcd::done__update = mux/1 GCDWrapper::gcd::reset 0 T17
GCDWrapper::gcd::done = reg/1 T17 GCDWrapper::gcd::done__update
T18 = mux/1 T2 1 GCDWrapper::gcd::busy
T19 = mux/1 T16 0 T18
GCDWrapper::gcd::busy__update = mux/1 GCDWrapper::gcd::reset 0 T19
GCDWrapper::gcd::busy = reg/1 T19 GCDWrapper::gcd::busy__update
GCDWrapper::gcd::io_ain = mov/32 GCDWrapper::cnt
T20 = mux/32 T2 GCDWrapper::gcd::io_ain GCDWrapper::gcd::a
T21 = sub/32 GCDWrapper::gcd::a GCDWrapper::gcd::b
T22 = not/1 T6
T23 = and/1 T22 T5
T24 = and/1 T10 T23
T25 = mux/32 T24 T21 T20
GCDWrapper::gcd::a__update = mux/32 GCDWrapper::gcd::reset 0 T25
GCDWrapper::gcd::a = reg/32 T25 GCDWrapper::gcd::a__update
GCDWrapper::gcd::io_vld = mov/1 T16
GCDWrapper::io_valid = out/1 GCDWrapper::gcd::io_vld
T26 = add/32 GCDWrapper::cnt 1
GCDWrapper::cnt__update = mux/32 reset 0 T26
GCDWrapper::cnt = reg/32 T26 GCDWrapper::cnt__update
GCDWrapper::gcd::io_out = mov/32 GCDWrapper::gcd::a
GCDWrapper::io_data = out/32 GCDWrapper::gcd::io_out
EOF

#include "harness.bash"