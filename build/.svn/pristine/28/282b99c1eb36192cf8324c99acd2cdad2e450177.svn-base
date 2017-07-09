#!/usr/bin/env perl

use strict;
use warnings;

use Cwd qw( realpath );
use File::Basename;
use File::Spec;
use FindBin;
use lib "$FindBin::Bin/../contrib";

use Getopt::Std;

my %opt;
getopts( 'd:', \%opt );

die("usage: $FindBin::Script template.ct\n") unless @ARGV == 1;

$opt{d} ||= '.';

my $arg_filename = $ARGV[0];

my ($filename, $dirname, $suffix) = fileparse( $arg_filename, '.ct' );
$dirname =~ s|^\./||;
mkdir( "$opt{d}/$dirname" ) unless -d "$opt{d}/$dirname";

my $prefix = $dirname . $filename;
my $class = $filename;
my $full_class = $prefix;
$full_class =~ s|/|::|g;
my $define_name = $prefix;
$define_name =~ s|/|__|g;
my @namespaces = split( /\//, $dirname );
my %includes;

undef $/; 
my $template = <>;

my %declarations;
my @c_includes;

# INCLUDEs - included NOW
do_includes( $dirname, $arg_filename, \$template );

# extra C/C++ includes
$template =~ s/ <% \# include \s+ (.+?) \s* %> / push(@c_includes, $1); '' /gsex;

# data declarations
$template =~ s/ <% ! \s* (.+?) \b (\w+) \s* (= \s* [^%]+)? %> / $declarations{$2} = [$1, $3 || '']; '' /gsex;

# HTML-escape
$template =~ s/ <% h= (.+?) %> /<% { std::stringstream _tmp; _tmp << $1; _output << Template::HTML_escape( _tmp.str() ); } %>/gsx;

# HTML-escape with <BR>s
$template =~ s/ <% hbr= (.+?) %> /<% { std::stringstream _tmp; _tmp << $1; _output << Template::HTML_escape_with_BRs( _tmp.str() ); } %>/gsx;

# JavaScript-escape
$template =~ s/ <% j= (.+?) %> /<% { std::stringstream _tmp; _tmp << $1; _output << Template::JavaScript_escape( _tmp.str() ); } %>/gsx;

# URL-escape
$template =~ s/ <% u= (.+?) %> /<% { std::stringstream _tmp; _tmp << $1; _output << Template::URL_escape( _tmp.str() ); } %>/gsx;

# HTML then JavaScript escape
$template =~ s/ <% jh= (.+?) %> /<% { std::stringstream _tmp; _tmp << $1; _output << Template::JavaScript_escape( Template::HTML_escape( _tmp.str() ) ); } %>/gsx;
$template =~ s/ <% jhbr= (.+?) %> /<% { std::stringstream _tmp; _tmp << $1; _output << Template::JavaScript_escape( Template::HTML_escape_with_BRs( _tmp.str() ) ); } %>/gsx;

# simple "print"
$template =~ s/ <% = (.+?) %> /<% _output << $1; %>/gsx;

# encapsulate the rest
my $raw;
$template =~ s/ (^|%>) (.*?) (<%|$) / $raw = $2; $raw =~ s|\\|\\\\|g; $raw =~ s|\"|\\\"|g; $raw =~ s|\n|\\n\"\n\t\t<< \"|g; qq{;\t_output << "$raw"; } /gsex;

# bit of optimisation
$template =~ s/<< ""//g;
$template =~ s/_output\s*;//g;

# --- CPP file --- #

my $cpp = '';

foreach my $c_include (@c_includes) {
	$cpp .= "#include $c_include\n";
}

$cpp .= <<"__CPP__";
#include "$prefix.hxx"

#include <sstream>

using std::string;

std::string Templates::$full_class\::render() {
	std::stringstream _output;
	
#line 1 "$dirname$filename$suffix"
__CPP__
 
$cpp .= $template;

$cpp .= <<"__CPP__";

	return _output.str();
};

__CPP__


open(CXX, '>', "$opt{d}/$prefix.cxx");
print CXX $cpp;
close(CXX);

# --- HXX file --- #

my $HXX = <<"__HXX__";
#ifndef __TEMPLATE__$define_name
#define __TEMPLATE__$define_name

#include "Template.hpp"

// include config.hpp to allow support for extra features
#include "config.hpp"

__HXX__

# handle includes for data types
foreach my $decl_info (values %declarations) {
	$HXX .= type_include( $decl_info->[0] );
}

$HXX .= <<"__HXX__";

using std::string;

namespace Templates {
__HXX__

foreach my $namespace (@namespaces) {
	$HXX .= "namespace $namespace {\n";
}

$HXX .= <<"__HXX__";

	class $class\: public Template {
		public:
			#ifdef TEMPLATE_DECL
				TEMPLATE_DECL
			#endif

			$class( Request *incoming_req ): Template(incoming_req) { 
				#ifdef TEMPLATE_INIT
					TEMPLATE_INIT
				#endif
			};
__HXX__

while( my ($name, $decl_info) = each %declarations ) {
	$HXX .= "\t\t\t$decl_info->[0]$name $decl_info->[1];\n";
}

$HXX .= <<'__HXX__';

			std::string render();
	};
};
__HXX__

foreach my $namespace (@namespaces) {
	$HXX .= "};\n";
}

$HXX .= <<"__HXX__";

#endif
__HXX__


open(HXX, '>', "$opt{d}/$prefix.hxx");
print HXX $HXX;
close(HXX);


# --- makefile --- #

my $MK = "$opt{d}/$prefix.cxx: ";
$MK .= join(' ', map { s|^(.*)\.ct$|$opt{d}/$1.cxx|; $_ } keys %includes) . ' ';
# $MK .= "$opt{d}/$prefix.cxx\n";
$MK .= "\n";

open(MK, '>', "$opt{d}/$prefix.mk");
print MK $MK;
close(MK);





# --- #

sub do_includes {
	my ($root_dirname, $pathname, $template_ref) = @_;

	my ($filename, $dirname, $suffix) = fileparse( $pathname, '.ct' );
	
	my $start = length( $$template_ref );
	while( ($start = rindex($$template_ref, '<%include ', $start)) != -1 ) {
		my $line = 1 + substr($$template_ref, 0, $start) =~ tr|\n|\n|;
	
		my $end = index($$template_ref, '%>', $start);
		die("Can't find closing '%>' for include at $pathname line $line.\n") if $end == -1;
		
		my $includename = substr($$template_ref, $start + length('<%include '), $end - ($start + length('<%include ')));
		# convert dirname + filename to full path
		$includename = realpath( $dirname . $includename );
		# but make it relative to "root" dirname
		$includename = File::Spec->abs2rel( $includename );
		
		substr($$template_ref, $start, $end - $start + 2) = include($root_dirname, $includename) . qq{<%\n#line $line "$pathname"\n%>};
	}
} 
	

sub include {
	my ($root_dirname, $pathname) = @_;

	unless( exists $includes{ $pathname } ) {
		open(INC, $pathname) || die("Can't include '$pathname': $!\n");
		my $include = qq{<%\n#line 1 "$pathname"\n%>} . join('', <INC>);
		close(INC);

		# check for nested includes
		do_includes( $root_dirname, $pathname, \$include );
		
		$includes{ $pathname } = $include;
	}
	
	return $includes{ $pathname };
}




sub type_include {
	my ($type) = @_;
	
	my $output = '';
	
	$type =~ s/ [\t \*]+ $ //ox;

	# don't include basic types
	return $output if index($type, '<') == -1 && $type =~ m/ (^|\s) (long|time_t|int|float|double|bool|char|uint) /ox;

	if ($type =~ m/ (^|\s) (?: std:: )? (vector|map|list|string) ( < ( .+ ) > )? /ox) {
		$output .= qq{#include <$2>\n};
		$output .= qq{using std::$2;\n};
		
		# recursive with template
		if ( my $template = $4 ) {
			foreach my $template_type ( split( ',', $template ) ) {
				$template_type =~ s|^\s*(.*?)\s*$|$1|;
				$output .= type_include( $template_type );
			}  
		}
	} else {
		# ugh nasty type to filename conversion
		my $orig_type = $type;
		
		$type =~ s|(.)([A-Z][a-z])|$1/$2|g;
		$type =~ s|([a-z])(?!ID)([A-Z])|$1/$2|g;
	
		if ($type ne $orig_type) {
			$output .= "// was: $orig_type\n";
		}
		
		$output .= qq{#include "$type.hpp"\n};
	}
	
	return $output;
} 

	
