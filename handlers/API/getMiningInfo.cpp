#include "API/getMiningInfo.hpp"

#include "BlockCache.hpp"


int Handlers::API::getMiningInfo::inner( struct MHD_Connection *connection, Request *req, Response *resp ) {
	resp->status_code = 200;
	resp->content = BlockCache::get_mining_info();

	return MHD_YES;
}


int Handlers::API::getMiningInfo::process( struct MHD_Connection *connection, Request *req, Response *resp ) {
	return inner(connection, req, resp);
}
