#ifndef HANDLERS__HOME_CPP
#define HANDLERS__HOME_CPP

#include "Handler.hpp"


namespace Handlers {

		class home: public Handler {
			public:
				virtual int process( struct MHD_Connection *connection, Request *req, Response *resp );
		};

} // Handlers namespace


#endif
