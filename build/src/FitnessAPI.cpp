#include "FitnessAPI.hpp"

#include <stdint.h>
#include <stdlib.h>
#include <iostream>


static size_t write_data( void *buffer, size_t size, size_t nmemb, std::string *output ) {
	size_t length = size * nmemb;
	output->append( (const char *) buffer, length );
	return length;
}


FitnessAPI::call::call( std::string url ) {
	curl_handle = curl_easy_init();
	headers = nullptr;

	curl_easy_setopt( curl_handle, CURLOPT_URL, url.c_str() );
	curl_easy_setopt( curl_handle, CURLOPT_NOSIGNAL, (long) 1);
	curl_easy_setopt( curl_handle, CURLOPT_WRITEFUNCTION, write_data );
	curl_easy_setopt( curl_handle, CURLOPT_WRITEDATA, &output_string );
}


std::string FitnessAPI::call::perform( std::string post_data ) {
	if (headers)
		curl_easy_setopt( curl_handle, CURLOPT_HTTPHEADER, headers );

	if ( ! post_data.empty() )
		curl_easy_setopt( curl_handle, CURLOPT_POSTFIELDS, post_data.c_str() );

	CURLcode result = curl_easy_perform( curl_handle );

	if ( result != 0 )
		std::cerr << "CURL error: " << curl_easy_strerror(result) << std::endl;

	if (headers)
		curl_slist_free_all( headers );

	curl_easy_cleanup( curl_handle );

	return output_string;
}


void FitnessAPI::call::add_header( std::string header ) {
	headers = curl_slist_append( headers, header.c_str() );
}
