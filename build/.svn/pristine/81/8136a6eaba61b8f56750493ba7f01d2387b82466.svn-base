#/bin/sh

s1=`./dehex.pl $1 | sha256 -q`
r1=`./dehex.pl $s1 | rmd160 -q`

prefix=`echo -n 00$r1`

s2=`./dehex.pl $prefix | sha256 -q`
s3=`./dehex.pl $s2 | sha256 -q`

suffix=`echo -n $s3 | cut -c1-8`

./test-base58 $prefix$suffix
