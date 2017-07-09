#ifndef INCLUDE__TEMPLATE_HPP
#define INCLUDE__TEMPLATE_HPP

#include <string>
#include <list>
#include <map>
#include <vector>

#include <time.h>


typedef std::map<unsigned long long int,const char *> sigfig_suffixes_t;
typedef std::pair<unsigned long long int,const char *> sigfig_suffix_tuple;

typedef std::string (*string_string_func_t)(std::string);

class Request;

class Template {
	protected:
		Request *req;

		static std::string find_and_replace( std::string str, std::string find, std::string replace );

	public:
		Template(Request *incoming_req) : req(incoming_req) {};
		virtual ~Template();
		virtual std::string render() =0;
		
		static std::string HTML_escape( std::string str );
		static std::string HTML_escape_with_BRs( std::string str );
		static std::string JavaScript_escape( std::string str );
		static std::string URL_escape( std::string str );
		
		static std::string JS_quote( std::string str );
		
		static std::list<std::string> map( string_string_func_t fn, std::list<std::string> &items );
		static std::string join( std::string delimiter, std::list<std::string> items );
		static std::string join( const char delimiter[], std::list<std::string> items );
		static std::vector<std::string> split( const char delimiter, const std::string str );

		static std::string pretty_bytesize( long long int value, char sigfig = 4 );
		static std::string pretty_size( long long int value, char sigfig = 4 );
		static std::string sigfig_suffix( long long int value, sigfig_suffixes_t *suffixes, char sigfig = 4 );
		static std::string pretty_price( double price, char sigfig = 2 );
		
		static std::string pretty_time( time_t time, std::string time_fmt = "" );
		
		static std::string break_base64( std::string str );

		template<typename T>
		static std::string plural( const T value, std::string suffix = "s" ) {
			return (value != 1 ? suffix : "");
		}

		template<typename T>
		static std::string plural_with_value( const T value, std::string prefix, std::string suffix = "s" ) {
			return std::to_string(value) + " " + prefix + (value != 1 ? suffix : "");
		}
};

#endif
