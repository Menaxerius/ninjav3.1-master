#include "RegistryHandler.hpp"
#include "Request.hpp"


handler_registry_t RegistryHandler::registry;


Handler *RegistryHandler::route( struct MHD_Connection *connection, Request *req, Response *resp ) {
	// is it something we handle?
	
	// map just "/" to something easier to handle, like "[root]"
	std::string url = req->url;
	if (url == "/")
		url = "[root]";
	
	// check registry first
	for( handler_registry_t::iterator it = registry.begin(); it != registry.end(); it++ ) {
		
		#ifdef HANDLER_DEBUG
			std::cout << "[RegistryHandler] Comparing URL [" << url << "] with: " << it->first << std::endl;
		#endif
	
		const int compare_length = it->first.length();

		// firstly, URL needs to be at least as long for comparing string
		if ( url.length() < compare_length)
			continue;

		// URL needs to have either / or ? or [nothing] at char after comparing string
		if ( url.length() > compare_length )
			if ( url[compare_length] != '/' && url[compare_length] != '?' )
				continue;

		if ( url.compare( 0, compare_length, it->first ) == 0 ) {
			#ifdef HANDLER_DEBUG
				std::cout << "Found matching handler!" << std::endl;
			#endif

			// handle this!
			Handler *(*handler_factory)() = it->second;
			Handler *handler = handler_factory();

			return handler;
		}
	}
	
	// return NULL if we don't handle it
	return NULL;
};


int RegistryHandler::process( struct MHD_Connection *connection, Request *req, Response *resp ) {
	if (MHD_VERSION < 0x00095401)
		return MHD_NO;	// we can't queue a 404 response as it crashes libmicrohttpd

	resp->content = "Not Found";
	resp->status_code = 404;

	return MHD_YES;
};
