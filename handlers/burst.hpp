#ifndef HANDLERS__BURST_HPP
#define HANDLERS__BURST_HPP

#include "Handler.hpp"


namespace Handlers {

		class burst: public Handler {
			public:
				virtual int process( struct MHD_Connection *connection, Request *req, Response *resp );

				virtual Handler *route( struct MHD_Connection *connection, Request *req, Response *resp );
		};

} // Handlers namespace


#endif
