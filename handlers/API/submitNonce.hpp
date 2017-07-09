#ifndef HANDLERS__API__SUBMITNONCE_HPP
#define HANDLERS__API__SUBMITNONCE_HPP

#include "Handler.hpp"


namespace Handlers {
	namespace API {

		class submitNonce: public Handler {
			public:
				virtual int process( struct MHD_Connection *connection, Request *req, Response *resp );
				static int inner( struct MHD_Connection *connection, Request *req, Response *resp );
		};

	} // API namespace
} // Handlers namespace


#endif
