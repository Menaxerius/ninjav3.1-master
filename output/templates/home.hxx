#ifndef __TEMPLATE__home
#define __TEMPLATE__home

#include "Template.hpp"

// include config.hpp to allow support for extra features
#include "config.hpp"


using std::string;

namespace Templates {

	class home: public Template {
		public:
			#ifdef TEMPLATE_DECL
				TEMPLATE_DECL
			#endif

			home( Request *incoming_req ): Template(incoming_req) { 
				#ifdef TEMPLATE_INIT
					TEMPLATE_INIT
				#endif
			};

			std::string render();
	};
};

#endif
