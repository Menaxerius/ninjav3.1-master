#include "fetch.hpp"

#include <curl/curl.h>
#include <cstdio>
#include <iostream>
#include <thread>


static size_t write_data( void *buffer, size_t size, size_t nmemb, std::string *output ) {
	size_t length = size * nmemb;
	output->append( (const char *) buffer, length );
	return length;
}


std::string fetch( const std::string &url, const std::string &post_data, const std::string &content_type, const std::string &auth, const time_t timeout ) {
	CURL *curl_handle = curl_easy_init();

	std::string output_string;
	struct curl_slist *headers = nullptr;

	headers = curl_slist_append( headers, "Accept: application/json" );
	headers = curl_slist_append( headers, "Accept-Language: en_US" );

	if ( !content_type.empty() ) {
		const std::string content_type_header = "Content-Type: " + content_type;
		headers = curl_slist_append( headers, content_type_header.c_str() );
	}

	curl_easy_setopt( curl_handle, CURLOPT_URL, url.c_str() );
	curl_easy_setopt( curl_handle, CURLOPT_NOSIGNAL, 1L );
	curl_easy_setopt( curl_handle, CURLOPT_WRITEFUNCTION, write_data );
	curl_easy_setopt( curl_handle, CURLOPT_WRITEDATA, &output_string );
	curl_easy_setopt( curl_handle, CURLOPT_HTTPHEADER, headers );
	
	if ( ! post_data.empty() )
		curl_easy_setopt( curl_handle, CURLOPT_POSTFIELDS, post_data.c_str() );

	if ( ! auth.empty() ) {
		curl_easy_setopt( curl_handle, CURLOPT_USERPWD, auth.c_str() );
		curl_easy_setopt( curl_handle, CURLOPT_HTTPAUTH, CURLAUTH_BASIC );
	}

	if ( timeout != 0 ) {
		curl_easy_setopt( curl_handle, CURLOPT_CONNECTTIMEOUT, timeout );
		curl_easy_setopt( curl_handle, CURLOPT_TIMEOUT, timeout );
	}

	try {
		#ifdef FETCH_CURL_VERBOSE
			CURLcode result = curl_easy_perform( curl_handle );
			if ( result != 0 )
				std::cout << "CURL error: " << curl_easy_strerror(result) << std::endl;
		#else
			curl_easy_perform( curl_handle );
		#endif
	} catch(const std::exception &e) {
		std::cerr << "CURL threw: " << e.what() << std::endl;
		throw(e);
	}

	curl_slist_free_all( headers );
	curl_easy_cleanup( curl_handle );

	return output_string;
}


static void async_fetch_thread( std::string url, std::string post_data, std::string content_type, std::string auth, time_t timeout ) {
	fetch(url, post_data, content_type, auth, timeout);
}


void async_fetch( const std::string &url, const std::string &post_data, const std::string &content_type, const std::string &auth, const time_t timeout ) {
	std::thread thread( async_fetch_thread, url, post_data, content_type, auth, timeout );
	thread.detach();
}


std::string URL_encode( std::string data ) {
	size_t offset = data.length();

	while( ( offset = data.find_last_of("%?& <>\"\n\r=", offset) ) != std::string::npos ) {
		if ( data[offset] == ' ' )
			data[offset] = '+';
		else {
			char hex[4];
			sprintf( hex, "%%%02hhx", data[offset] );		// '%' followed by leading-0 2-wide hex string based on unsigned char ("hh" modifier)

			data.replace( offset, 1, hex, sizeof(hex) );

			// last char?
			if (offset == 0)
				break;

			// carry on
			offset--;
		}
	}

	return data;
}
