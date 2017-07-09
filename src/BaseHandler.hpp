#ifndef SRC_BASEHANDLER_HPP_
#define SRC_BASEHANDLER_HPP_

#include "Handler.hpp"
#include "request_extra.hpp"

class BaseHandler: public Handler {
	public:
		request_extra extra;
		static const time_t server_start_time;
		static bool time_to_die;

		virtual Handler *route( struct MHD_Connection *connection, Request *req, Response *resp );
		virtual int process_headers( struct MHD_Connection *connection, Request *req, Response *resp );

		void global_init();
		void global_shutdown();
};

#endif
