#!/usr/bin/env perl

use strict;
use warnings;

$_ = @ARGV ? join(' ', @ARGV) : <>;

my $output;

for(my $i=0; $i<length($_); $i++) {
	$output .= chr( ord( substr($_, $i, 1) ) ^ ( ($i+1) % 4 ) );
}

# escape "s for easy copy&paste

$output =~ s/"/\\"/g;

print "Encoded with escaped quotes:\n$output\n";
