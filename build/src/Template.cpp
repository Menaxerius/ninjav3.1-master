#include "Template.hpp"

#include <sstream>
#include <cmath>


using std::string;

#define URL_VALID "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789"

sigfig_suffixes_t byte_suffixes;
sigfig_suffixes_t SI_suffixes;


Template::~Template() {
}


string Template::HTML_escape( string str ) {
	str = find_and_replace( str, "&", "&amp;" );
	str = find_and_replace( str, "<", "&lt;" );
	str = find_and_replace( str, ">", "&gt;" );
	str = find_and_replace( str, "'", "&apos;" );
	str = find_and_replace( str, "\"", "&quot;" );
	
	return str;
}


string Template::HTML_escape_with_BRs( string str ) {
	str = HTML_escape( str );
	
	str = find_and_replace( str, "\n\r", "<br>\n\r" );
	str = find_and_replace( str, "\r\n", "<br>\r\n" );
	str = find_and_replace( str, "\n", "<br>\n" );
	str = find_and_replace( str, "\r", "<br>\r" );
	
	return str;
}


string Template::JavaScript_escape( string str ) {
	str = find_and_replace( str, "\\", "\\u005C" );
	str = find_and_replace( str, "'", "\\u0027" );
	str = find_and_replace( str, "<", "\\u003C" );
	
	str = find_and_replace( str, "\n", "\\u000A" ); 
	str = find_and_replace( str, "\r", "\\u000D" ); 

	return str;
}


string Template::URL_escape( string str ) {
	std::size_t pos = str.find_last_not_of( URL_VALID );
	
	while( pos != std::string::npos ) {
		std::stringstream replacement;
		replacement << "%" << std::hex;
		replacement.fill( '0' );
		replacement.width( 2 );
		replacement << (int) str[pos];
	
		str.replace( pos, 1, replacement.str() );
		
		pos = str.find_last_not_of( URL_VALID, pos - 1 );
	}

	return str; 
}


string Template::find_and_replace( string str, string find, string replace ) {
	std::size_t find_len = find.length();
	std::size_t replace_len = replace.length();
	std::size_t pos_inc = replace_len - find_len + 1; 
	
	std::size_t pos = str.find( find );
	
	while( pos != std::string::npos ) {
		str.replace( pos, find_len, replace );
		
		pos = str.find( find, pos + pos_inc );
	}

	return str; 
}


string Template::JS_quote( std::string str ) {
	string quoted = "'";
	quoted += str;
	quoted += "'";
	
	return quoted;
}


std::list<string> Template::map( string_string_func_t fn, std::list<string> &items ) {
	std::list<string> mapped;
	
	for( std::list<string>::iterator it = items.begin(); it != items.end(); it++ ) {
		mapped.push_back( fn( *it ) );
	}
	
	return mapped;
}


string Template::join( string delimiter, std::list<string> items ) {
	return join( delimiter.c_str(), items );
}


string Template::join( const char delimiter[], std::list<string> items ) {
	string joined;

	for( std::list<string>::iterator it = items.begin(); it != items.end(); it++ ) {
		if (it == items.begin()) {
			// first one
			joined = *it;
		} else {
			// more
			joined += delimiter;
			joined += *it;
		}
	}
	
	return joined;
}


std::vector<std::string> Template::split( const char delimiter, const std::string str ) {
	std::vector<std::string> results;

	std::string::size_type start = 0;
	std::string::size_type end;

	while( ( end = str.find( delimiter, start) ) != std::string::npos ) {
		results.push_back( str.substr(start, end-1) );
		start = end + 1;
	}

	// any left after final delimiter?
	if ( start < str.length() )
		results.push_back( str.substr(start) );

	return results;
}


string Template::pretty_bytesize( long long int value, char sigfig ) {
	if (! byte_suffixes.size() ) {
		// first-time init
		byte_suffixes[ pow(2.0, 10) ] = "K";
		byte_suffixes[ pow(2.0, 20) ] = "M";
		byte_suffixes[ pow(2.0, 30) ] = "G";
		byte_suffixes[ pow(2.0, 40) ] = "T";
	}

	return sigfig_suffix(value, &byte_suffixes, sigfig );
}


string Template::pretty_size( long long int value, char sigfig ) {
	if (! SI_suffixes.size() ) {
		// first-time init
		SI_suffixes[ pow(10.0, 3) ] = "K";
		SI_suffixes[ pow(10.0, 6) ] = "M";
		SI_suffixes[ pow(10.0, 9) ] = "G";
		SI_suffixes[ pow(10.0, 12) ] = "T";
	}

	return sigfig_suffix(value, &SI_suffixes, sigfig );
}


string Template::sigfig_suffix( long long int value, sigfig_suffixes_t *suffixes, char sigfig ) {
	for( sigfig_suffix_tuple suffix :  *suffixes ) {
		long long int multiplier = suffix.first;

		if (multiplier) {
			long long int pre_point = value / multiplier;

			std::ostringstream pre_point_ss;
			pre_point_ss << pre_point;
			std::string pre_point_s = pre_point_ss.str();

			if ( pre_point_s.length() <= sigfig ) {
				value -= pre_point * multiplier;
			
				long long int post_point = value * pow( 10.0, sigfig ) / multiplier;
				std::ostringstream post_point_ss;
				post_point_ss << post_point;
				std::string post_point_s = post_point_ss.str();

				if ( pre_point_s.length() + post_point_s.length() > sigfig )
					post_point_s.erase( sigfig - pre_point_s.length() );

				while( *(post_point_s.rbegin()) == '0' )
					post_point_s.erase( post_point_s.length() - 1 );

				if ( *(post_point_s.rbegin()) == '.' )
					post_point_s.erase( post_point_s.length() - 1 );

				string final = pre_point_s;

				if ( post_point_s != "" ) {
					final += ".";
					final += post_point_s;
				}

				final += suffix.second;

				return final;
			}
		}
	}

	std::ostringstream give_up_ss;
	give_up_ss << value;
	return give_up_ss.str() + (*suffixes)[0];
}


string Template::pretty_price( double price, char sigfig ) {
	std::ostringstream ss;
	ss.setf( std::ios::fixed );
	ss.precision( sigfig );
	ss << price;
	return ss.str();
}


string Template::pretty_time( time_t time, std::string time_fmt ) {
	if ( time_fmt.empty() )
		time_fmt = "%A, %eth %B %Y %H:%M:%S";
		
	struct tm *tm = localtime( &time );

	char time_buffer[100];
	
	strftime( time_buffer, sizeof(time_buffer), time_fmt.c_str(), tm );
	
	std::string time_out = time_buffer;
	
	std::size_t th_pos = time_out.find( "th " );
	
	if ( th_pos != std::string::npos ) {
		switch ( tm->tm_mday % 10 ) {
			case 1:
				time_out.replace( th_pos, 2, "st" );
				break;
			case 2:
				time_out.replace( th_pos, 2, "nd" );
				break;
			case 3:
				time_out.replace( th_pos, 2, "rd" );
				break;
			default:
				break;
		}
	}

	return time_out;	
}


string Template::break_base64( std::string str ) {
	int offset = 0;
	while( offset < str.length() ) {
		str.insert( offset, "\r\n" );
		offset += 2;
		offset += 76;
	}
	
	return str;
}
