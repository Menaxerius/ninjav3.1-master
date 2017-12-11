#include "burst.hpp"

#include "API/submitNonce.hpp"
#include "API/getMiningInfo.hpp"

#include "JSON.hpp"


Handler *Handlers::burst::route( struct MHD_Connection *connection, Request *req, Response *resp ) {
	// if there's no Content-Type header we need to add it to appease libmicrohttpd
	if ( MHD_lookup_connection_value(connection, MHD_HEADER_KIND, MHD_HTTP_HEADER_CONTENT_TYPE) == nullptr )
		MHD_set_connection_value(connection, MHD_HEADER_KIND, MHD_HTTP_HEADER_CONTENT_TYPE, (char *)"application/x-www-form-urlencoded");

	// no need for any more routing
	return nullptr;
}


int Handlers::burst::process( struct MHD_Connection *connection, Request *req, Response *resp ) {
	// have we been given a requestType?
	std::string request_type = req->get_query_or_post("requestType");
	
	if (request_type == "getMiningInfo")
		return Handlers::API::getMiningInfo::inner( connection, req, resp );

	if (request_type == "submitNonce")
		return Handlers::API::submitNonce::inner( connection, req, resp );

	// we're in error-land now
	JSON error_json;

	if ( request_type.empty() ) {
		error_json.add_number( "errorCode", 2 );
		error_json.add_string( "errorDescription", "This Burst pool API endpoint requires a 'requestType' argument." );
		resp->status_code = 400;
	} else {
		error_json.add_number( "errorCode", 1 );
		error_json.add_string( "errorDescription", "This Burst pool API endpoint only supports 'getMiningInfo' and 'submitNonce' requestTypes." );
		resp->status_code = 404;
	}

	resp->content = error_json.to_string();

	// done!
	return MHD_YES;
}
