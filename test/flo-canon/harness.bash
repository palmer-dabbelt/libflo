export LC_ALL="C"

#############################################################################
# Run the test without valgrind                                             #
#############################################################################
$PTEST_BINARY test.in > test.out

cat test.out | sort > test.out.sort
cat test.gold | sort > test.gold.sort

cat test.out
cat test.gold

diff -au test.gold.sort test.out.sort
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

valgrind -q $PTEST_BINARY test.in >test.out 2>test.valgrind
cat test.valgrind

if [[ "$(cat test.valgrind | wc -l)" != 0 ]]
then
    exit 1
fi

cat test.out | sort > test.out.sort
cat test.gold | sort > test.gold.sort

cat test.out
cat test.gold

diff -au test.gold.sort test.out.sort
out="$?"
if [[ "$out" != "0" ]]
then
    exit "$out"
fi
