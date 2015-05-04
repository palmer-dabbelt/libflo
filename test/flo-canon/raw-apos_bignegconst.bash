#include "tempdir.bash"

cat >test.in <<"EOF"
Decoder::inst = in'32
Decoder::reset = rst'1
Decoder::mask = mov'32 -33525633'32
Decoder::isAdd_0 = and'32 Decoder::mask Decoder::inst
Decoder::isAdd_1 = eq'32 Decoder::isAdd_0 51'32
Decoder::isAdd = out'1 Decoder::isAdd_1
EOF

cat >test.gold <<"EOF"
Decoder::inst = in'32
Decoder::reset = rst'1
Decoder::mask = mov'32 4261441663'32
Decoder::isAdd_0 = and'32 Decoder::mask Decoder::inst
Decoder::isAdd_1 = eq'32 Decoder::isAdd_0 51'32
Decoder::isAdd = out'1 Decoder::isAdd_1
EOF
