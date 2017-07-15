#!/usr/bin/env perl

use strict;
use warnings;

use Cwd qw( realpath );
use File::Basename;
use FindBin;
use lib "$FindBin::Bin/../contrib";

die("usage: MYSQL='mysql-command' $FindBin::Script Object.hpp\n") unless @ARGV;

my $MYSQL = $ENV{MYSQL} || 'mysql';

undef $/;

foreach my $filename (@ARGV) {
	open(IN, '<', $filename) || die("Can't open $filename: $!\n");
	my $in = <IN>;
	close(IN);

	if ( $in =~ m|^/\*(.+?)\*/|sm ) {
		my $sql = $1;
	
		$sql =~ s/create table (\S+)/Create table _tmp_$1/ios;
		my $table_name = $1;
		
		open(SQL, '|-', $MYSQL . ' 1>/dev/null') || die("Can't pipe to mySQL: $!\n");
		print SQL "drop table if exists _tmp_${table_name};\n";
		print SQL $sql;
		close(SQL);
		
		open(SQL, '|-', $MYSQL . ' > current-db.log') || die("Can't pipe to mySQL: $!\n");
		print SQL "select COLUMN_NAME,COLUMN_DEFAULT,IS_NULLABLE,COLUMN_TYPE,COLUMN_KEY,EXTRA from INFORMATION_SCHEMA.COLUMNS where TABLE_SCHEMA=database() and TABLE_NAME='${table_name}';\n";
		close(SQL);
		
		open(SQL, '|-', $MYSQL . ' > new-db.log') || die("Can't pipe to mySQL: $!\n");
		print SQL "select COLUMN_NAME,COLUMN_DEFAULT,IS_NULLABLE,COLUMN_TYPE,COLUMN_KEY,EXTRA from INFORMATION_SCHEMA.COLUMNS where TABLE_SCHEMA=database() and TABLE_NAME='_tmp_${table_name}';\n";
		close(SQL);
		
		open(SQL, '|-', $MYSQL . ' 1>/dev/null') || die("Can't pipe to mySQL: $!\n");
		print SQL "drop table if exists _tmp_${table_name};\n";
		close(SQL);

		my $diff = `diff current-db.log new-db.log`;
		unlink("current-db.log", "new-db.log");
		
		print "$filename:\n$diff" if $diff ne '';
	}
}	
