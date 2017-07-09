#ifndef INCLUDE__FITBITAPI_HPP
#define INCLUDE__FITBITAPI_HPP

#include "FitnessAPI.hpp"

#include "Base64.hpp"
#include "JSON.hpp"

#include <string>


class FitBitAPI : public FitnessAPI {
	private:
		std::string base64_auth;

		FitnessAPI::call prep_call( std::string url );

	public:
		std::string clientID;
		std::string client_secret;

		FitBitAPI(std::string id, std::string secret): clientID(id), client_secret(secret) {
			std::string base64_input( clientID + ":" + client_secret );

			char *base64_output;
			uint64_t base64_len = base64_encode(&base64_output, base64_input.c_str(), base64_input.length() );

			base64_auth = std::string( base64_output, base64_len );
		};

		#ifdef FitnessAPI_lambda_debug
			FitBitAPI(FitBitAPI &&) = default; // move constructor
			FitBitAPI(FitBitAPI &) = delete; // no copy constructor
		#endif

		~FitBitAPI() {
			if (destructor_callback)
				destructor_callback();
		}
	
		virtual std::string get_authorization_URL( std::string username = "");
		virtual bool authorization_callback( std::string auth_code );
		virtual bool refresh_authorization();
		virtual int get_steps( time_t date = 0 );
};

#endif
