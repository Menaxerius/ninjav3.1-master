#include "Handler.hpp"
#include "ReqResp.hpp"

#include <iostream>

// for strlen() on char *
#include <string.h>
#include <microhttpd.h>


// inheritance endpoint: no further handler
Handler *Handler::route( struct MHD_Connection *connection, Request *req, Response *resp ) {
	return NULL;
}


// simple post_processor accepts everything
int Handler::post_processor( void *coninfo_cls, enum MHD_ValueKind kind, const char *key, const char *filename, 
								const char *content_type, const char *transfer_encoding, const char *data, uint64_t off, size_t size ) {

	ReqResp *req_resp = (ReqResp *) coninfo_cls;

	#ifdef HANDLER_DEBUG
		std::cout << "[Handler] " << key << " += \"" << std::string(data + off, size) << "\" [" << size << "@" << off << "]" << std::endl;
	#endif

	req_resp->req->add_post_data( std::string(key), data, off, size );

	return MHD_YES;
}


uint64_t Handler::max_raw_post_data_size() {
	return 0;
}


int Handler::raw_post_processor( void *coninfo_cls, const char *upload_data, size_t *upload_data_size ) {
	// if this is the "POST upload finished" call then all's good
	if (upload_data == nullptr)
		return MHD_YES;

	ReqResp *req_resp = (ReqResp *) coninfo_cls;
	Request *req = req_resp->req;

	if ( req->add_raw_post_data(upload_data, *upload_data_size) )
		return MHD_YES;

	return MHD_NO;
}


int Handler::process_headers( struct MHD_Connection *connection, Request *req, Response *resp ) {
	return MHD_YES;
}


int Handler::process( struct MHD_Connection *connection, Request *req, Response *resp ) {
	// if we get this far then something isn't right
	std::cout << "[Handler] No handler found?" << std::endl;
	return MHD_NO;
}
