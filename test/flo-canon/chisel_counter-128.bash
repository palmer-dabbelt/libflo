#include "tempdir.bash"
#include "chisel-jar.bash"

cat >test.scala <<EOF
import Chisel._

class test extends Module {
  val io = new Bundle {
    val o = UInt(OUTPUT, width = 128)
  }

  val r = Reg(init = UInt(0, width = 128))
  r := r + UInt(1)
  io.o := r
}

class tests(t: test) extends Tester(t) {
  var cycle = 0
  do {
    step(1)
    cycle += 1
  } while (cycle < 10)
}

object test {
  def main(args: Array[String]): Unit = {
    chiselMainTest(args, () => Module(new test())) { t => new tests(t) }
  }
}
EOF

cat >test.gold <<EOF
reset = rst/1
T0 = add/128 test::r 1
test::r__update = mux/128 reset 0 T0
test::r = reg/128 1 test::r__update
test::io_o = out/128 test::r
EOF

#include "harness.bash"
