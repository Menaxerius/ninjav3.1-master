#include "BaseHandler.hpp"

#include "account_updater.hpp"
#include "blockchain_monitor.hpp"
#include "blockchain_refresh.hpp"
#include "block_forger.hpp"
#include "reward_payer.hpp"
#include "bonus_processor.hpp"
#include "database_trimmer.hpp"

#include "handlers/handlers.hxx"
#include "templates/static-templates.hxx"

#include "Request.hpp"
#include "StaticHandler.hpp"
#include "StaticTemplateHandler.hpp"
#include "RegistryHandler.hpp"

#include "config.hpp"
#include "server.hpp"

#include <curl/curl.h>


const time_t BaseHandler::server_start_time = time(nullptr);
bool BaseHandler::time_to_die = false;


Handler *handler_factory() { return new BaseHandler(); }


Handler *BaseHandler::route( struct MHD_Connection *connection, Request *req, Response *resp ) {
	// everything's ok - carry on
	return new RegistryHandler;
}


int BaseHandler::process_headers( struct MHD_Connection *connection, Request *req, Response *resp ) {
	// could do extra stuff using headers

	return Handler::process_headers( connection, req, resp );
}


void BaseHandler::global_init() {
	StaticHandler::document_root = DOC_ROOT;

	// URL substrings are checked IN ORDER

	// import generated list of staff handlers...
	 using namespace Handlers;

	#include "handlers/handlers.cxx"

	RegistryHandler::register_handler<StaticHandler>( "/css" );
	RegistryHandler::register_handler<StaticHandler>( "/js" );
	RegistryHandler::register_handler<StaticHandler>( "/images" );
	RegistryHandler::register_handler<StaticHandler>( "/fonts" );
	RegistryHandler::register_handler<StaticHandler>( "/audio" );
	RegistryHandler::register_handler<StaticHandler>( "/favicon.ico" );
	RegistryHandler::register_handler<StaticHandler>( "/google" );
	RegistryHandler::register_handler<StaticHandler>( "/robots.txt" );

	RegistryHandler::register_handler<Handlers::home>( "/" );
	RegistryHandler::register_handler<Handlers::home>( "[root]" );

	// URL full-strings are checked IN ORDER
	#include "templates/static-templates.cxx"

	// extension -> mime-type mappings
	StaticHandler::register_mime_type( ".js", "text/javascript" );
	StaticHandler::register_mime_type( ".css", "text/css" );
	StaticHandler::register_mime_type( ".png", "image/png" );
	StaticHandler::register_mime_type( ".mp3", "audio/mpeg" );
	StaticHandler::register_mime_type( ".ttf", "application/x-font-truetype" );

	// init objects too!
	curl_global_init(CURL_GLOBAL_ALL);

	std::thread(blockchain_monitor).detach();
	std::thread(blockchain_refresh).detach();
	std::thread(block_forger).detach();
	std::thread(account_updater).detach();
	std::thread(reward_payer).detach();
	std::thread(bonus_processor).detach();
	std::thread(database_trimmer).detach();
}


void BaseHandler::global_shutdown() {
	time_to_die = true;

	curl_global_cleanup();
}
