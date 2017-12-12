#!/usr/bin/env perl

use strict;
use warnings;

die("usage: $0 config.js requestType [request-arg [...]]\n") unless @ARGV >= 2;
our $CONFIG_FILENAME = shift(@ARGV);
our $REQUEST_TYPE = shift(@ARGV);

my @servers;

open(CONFIG, '<', $CONFIG_FILENAME) || die("Can't open '$CONFIG_FILENAME': $!\n");
while(<CONFIG>) {
	next unless m/burstServers/;

	# example entry:
	#         "burstServers": [ "http://127.0.0.1:8125", "https://mwallet.burst-team.us:8125", "https://wallet.burst-team.us:8127", "https://wallet.burst-team.us:8128" ],

	# chuck away leading and trailing crap 
	s/^.*\[//;
	s/\].*$//;
	chomp;

	# remove any whitespace and quotes
	tr[ "'	][]d;

	@servers = split(',', $_);
	last;
}
close(CONFIG);

foreach my $server (@servers) {
	my $args = join('&', '', @ARGV);
	my $url = "$server/burst";
	my $data = "requestType=$REQUEST_TYPE$args";
	print "$url?$data\n";
	system("curl --silent --connect-timeout 4 --url '$url' --data-ascii '$data' --header 'Content-Type: application/x-www-form-urlencoded'");
	print "\n\n";
}
