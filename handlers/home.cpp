#include "home.hpp"

#include "templates/home.hxx"

#include "config.hpp"


int Handlers::home::process( struct MHD_Connection *connection, Request *req, Response *resp ) {
	Templates::home home_template(req);

	resp->status_code = 200;
	resp->content = home_template.render();

	// done!
	return MHD_YES;
}
