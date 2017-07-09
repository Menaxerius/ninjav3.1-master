#ifndef INCLUDE__REQUEST_HPP
#define INCLUDE__REQUEST_HPP

#include <microhttpd.h>

#include <map>
#include <string>

typedef enum { GET, POST } Request_methods;

typedef std::map<std::string, std::string> query_args_t;
typedef std::pair<std::string, std::string> query_arg_t;

typedef std::map<std::string, std::string> post_data_t;
typedef std::pair<std::string, std::string> post_datum_t;

typedef std::map<std::string, std::string> cookies_t;
typedef std::pair<std::string, std::string> cookie_t;

typedef std::map<std::string, std::string> headers_t;
typedef std::pair<std::string, std::string> header_t;

class Handler;

class Request {
	protected:
		query_args_t				query_args;
		post_data_t					post_data;
		cookies_t					cookies;
		headers_t					headers;
		char *						raw_post_data;
		uint64_t					raw_post_data_size;

	public:
		Request_methods				method;
		std::string					url;
		float						version;
		
		Handler						*base_handler;
		Handler						*handler;
		struct MHD_PostProcessor	*post_processor;

		Request();
		~Request();

		
		std::string get_query_arg( std::string name );
		void parse_query( struct MHD_Connection *connection );
		//this has to public to have access from non-object function	
		int _parse_query( void *cls, enum MHD_ValueKind kind, const char *key, const char *value );
		
		std::string get_cookie( std::string name );
		void set_cookie( std::string name, std::string value );
		cookies_t &get_cookies();
		void parse_cookies( struct MHD_Connection *connection );
		//this has to public to have access from non-object function	
		int _parse_cookie( void *cls, enum MHD_ValueKind kind, const char *key, const char *value );
		
		void add_post_data( std::string name, const char *data, uint64_t offset, size_t size );
		std::string get_post_data( std::string name );
		post_data_t &get_post_data();

		const char *get_raw_post_data();
		const uint64_t get_raw_post_data_size();
		bool add_raw_post_data(const char *data, size_t size);

		std::string form( std::string name );
		std::string get_query_or_post( std::string name );

		std::string get_header( std::string name );
		headers_t &get_headers();
		void parse_headers( struct MHD_Connection *connection );
		//this has to public to have access from non-object function	
		int _parse_header( void *cls, enum MHD_ValueKind kind, const char *key, const char *value );
		
		static std::string trim( std::string value );
		static bool check_email( std::string email );
};

#endif
