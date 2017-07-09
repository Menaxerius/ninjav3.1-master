#include "StaticHandler.hpp"

#define MIN_TTL 3600

// for open()
#include <fcntl.h>
#include <time.h>

#include <sstream>

#ifdef FOLLOW_SYMLINKS
#define OPEN_FLAGS O_RDONLY
#define FILE_TYPES ( S_IFREG | S_IFLNK )
#else
#define OPEN_FLAGS O_RDONLY | O_NOFOLLOW
#define FILE_TYPES S_IFREG
#endif


std::string StaticHandler::document_root;
std::string StaticHandler::root_URL;

mime_type_map_t StaticHandler::mime_type_map;


int StaticHandler::untaint_url( std::string tainted_url ) {
	// !!! DANGEROUS - NEEDS CHECKING !!! //
	
	// special case for accessing [root] or "/"
	if ( !root_URL.empty() && ( tainted_url == "[root]" || tainted_url == "/" ) )
		tainted_url = root_URL;

	// CHECK: do we need to unescape URL?
	
	// CHECK: no ../ allowed
	if ( tainted_url.find( "../" ) != std::string::npos ) {
		// 400 bad request?
		return MHD_NO;
	}
	
	tainted_url = document_root + tainted_url;
	
	int res = lstat( tainted_url.c_str(), &file_sb );
	if (res == -1) {
		// couldn't stat?
		// could return 404 FILE NOT FOUND
		// or special 404 page
		return MHD_NO;
	}
	if ( (file_sb.st_mode & FILE_TYPES) == 0) {
		// not regular file?
		// could return 404 FILE NOT FOUND
		// or special 404 page
		return MHD_NO;
	}

	safe_url = tainted_url;

	// generate e-tag
	e_tag = std::to_string(file_sb.st_ino) + "-" + std::to_string(file_sb.st_mtim.tv_sec);

	return MHD_YES;
}



int StaticHandler::process_headers( struct MHD_Connection *connection, Request *req, Response *resp ) {
	if ( untaint_url( req->url) == MHD_NO )
		return MHD_NO;

	std::string etag_to_check = req->get_header("If-None-Match");

	if ( !etag_to_check.empty() ) {
		if (etag_to_check == e_tag) {
			// std::cout << "StaticHandler: matching ETag '" << e_tag << "' for " << req->url << std::endl;
			resp->status_code = 304;
			return MHD_YES;
		}
	}

	return MHD_YES;
}


int StaticHandler::process( struct MHD_Connection *connection, Request *req, Response *resp ) {
	// spew static content

	int fd = open(safe_url.c_str(), OPEN_FLAGS );
	if (fd == -1) {
		// file not found?
		// could return 404 FILE NOT FOUND
		// or special 404 page
		return MHD_NO;
	}

	resp->status_code = 200;
	resp->fd = fd;

	// send e-tag
	resp->headers.push_back( header_t( "ETag", e_tag) );

 	// add some basic caching!
 	std::string cache_control = "public, max-age=" + std::to_string(MIN_TTL);
 	resp->headers.push_back( header_t( "Cache-Control", cache_control ) );
 	
 	const time_t now = time(NULL) + MIN_TTL;
 	struct tm *now_tm = gmtime( &now );
 	char expires_date[40];
 	strftime( expires_date, sizeof(expires_date), "%a, %d %b %Y %T GMT", now_tm );

 	resp->headers.push_back( header_t( "Expires", expires_date ) );

 	// mime-type ?
	std::size_t pos = safe_url.find_last_of( "." );
	if (pos != std::string::npos) {
		std::string mime_type = mime_type_map[ safe_url.substr( pos, safe_url.length() - pos ) ];
		if (mime_type != "")
			resp->headers.push_back( header_t( "Content-Type", mime_type ) );
	}

 	return MHD_YES;
};


void StaticHandler::register_mime_type( std::string ext, std::string mime_type ) {
	mime_type_map[ ext ] = mime_type;
};

