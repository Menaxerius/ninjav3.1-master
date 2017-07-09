#ifndef INCLUDE__PAYPAL_HPP
#define INCLUDE__PAYPAL_HPP

#include <curl/curl.h>
#include <string> 


class PayPal_Payment {
	public:
		PayPal_Payment( std::string paymentID, std::string approval_URL, std::string execute_URL );

		std::string paymentID;
		std::string approval_URL;
		std::string execute_URL;
};


class PayPal {
	public:
		PayPal( std::string client_id, std::string secret );
		~PayPal();
		static int init();

		PayPal_Payment *payment( std::string returnURL, std::string cancelURL, double total, std::string currency, std::string description );
		std::string execute( std::string executeURL, std::string payerID, std::string &transactionID );
		
	protected:
		static bool has_called_curl_global_init;
		CURL *curl_handle;
		
		std::string client_id;
		std::string secret;
		
		std::string bearer_token;
		time_t bearer_expiry;
		
		std::string call( std::string URL, std::string payload, bool requires_token );
		static size_t write_data( void *buffer, size_t size, size_t nmemb, std::string *output );			
};

#endif
