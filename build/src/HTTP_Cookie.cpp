#include "HTTP_Cookie.hpp"

#include <sstream>

#define COOKIE_VALID "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789"


HTTP_Cookie::HTTP_Cookie() {
	secure = false;
	// 0 = session cookie, use 1 (or some other time in the past) to expire
	expires = 0;
	// WATCH OUT! ON BY DEFAULT!
	http_only = true;
}


std::string HTTP_Cookie::to_string() {
	// copy and escape value first
	std::string str = value;
	
	std::size_t pos = str.find_last_not_of( COOKIE_VALID );
	
	while( pos != std::string::npos ) {
		std::stringstream replacement;
		replacement << "%x";
		replacement.fill( '0' );
		replacement.width( 2 );
		replacement << (unsigned char) str[pos];
	
		str.replace( pos, 1, replacement.str() );
		
		if ( pos > 0 )
			pos = str.find_last_not_of( COOKIE_VALID, pos - 1 );
		else
			pos = std::string::npos;
	}
	
	// add extra stuff
	str = name + "=" + str;
	
	// expires?
	if (expires) {
		str += "; Expires=";
	 	struct tm *now_tm = gmtime( &expires );
	 	char expires_date[40];
	 	strftime( expires_date, sizeof(expires_date), "%a, %d %b %Y %T GMT", now_tm );
	 	str += expires_date;
	}
	
	// domain?
	if (domain != "")
		str += "; Domain=" + domain;
		
	// path?
	if (path != "")
		str += "; Path=" + path;
		
	// secure?
	if (secure)
		str += "; Secure";
		
	// HTTP only?
	if (http_only)
		str += "; HttpOnly";
		
	return str;
}
