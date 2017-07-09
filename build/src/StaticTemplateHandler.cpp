#include "StaticTemplateHandler.hpp"
#include "Template.hpp"


template_registry_t StaticTemplateHandler::registry;


int StaticTemplateHandler::process( struct MHD_Connection *connection, Request *req, Response *resp ) {
	// pick template
	Template *t = NULL;

	// check registry
	for( template_registry_t::iterator it = registry.begin(); t == NULL && it != registry.end(); it++ ) {
		
		#ifdef HANDLER_DEBUG
			std::cout << "[StaticTemplateHandler] Comparing URL [" << req->url << "] with: " << it->first << std::endl;
		#endif
	
		if ( req->url.compare( 0, req->url.length(), it->first ) == 0 ) {
			#ifdef HANDLER_DEBUG
				std::cout << "Found matching template!" << std::endl;
			#endif
		
			// handle this!
			template_factory_t template_factory = it->second;
			t = template_factory( req );
		}
	}	

	if (t == NULL) {
		// 404?
		return MHD_NO;
	}

	// render template to string
	resp->status_code = 200;
	resp->content = t->render();
	
	delete t;
	
	// done!
	return MHD_YES;
};
