#ifndef INCLUDE__FITNESSAPI_HPP
#define INCLUDE__FITNESSAPI_HPP

#include <curl/curl.h>
#include <string>
#include <functional>


class FitnessAPI {
	public:
		std::string access_token;
		std::string refresh_token;
		time_t expires_when;
		std::string user;
		std::function<void()> destructor_callback;

		FitnessAPI() : expires_when(0) {};
		#ifdef FitnessAPI_lambda_debug
			FitnessAPI(FitnessAPI &&) = default; // move constructor
			FitnessAPI(FitnessAPI &) = delete; // no copy constuctor
		#endif

		virtual std::string get_authorization_URL( std::string username ) =0;
		virtual bool authorization_callback( std::string auth_code ) =0;
		virtual bool refresh_authorization() =0;
		virtual int get_steps( time_t date = 0 ) =0;


		class call {
			private:
				CURL *curl_handle;
				struct curl_slist *headers;
				std::string output_string;

			public:
				call( std::string url );
				std::string perform( std::string post_data = "" );
				void add_header( std::string header );
		};
};

#endif
