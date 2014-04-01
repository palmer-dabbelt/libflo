export LC_ALL="C"

if [[ "$TEST" == "" ]]
then
    TEST="test"
fi

#############################################################################
# Check if we need to generate a Flo file                                   #
#############################################################################
if test -f $TEST.scala
then
    cat $TEST.scala

    scalac $TEST.scala -classpath chisel.jar:.

    scala $SCALA_FLAGS -classpath chisel.jar:. $TEST \
        --debug --backend flo \
        || true

    cat $TEST.flo
    mv $TEST.flo $TEST.in
fi

#############################################################################
# Run the test without valgrind                                             #
#############################################################################
$PTEST_BINARY $TEST.in > $TEST.out

cat $TEST.out | sort > $TEST.out.sort
cat $TEST.gold | sort > $TEST.gold.sort

cat $TEST.out
cat $TEST.gold

diff -au $TEST.gold.sort $TEST.out.sort
out="$?"
if [[ "$out" != "0" ]]
then
    exit "$out"
fi

#############################################################################
# Run the test with valgrind                                                #
#############################################################################
if [[ "$(which valgrind)" == "" ]]
then
    exit 0
fi

if test ! -x `which valgrind`
then
    exit 0
fi

valgrind -q $PTEST_BINARY $TEST.in >$TEST.out 2>$TEST.valgrind
cat $TEST.valgrind

if [[ "$(cat $TEST.valgrind | wc -l)" != 0 ]]
then
    exit 1
fi

cat $TEST.out | sort > $TEST.out.sort
cat $TEST.gold | sort > $TEST.gold.sort

cat test.out
cat test.gold

diff -au $TEST.gold.sort $TEST.out.sort
out="$?"
if [[ "$out" != "0" ]]
then
    exit "$out"
fi
