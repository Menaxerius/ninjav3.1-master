#!/usr/bin/env perl

use strict;
use warnings;

die("usage: $0 some/handler.cpp\n") unless @ARGV;

my ($pathname) = @ARGV;

die("Can't find $pathname\n") unless -f $pathname;

my @paths = split( qr"/", $pathname );
my $filename = pop(@paths);
my $class = substr($filename, 0, -4);
my $hpp_guard_filename = $filename;
$hpp_guard_filename =~ tr/[a-z]./[A-Z]_/;

my $hpp_guard = join('__', map { uc($_) } @paths, uc($hpp_guard_filename) );

my $namespace_openings;
for(my $i = 0; $i < @paths; ++$i) {
	$namespace_openings .= "\t" x ($i + 1);
	$namespace_openings .= "namespace $paths[$i] {\n";
}

my $namespace_closures;
for(my $i=@paths - 1; $i >= 0; --$i) {
	$namespace_closures .= "\n" . "\t" x ($i + 1);
	$namespace_closures .= "} // $paths[$i] namespace";
}

undef $/;
print <<"__HPP__";
#ifndef HANDLERS__${hpp_guard}
#define HANDLERS__${hpp_guard}

#include "Handler.hpp"


namespace Handlers {
${namespace_openings}
		class ${class}: public Handler {
			public:
				virtual int process( struct MHD_Connection *connection, Request *req, Response *resp );
		};
${namespace_closures}
} // Handlers namespace


#endif
__HPP__
