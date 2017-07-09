#ifndef INCLUDE__RESPONSE_HPP
#define INCLUDE__RESPONSE_HPP

#include "HTTP_Cookie.hpp"
#include "Request.hpp"

#include <microhttpd.h>
#include <string>
#include <vector>


typedef std::pair< std::string, std::string > header_t;


class Response {
	private:
		MHD_UpgradeHandler			upgrade_handler;
		void						*upgrade_handler_cls;

	public:
		unsigned int				status_code;
		std::string					default_content_type;
		std::string					content;
		int							fd;
		std::vector<header_t>		headers;
		std::vector<HTTP_Cookie *>	cookies;

		Response();

		void add_cookie(HTTP_Cookie *hc);

		int send( struct MHD_Connection *connection );

		int redirect( unsigned int redirect_code, std::string url );

		void add_header( std::string header, std::string value );

		void upgrade_protocol( std::string upgrade_proto, MHD_UpgradeHandler handler, void *cls );

		int upgrade_websocket( Request *req, std::string websock_proto, MHD_UpgradeHandler handler );
};

#endif
