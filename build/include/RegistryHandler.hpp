#ifndef INCLUDE__REGISTRYHANDLER_HPP
#define INCLUDE__REGISTRYHANDLER_HPP


#include "Handler.hpp"

#include <list>
#include <string>

typedef std::pair<std::string, Handler *(*)()> handler_registry_tuple;
typedef std::list<handler_registry_tuple> handler_registry_t;


template <class HANDLER>
Handler *handler_factory() {
	return new HANDLER;
};


class RegistryHandler: public Handler {
	private:
		static handler_registry_t registry;

	public:
		virtual Handler *route( struct MHD_Connection *connection, Request *req, Response *resp );
	
		virtual int process( struct MHD_Connection *connection, Request *req, Response *resp );
		
		template <class HANDLER>
		static void register_handler( std::string url_prefix ) {
			// shove into registry
			registry.push_back( handler_registry_tuple( url_prefix, &handler_factory<HANDLER> ) );
		};
};


#endif
