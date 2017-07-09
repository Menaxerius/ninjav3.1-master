#ifndef HANDLERS__XHR__GETRECENTBLOCKS_HPP
#define HANDLERS__XHR__GETRECENTBLOCKS_HPP

#include "Handler.hpp"


namespace Handlers {
	namespace XHR {

		class getRecentBlocks: public Handler {
			public:
				virtual int process( struct MHD_Connection *connection, Request *req, Response *resp );
		};

	} // XHR namespace
} // Handlers namespace


#endif
