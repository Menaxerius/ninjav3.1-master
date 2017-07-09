#ifndef INCLUDE__HANDLER_HPP
#define INCLUDE__HANDLER_HPP

#include <microhttpd.h>
#include <string>

#include "Request.hpp"
#include "Response.hpp"
#include "ReqResp.hpp"

class Handler {
	public:
		virtual ~Handler() { };

		virtual Handler *route( struct MHD_Connection *connection, Request *req, Response *resp );
	
		virtual int post_processor( void *coninfo_cls, enum MHD_ValueKind kind, const char *key, const char *filename,
									const char *content_type, const char *transfer_encoding, const char *data, uint64_t off, size_t size );

		virtual uint64_t max_raw_post_data_size();
		virtual int raw_post_processor( void *coninfo_cls, const char *upload_data, size_t *upload_data_size );
		
		virtual int process_headers( struct MHD_Connection *connection, Request *req, Response *resp );
		virtual int process( struct MHD_Connection *connection, Request *req, Response *resp );
		
		virtual void cleanup() { };

		virtual void global_init() { };
		virtual void global_shutdown() { };
};

#endif
