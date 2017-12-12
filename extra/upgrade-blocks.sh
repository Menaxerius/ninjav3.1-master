#!/bin/sh

BURST="http://localhost:8125"

# make sure you have correct values in pool.cnf
MYSQL="mysql --defaults-file=pool.cnf --batch --skip-column-names"

if echo 'desc Blocks;' | ${MYSQL} | fgrep --silent block_id; then
	echo Blocks table already altered
else
	echo 'alter table Blocks add block_id bigint unsigned; alter table Blocks add tx_fees bigint unsigned;' | ${MYSQL}
fi	

# mass update from static file
${MYSQL} < block-updates.sql

for block_height in `echo 'select blockID from Blocks where block_id is null;' | ${MYSQL}`; do
	echo Updating block ${block_height}
	curl --silent --url "${BURST}/burst" --data "requestType=getBlock&height=${block_height}" | \
		perl -n -e '$fee = $1 if m/"totalFeeNQT":"(\d+)"/; $block_id = $1 if m/"block":"(\d+)"/; $height = $1 if m/"height":(\d+)/; printf "UPDATE Blocks set tx_fees=%d, block_id=%lu where blockID=%d;\n", $fee, $block_id, $height if $block_id' | \
		${MYSQL}
done
