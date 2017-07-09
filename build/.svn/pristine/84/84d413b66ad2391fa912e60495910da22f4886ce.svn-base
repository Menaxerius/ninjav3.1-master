#include "Request.hpp"
#include "Handler.hpp"

#include <microhttpd.h>

#include <locale>
#include <regex>

#include <netinet/in.h>
#include <arpa/nameser.h>
#include <resolv.h>


Request::Request() {
	base_handler = NULL;
	handler = NULL;
	post_processor = NULL; 
	raw_post_data = nullptr;
	raw_post_data_size = 0;
}


Request::~Request() {
	if (handler)
		delete handler;

	if (base_handler)
		delete base_handler;

	if (post_processor)
		MHD_destroy_post_processor( post_processor );

	if (raw_post_data != nullptr)
		free(raw_post_data);
}


int parse_query_bounce( void *cls, enum MHD_ValueKind kind, const char *key, const char *value ) {
	if (value == NULL) {
		return MHD_YES;
	} else {
		Request *me = (Request *) cls;
		return me->_parse_query( cls, kind, key, value);
	}
}


int Request::_parse_query( void *cls, enum MHD_ValueKind kind, const char *key, const char *value ) {
	// could add automatic support for vectorisation of keys of the style "my_key[]"
	this->query_args[key] = value;
	
	return MHD_YES;
}


void Request::parse_query( struct MHD_Connection *connection ) {
	query_args.clear();
	MHD_get_connection_values( connection, MHD_GET_ARGUMENT_KIND, &parse_query_bounce, this );
}


int parse_cookie_bounce( void *cls, enum MHD_ValueKind kind, const char *key, const char *value ) {
	Request *me = (Request *) cls;
	return me->_parse_cookie( cls, kind, key, value);
}


int Request::_parse_cookie( void *cls, enum MHD_ValueKind kind, const char *key, const char *value ) {
	this->cookies[key] = value;
	
	return MHD_YES;
}


void Request::parse_cookies( struct MHD_Connection *connection ) {
	cookies.clear();
	MHD_get_connection_values( connection, MHD_COOKIE_KIND, &parse_cookie_bounce, this );
}


void Request::add_post_data( std::string name, const char *data, uint64_t offset, size_t size ) {
	std::string current = post_data[ name ];
	current += std::string( data, size );
	post_data[name] = current;
}


// lookup without creating empty element if it doesn't exist
std::string Request::get_query_arg( std::string name ) {
	query_args_t::iterator it = query_args.find( name );
	
	if (it == query_args.end()) {
		return std::string("");
	} else {
		return it->second;
	}
}


// lookup without creating empty element if it doesn't exist
std::string Request::get_post_data( std::string name ) {
	post_data_t::iterator it = post_data.find( name );

	if (it == post_data.end()) {
		return std::string("");
	} else {
		return it->second;
	}
}


post_data_t &Request::get_post_data() {
	return post_data;
}


const char *Request::get_raw_post_data() {
	return raw_post_data;
}


const uint64_t Request::get_raw_post_data_size() {
	return raw_post_data_size;
}


bool Request::add_raw_post_data(const char *data, size_t size) {
	if ( this->raw_post_data_size + size > this->handler->max_raw_post_data_size() )
		return false;

	if ( this->raw_post_data == nullptr )
		this->raw_post_data = (char *)malloc( this->handler->max_raw_post_data_size() );

	memcpy( &this->raw_post_data[this->raw_post_data_size], data, size );

	this->raw_post_data_size += size;

	return true;
}


cookies_t & Request::get_cookies() {
	return cookies;
}


std::string Request::get_cookie( std::string name ) {
	cookies_t::iterator it = cookies.find( name );

	if (it == cookies.end()) {
		return std::string("");
	} else {
		return it->second;
	}
}


void Request::set_cookie( std::string name, std::string value ) {
	cookies[ name ] = value;
}


std::string Request::form( std::string name ) {
	if (method == GET)
		return get_query_arg( name );
	else
		return get_post_data( name );
}


std::string Request::get_query_or_post( std::string name ) {
	std::string data = get_query_arg( name );
	if ( data.empty() )
		data = get_post_data( name );
	return data;
}


headers_t & Request::get_headers() {
	return headers;
}


std::string Request::get_header( std::string name ) {
	headers_t::iterator it = headers.find( name );

	if (it == headers.end()) {
		return std::string("");
	} else {
		return it->second;
	}
}

int parse_header_bounce( void *cls, enum MHD_ValueKind kind, const char *key, const char *value ) {
	Request *me = (Request *) cls;
	return me->_parse_header( cls, kind, key, value);
}


int Request::_parse_header( void *cls, enum MHD_ValueKind kind, const char *key, const char *value ) {
	this->headers[key] = value;
	
	return MHD_YES;
}


void Request::parse_headers( struct MHD_Connection *connection ) {
	headers.clear();
	MHD_get_connection_values( connection, MHD_HEADER_KIND, &parse_header_bounce, this );
}


std::string Request::trim( std::string value ) {
	while ( !value.empty() && std::isspace( value[0] ) )
		value.erase(0, 1);
		
	while ( !value.empty() && std::isspace( value[value.length() - 1] ) )
		value.erase(value.length() - 1, 1);
		
	return value;
}


bool Request::check_email( std::string email ) {
	std::smatch matches;

	std::regex_match( email, matches, std::regex( "^([^:;@<>]+)@([a-zA-Z0-9._-]+)$" ) );
	if ( matches.size() != 3 )
		return false; 
	
	// check MX
	unsigned char answer[1500];
	int ret = res_query( matches[2].str().c_str(), C_IN, T_MX, answer, sizeof(answer) );
	
	if (ret == -1)
		return false;
	
	HEADER *hdr = reinterpret_cast<HEADER*>(answer);
	
	if (hdr->rcode != NOERROR)
		return false;
		
	// no answers? no go!
	if (hdr->ancount == 0)
		return false;
	
	return true;
}
