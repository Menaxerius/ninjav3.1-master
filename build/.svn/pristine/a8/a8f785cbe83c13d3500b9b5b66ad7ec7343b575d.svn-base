#include "PayPal.hpp"
#include "cJSON.hpp"

#include <stdexcept>
#include <iostream>
#include <sstream>
#include <string.h>

#define AUTH_ENDPOINT "https://api.paypal.com/v1/oauth2/token"
#define PAYMENT_ENDPOINT "https://api.paypal.com/v1/payments/payment"


bool PayPal::has_called_curl_global_init = false;


PayPal::PayPal( std::string client_id, std::string secret ) {
	if ( ! has_called_curl_global_init )
		throw new std::runtime_error( "PayPal::init() needs to be called before first use" );

	curl_handle = curl_easy_init();

	if (! curl_handle )
		throw new std::runtime_error( "libCURL couldn't initialize" );
	
	this->client_id = client_id;
	this->secret = secret;
};


PayPal::~PayPal() {
	curl_easy_cleanup( curl_handle );
};


// we need to do it this way because libcurl init needs to be called before threads are created
int PayPal::init() {
	if ( ! has_called_curl_global_init ) {
		if ( ! curl_global_init(CURL_GLOBAL_ALL) ) {
			has_called_curl_global_init = true;
			return 0;
		} else {
			return -1;
		}
	}
	
	// all OK
	return 0;
};


std::string PayPal::call( std::string URL, std::string payload, bool requires_token ) {
	curl_easy_reset( curl_handle );
	
	std::string output_string;

	struct curl_slist *headers = NULL;

	if (requires_token) {
		if ( ! bearer_token.empty() ) {
			if ( bearer_expiry < time(NULL) ) {
				// wipe old token
				bearer_expiry = 0;
				bearer_token = "";
			}
		}
		
		if ( bearer_token.empty() ) {
			// obtain token using recursion!
			// (note: wipes all previously set curl options so do them last)
			std::string token_json = call( AUTH_ENDPOINT, "grant_type=client_credentials", false );

			cJSON *root = cJSON_Parse( token_json.c_str() );
			
			if (!root)
				throw new std::runtime_error( "Call to obtain bearer token from PayPal failed" );
				
			if ( cJSON *error_desc = cJSON_GetObjectItem( root, "error_description" ) ) {
				std::string error = "PayPal error: ";
				error += error_desc->valuestring;
				throw new std::runtime_error( error );
			}
			
			if ( cJSON *access_token = cJSON_GetObjectItem( root, "access_token" ) )
				bearer_token = access_token->valuestring;
			
			if ( cJSON *expires_in = cJSON_GetObjectItem( root, "expires_in" ) )
				bearer_expiry = time(NULL) + expires_in->valueint;
				
			cJSON_Delete( root );
			
			std::cerr << "New bearer token: " << bearer_token << std::endl;
		}
	
		std::string auth_header = "Authorization: Bearer ";
		auth_header += bearer_token;
		
		headers = curl_slist_append( headers, auth_header.c_str() );
		headers = curl_slist_append( headers, "Content-Type: application/json" );
	} else {
		// no token? best use the client_id and secret then
		std::string auth = client_id;
		auth += ":";
		auth += secret;
		
		curl_easy_setopt( curl_handle, CURLOPT_USERPWD, auth.c_str() );
	}	

	headers = curl_slist_append( headers, "Accept: application/json" );
	headers = curl_slist_append( headers, "Accept-Language: en_US" );
	curl_easy_setopt( curl_handle, CURLOPT_HTTPHEADER, headers );

	curl_easy_setopt( curl_handle, CURLOPT_URL, URL.c_str() );
	curl_easy_setopt( curl_handle, CURLOPT_POSTFIELDS, payload.c_str() );
	curl_easy_setopt( curl_handle, CURLOPT_NOSIGNAL, (long) 1);
	curl_easy_setopt( curl_handle, CURLOPT_WRITEFUNCTION, write_data );
	curl_easy_setopt( curl_handle, CURLOPT_WRITEDATA, &output_string );

	// perform
	std::cerr << "Payload: " << payload << std::endl;
	curl_easy_perform( curl_handle );

	if (headers)
		curl_slist_free_all( headers );
		
	return output_string;
};


size_t PayPal::write_data( void *buffer, size_t size, size_t nmemb, std::string *output ) {
	size_t length = size * nmemb;
	output->append( (const char *) buffer, length );
	return length;
};

PayPal_Payment *PayPal::payment( std::string return_URL, std::string cancel_URL, double total, std::string currency, std::string description ) {
	/*
	    "payer": {
	        "payment_method": "paypal"
	    },
	    "transactions": [
	        {
	            "amount": {
	                "currency": "<%=currency%>",
	                "total": "<%=std::to_string(total)%>"
	            },
	            "description": "<%=description%>"
	        }
	    ],
	    "redirect_urls": {
	        "return_url": "<%=return_url%>",
	        "cancel_url": "<%=cancel_url%>"
	    }
	*/

	cJSON *root = cJSON_CreateObject();
	cJSON_AddStringToObject( root, "intent", "sale" );
	
	cJSON *payer = cJSON_CreateObject();
	cJSON_AddItemToObject( root, "payer", payer );
	cJSON_AddStringToObject( payer, "payment_method", "paypal" );
	
	cJSON *transactions = cJSON_CreateArray();
	cJSON_AddItemToObject( root, "transactions", transactions );

	cJSON *transaction1 = cJSON_CreateObject();
	cJSON_AddItemToArray( transactions, transaction1 );

	cJSON *amount = cJSON_CreateObject();
	cJSON_AddItemToObject( transaction1, "amount", amount );
	cJSON_AddStringToObject( amount, "currency", currency.c_str() );

	// we need to enforce 2 decimal places as doubles are prone to inaccuracy
	std::ostringstream total_fixed;
	total_fixed.setf( std::ios::fixed, std:: ios::floatfield );
	total_fixed.precision(2);
	total_fixed << total;
	cJSON_AddStringToObject( amount, "total", total_fixed.str().c_str() );

	cJSON_AddStringToObject( transaction1, "description", description.c_str() );
	
	cJSON *redirect_urls = cJSON_CreateObject();
	cJSON_AddItemToObject( root, "redirect_urls", redirect_urls );
	cJSON_AddStringToObject( redirect_urls, "return_url", return_URL.c_str() );
	cJSON_AddStringToObject( redirect_urls, "cancel_url", cancel_URL.c_str() );

	char *json = cJSON_Print( root );
	std::string response_json = call( PAYMENT_ENDPOINT, json, true );
	free(json);
	cJSON_Delete( root );
	
	std::cerr << "Payment response JSON: " << response_json << std::endl;
	
	if ( response_json.empty() )
		return NULL;
		
	root = cJSON_Parse( response_json.c_str() );

	if ( cJSON *state = cJSON_GetObjectItem( root, "state" ) ) {
		if ( strcmp( state->valuestring, "created" ) != 0 ) {
			cJSON_Delete(root);
			return NULL;
		}
	} else {
		cJSON_Delete(root);
		return NULL;
	}

	// these fields should be here given state: "created"
	std::string paymentID = cJSON_GetObjectItem( root, "id" )->valuestring;
	std::string approval_URL;
	std::string execute_URL;
	
	cJSON *links = cJSON_GetObjectItem( root, "links" );
	int links_length = cJSON_GetArraySize( links );
	
	for(int i=0; i<links_length; i++) {
		cJSON *link = cJSON_GetArrayItem( links, i );
		std::string rel = cJSON_GetObjectItem( link, "rel" )->valuestring;
		
		if ( rel == "approval_url" )
			approval_URL = cJSON_GetObjectItem( link, "href" )->valuestring;
		else if ( rel == "execute" )
			execute_URL = cJSON_GetObjectItem( link, "href" )->valuestring;
	}	 

	cJSON_Delete(root);
	
	return new PayPal_Payment( paymentID, approval_URL, execute_URL );
};


std::string PayPal::execute( std::string execute_URL, std::string payerID, std::string &transactionID ) {
	cJSON *root = cJSON_CreateObject();
	cJSON_AddStringToObject( root, "payer_id", payerID.c_str() );
	
	char *json = cJSON_Print( root );
	std::string response_json = call( execute_URL, json, true );
	free(json);
	cJSON_Delete(root);

	std::cerr << "Payment execute response JSON: " << response_json << std::endl;

	if ( response_json.empty() )
		return "no response";

	root = cJSON_Parse( response_json.c_str() );
	std::string result;

	if ( cJSON *state = cJSON_GetObjectItem( root, "state" ) ) {
		result = state->valuestring; 

		// transactions[0] -> related_resources[0] -> sale -> id
		cJSON *transactions = cJSON_GetObjectItem( root, "transactions" );
		if ( cJSON_GetArraySize( transactions ) ) {
			cJSON *transaction = cJSON_GetArrayItem( transactions, 0 );
			
			cJSON *related_resources = cJSON_GetObjectItem( transaction, "related_resources" );
			if ( cJSON_GetArraySize( related_resources ) ) {
				cJSON *related_resource = cJSON_GetArrayItem( related_resources, 0 );
				
				cJSON *sale = cJSON_GetObjectItem( related_resource, "sale" );
				transactionID = cJSON_GetObjectItem( sale, "id" )->valuestring;
			}
		}
	} else {
		result = "error";
	}

	cJSON_Delete(root);
	
	return result;
};


PayPal_Payment::PayPal_Payment( std::string paymentID, std::string approval_URL, std::string execute_URL ) {
	this->paymentID = paymentID;
	this->approval_URL = approval_URL;
	this->execute_URL = execute_URL;
};
