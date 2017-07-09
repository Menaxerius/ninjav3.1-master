#!/bin/sh

rm -fr staging
rm -fr packaged.tar.xz

mkdir staging staging/output

cp INSTALL.txt staging/

build/contrib/DORM/bin/emit_object_SQL objects/*.hpp > staging/burstpool.sql

cp -a static staging/
cp output/lib* output/server build/lib/lib* build/contrib/DORM/lib/lib* staging/output/

(cd staging; tar -cvJf ../packaged.tar.xz *)

rm -fr staging
