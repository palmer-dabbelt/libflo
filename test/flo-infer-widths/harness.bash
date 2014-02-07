$PTEST_BINARY test.in > test.out

cat test.out | sort > test.out.sort
cat test.gold | sort > test.gold.sort

diff -au test.gold.sort test.out.sort
exit $?

