#include "FitBitAPI.hpp"
#include "JSON.hpp"

#include <iostream>
#include <time.h>


FitnessAPI::call FitBitAPI::prep_call( std::string url ) {
	// no auth data?
	if ( access_token.empty() || refresh_token.empty() || expires_when == 0 )
		throw std::runtime_error("FitBit access/refresh tokens missing");

	// do we need to refresh tokens?
	if ( expires_when < time(NULL) && !refresh_authorization() )
		throw std::runtime_error("Couldn't refresh FitBit access/refresh tokens");

	FitnessAPI::call request(url);
	request.add_header( "Authorization: Bearer " + access_token );
	request.add_header( "Content-Type: application/x-www-form-urlencoded" );
	return request;
}


std::string FitBitAPI::get_authorization_URL( std::string username ) {
	return "https://www.fitbit.com/oauth2/authorize?response_type=code&client_id=" + clientID + "&scope=activity%20nutrition%20sleep%20weight";
}


bool FitBitAPI::authorization_callback( std::string auth_code ) {
	// auth_code = "44071f504f0064ef8f8b3c0810429cd3ee18d6d1";

	// we need to get initial access & refresh token before it expires

	/*
		POST https://api.fitbit.com/oauth2/token
		Authorization: Basic MjI3TlNKOmY2NzFkYmU2MjlhZGE5NDUwYzllODlkMmZlNTI0NDU2
		Content-Type: application/x-www-form-urlencoded

		client_id=227NSJ&grant_type=authorization_code&code=44071f504f0064ef8f8b3c0810429cd3ee18d6d1
	 */
	FitnessAPI::call request( "https://api.fitbit.com/oauth2/token" );
	request.add_header( "Authorization: Basic " + base64_auth );
	request.add_header( "Content-Type: application/x-www-form-urlencoded" );

	std::string response = request.perform( "client_id=" + clientID + "&grant_type=authorization_code&code=" + auth_code );

	std::cout << "auth callback JSON: " << response << std::endl;

	try {
		JSON json(response);

		access_token = json.get_string("access_token");
		refresh_token = json.get_string("refresh_token");
		user = json.get_string("user_id");

		return true;
	} catch(...) {
	}

	// not good
	std::cerr << "Couldn't obtain initial FitBit access/refresh tokens:\n" << response << std::endl;
	return false;
}


bool FitBitAPI::refresh_authorization() {
	FitnessAPI::call request( "https://api.fitbit.com/oauth2/token" );
	request.add_header( "Authorization: Basic " + base64_auth );
	request.add_header( "Content-Type: application/x-www-form-urlencoded" );

	std::string response = request.perform( "client_id=" + clientID + "&grant_type=refresh_token&refresh_token=" + refresh_token );

	try {
		JSON json(response);

		access_token = json.get_string("access_token");
		refresh_token = json.get_string("refresh_token");

		return true;
	} catch(...) {
	}

	// not good
	std::cerr << "Couldn't refresh FitBit access/refresh tokens:\n" << response << std::endl;
	return false;
}


int FitBitAPI::get_steps( time_t date ) {
	std::string date_URL = "today/1d";

	if (date) {
		char formatted_date[12];
		struct tm result;

		// START DATE
		if ( gmtime_r( &date, &result ) == &result ) {
			strftime( formatted_date, sizeof(formatted_date), "%Y-%m-%d", &result );
			date_URL = std::string(formatted_date);
		}

		// END DATE
		date = time(NULL) - 86400;
		if ( gmtime_r( &date, &result ) == &result ) {
			strftime( formatted_date, sizeof(formatted_date), "%Y-%m-%d", &result );
			date_URL += "/" + std::string(formatted_date);
		}
	}

	FitnessAPI::call request = prep_call( "https://api.fitbit.com/1/user/" + user + "/activities/tracker/steps/date/" + date_URL + ".json" );

	std::string response = request.perform();

	try {
		JSON json(response);

		JSON_Array entries = json.get_array("activities-tracker-steps");

		int64_t steps = 0;
		for(int i=0; i<entries.size(); i++) {
			JSON entry = entries.get_item(i);

			steps += std::atoi( entry.get_string("value").c_str() );
		}

		return steps;
	} catch(...) {
		std::cout << response << std::endl;
	}

	return -1;
}
