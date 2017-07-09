#include "Response.hpp"
#include "Base64.hpp"

#include <regex>
#include <iostream>

// for fstat()
#include <sys/types.h>
#include <sys/stat.h>

// for websocket support
#include <openssl/sha.h>


Response::Response() {
	fd = 0;
	default_content_type = "text/html; charset=UTF-8";
	status_code = 0;
}


void Response::add_cookie( HTTP_Cookie *hc ) {
	cookies.push_back( hc );
}


int Response::send( struct MHD_Connection *connection ) {
	const char *page = content.c_str();

	struct MHD_Response *response;

	if (status_code == MHD_HTTP_SWITCHING_PROTOCOLS) {
		response = MHD_create_response_for_upgrade( upgrade_handler, upgrade_handler_cls );
	} else if (fd) {
		struct stat stat_info;
		int stat_ret = fstat(fd, &stat_info);
		if (stat_ret == -1) {
			std::cout << "Can't stat(" << fd << ") to send as response" << std::endl;
			// something bad!
			// could 500 here
			return MHD_NO;
		}

		uint64_t size = stat_info.st_size;

		response = MHD_create_response_from_fd( size, fd );
	} else {
		response = MHD_create_response_from_buffer( strlen(page), (void*) page, MHD_RESPMEM_MUST_COPY );
	}

 	if (!response) {
		std::cout << "Nothing to send as response?" << std::endl;
 		return MHD_NO;
 	}

 	// add headers and check for default Content-Type
 	bool has_mime_type = false;
 	for( std::vector<header_t>::iterator it = headers.begin(); it != headers.end(); it++ ) {
 		MHD_add_response_header( response, it->first.c_str(), it->second.c_str() );

 		std::regex content_type_RE( "Content-Type", std::regex::basic | std::regex::icase );

 		if ( std::regex_match( it->first, content_type_RE ) )
 			has_mime_type = true;
 	}
 	if (!has_mime_type)
 		MHD_add_response_header(response, "Content-Type", default_content_type.c_str() );


 	// send cookies
 	for( std::vector<HTTP_Cookie *>::iterator it = cookies.begin(); it != cookies.end(); it++ ) {
 		MHD_add_response_header( response, "Set-Cookie", (*it)->to_string().c_str() );
 		delete (*it);
 	}


	int ret = MHD_queue_response( connection, status_code, response );

 	MHD_destroy_response( response );

 	return ret;
};


int Response::redirect( unsigned int redirect_code, std::string url ) {
	status_code = redirect_code;
	headers.push_back( header_t( "Location", url ) );
	return MHD_YES;
};


void Response::add_header( std::string header, std::string value ) {
	headers.push_back( header_t( header, value ) );
};


void Response::upgrade_protocol( std::string upgrade_proto, MHD_UpgradeHandler handler, void *cls ) {
	status_code = MHD_HTTP_SWITCHING_PROTOCOLS;
	upgrade_handler = handler;
	upgrade_handler_cls = cls;
	add_header("Upgrade", upgrade_proto);
};


int Response::upgrade_websocket( Request *req, std::string websock_proto, MHD_UpgradeHandler handler ) {
	if (  strcasecmp( req->get_header("Upgrade").c_str(), (char *)"websocket" ) != 0 ) {
		std::cout << "Upgrade to websocket requested but header != websocket: " << req->get_header("Upgrade") << std::endl;
		return MHD_NO;
	}

	if (req->get_header("Sec-WebSocket-Version") != "13") {
		std::cout << "Upgrade to websocket requested but version != 13: " << req->get_header("Sec-WebSocket-Version") << std::endl;
		return MHD_NO;
	}

	std::string websock_key = req->get_header("Sec-WebSocket-Key");
	if ( websock_key.empty() ) {
		std::cout << "Upgrade to websocket requested but key empty?" << std::endl;
		return MHD_NO;
	}

	std::string websock_protos = req->get_header("Sec-WebSocket-Protocol");
	std::regex rgx("(^|, ?)" + websock_proto + "(, ?|$)");
	// passed websock_proto needs to be in above list
	if ( !std::regex_match(websock_protos, rgx) ) {
		std::cout << "Upgrade to websocket requested but passed protocol '" << websock_protos << "' doesn't match '" << websock_proto << "'" << std::endl;
		return MHD_NO;
	}

	std::string websock_hash_message = websock_key + "258EAFA5-E914-47DA-95CA-C5AB0DC85B11";

	unsigned char websock_hash[20];
	SHA1((const unsigned char *)websock_hash_message.c_str(), websock_hash_message.length(), websock_hash);

	char *websock_hash64;
	base64_encode( &websock_hash64, (const char *)websock_hash, 20);

	void *cls = req;
	upgrade_protocol( "websocket", handler, cls);

	// add some more headers
	add_header( "Sec-WebSocket-Accept", std::string(websock_hash64) );
	add_header( "Sec-WebSocket-Protocol", websock_proto );

	free(websock_hash64);

	return MHD_YES;
};
