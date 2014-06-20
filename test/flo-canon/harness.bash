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

cat $TEST.in

#############################################################################
# Run the test without valgrind                                             #
#############################################################################
$PTEST_BINARY $TEST.in > $TEST.out

cat $TEST.out | sort > $TEST.out.sort
if [[ "$NODIFF" != "true" ]]
then
    cat $TEST.gold | sort > $TEST.gold.sort
fi

cat $TEST.out
if [[ "$NODIFF" != "true" ]]
then
    cat $TEST.gold
fi

if [[ "$NODIFF" != "true" ]]
then
    diff -au $TEST.gold.sort $TEST.out.sort
    out="$?"
    if [[ "$out" != "0" ]]
    then
        exit "$out"
    fi
fi

# Run a second test, ensuring that libflo can parse its own output
$PTEST_BINARY $TEST.out > $TEST.out.out
cat $TEST.out.out | sort > $TEST.out.out.sort

diff -au $TEST.out.sort $TEST.out.out.sort
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

valgrind --log-file=$TEST.valgrind -q $PTEST_BINARY $TEST.in >$TEST.out
cat $TEST.valgrind

if [[ "$(cat $TEST.valgrind | wc -l)" != 0 ]]
then
    exit 1
fi

if [[ "$NODIFF" != "true" ]]
then
    diff -au $TEST.gold.sort $TEST.out.sort
    out="$?"
    if [[ "$out" != "0" ]]
    then
        exit "$out"
    fi
fi

# Run a second test, ensuring that libflo can parse its own output
valgrind --log-file=$TEST.out.valgrind -q $PTEST_BINARY $TEST.out >$TEST.out.out
cat $TEST.out.valgrind

if [[ "$(cat $TEST.out.valgrind | wc -l)" != 0 ]]
then
    exit 1
fi

cat $TEST.out.out | sort > $TEST.out.out.sort

diff -au $TEST.out.sort $TEST.out.out.sort
out="$?"
if [[ "$out" != "0" ]]
then
    exit "$out"
fi
