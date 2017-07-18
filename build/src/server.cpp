#include "Request.hpp"
#include "RegistryHandler.hpp"
#include "ReqResp.hpp"

#include "ftime.hpp"
#include "server.hpp"
#include "config_loader.hpp"

#include "DORM/DB.hpp"

#include <microhttpd.h>
#include <iostream>
#include <sstream>
#include <fstream>


#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <signal.h>
#ifdef __FreeBSD__
	#include <sys/signalvar.h>
#endif
#include <fcntl.h>
#include <unistd.h>
#include <grp.h>

#include <limits.h>

#include "pthread_np_shim.hpp"



static const uint64_t POST_BUFFER_SIZE = 65536;
static volatile bool time_to_die = false;


static int post_processor( void *coninfo_cls, enum MHD_ValueKind kind, const char *key, const char *filename, const char *content_type,
							const char *transfer_encoding, const char *data, uint64_t off, size_t size ) {
	// reroute to object call
	ReqResp *req_resp = (ReqResp *) coninfo_cls;
	
	return req_resp->req->handler->post_processor( coninfo_cls, kind, key, filename, content_type, transfer_encoding, data, off, size);
}


static int access_handler_initial( void *cls, struct MHD_Connection *connection, const char *url, const char *method, const char *version,
									const char *upload_data, size_t *upload_data_size, void **con_cls ) {
	// log it?
	// std::cout << ftime() << "request (connection=0x" << std::hex << (uint64_t)connection << "): " << std::dec << method << " " << url << std::endl;
	// std::cout << ftime() << "request: " << method << " " << url << std::endl;

	// store request
	Request *req = new Request();

	if ( strcmp(method, MHD_HTTP_METHOD_GET) == 0 ) {
		req->method = GET;
	} else if ( strcmp(method, MHD_HTTP_METHOD_POST) == 0 ) {
		req->method = POST;
	} else {
		// HTTP method not supported
		delete req;
		std::cout << ftime() << method << " not supported" << std::endl;
		
		// could send 400 BAD REQUEST?
		return MHD_NO;
	}

	pthread_set_name_np( pthread_self(), method );

	req->url = url;
	std::istringstream(version) >> req->version;

	// allocate DB connection
	// this can block!
	try {
		DORM::DB::check_connection();
	} catch (const DORM::DB::connection_issue &e) {
		// Sorry, too busy!
        std::cerr  << ftime() << "[server::access_handler_initial] Too many connections! " << e.getErrorCode() << ": " << e.what() << std::endl;
		char error[] = "Sorry, too busy - try again in a moment";

		struct MHD_Response *response = MHD_create_response_from_buffer( strlen(error), (void*) error, MHD_RESPMEM_MUST_COPY );
		MHD_queue_response( connection, 503, response );
	 	MHD_destroy_response( response );
	 	return MHD_YES;
	} catch(const sql::SQLException &e) {
        // Could not connect to db.
        std::cerr << ftime() << "[server::access_handler_initial] " << e.what() << std::endl;
        throw e;
    }

	// parse cookies
	req->parse_cookies( connection );

	// parse headers
	req->parse_headers( connection );

	// parse query string (regardless of HTTP mode)
	req->parse_query( connection );

	// assign default handler
	req->base_handler = handler_factory();

	// responses can be created from here on
	Response *resp = new Response;

	// allow base handler to process headers first
	int result = req->base_handler->process_headers( connection, req, resp );
	if (result == MHD_NO) {
		std::cout << ftime() << "base-handler HEADERS FAIL: " << url << std::endl;

		delete req;
		delete resp;

		return MHD_NO;
	}

	// call handler to determine what to do with URL
	Handler *handler = req->base_handler->route( connection, req, resp );

	// we need an initial handler for URL
	if (handler == nullptr) {
		std::cout << ftime() << "no initial handler for " << url << std::endl;

		delete req;
		delete resp;
		
		// could send 404 NOT FOUND?
		// or special 404 page
		return MHD_NO;
	}

	// recursive routing decisions
	while( Handler *new_handler = handler->route( connection, req, resp ) ) {
		delete handler;

		handler = new_handler;
	}
	// final answer
	req->handler = handler;

	// needs to be a header-only-processing hook here
	result = req->handler->process_headers( connection, req, resp );
	if (result == MHD_NO) {
		std::cout << ftime() << "HEADERS FAIL: " << url << std::endl;

		delete req;
		delete resp;

		return MHD_NO;
	}
	// short circuit response?
	if (resp->status_code) {
		if (HTTP_LOGGING)
			std::cout << ftime() << "HEADERS OK/" << resp->status_code << ": " << url << std::endl;
			
		result = resp->send( connection );

		delete req;
		delete resp;

		return MHD_YES;

	}

	// we're continuing so save state
	ReqResp *req_resp = new ReqResp( req, resp );

	// if this is a POST request, we need to make a post-processor
	if (req->method == POST ) {
		req->post_processor = MHD_create_post_processor( connection, POST_BUFFER_SIZE, post_processor, (void *) req_resp );
		
		if (req->post_processor == nullptr) {
			// does this handler accept raw post data?
			uint64_t max_raw_post_data_size = req->handler->max_raw_post_data_size();

			if ( max_raw_post_data_size > 0 ) {
				// do we have a content-length? if so, check length is within limits
				std::string content_length_s = req->get_header("Content-Length");

				if ( !content_length_s.empty() ) {
					uint64_t content_length = strtoull( content_length_s.c_str(), nullptr, 10 );

					if ( content_length > 0 || content_length <= max_raw_post_data_size ) {
						*con_cls = (void *) req_resp;
						return MHD_YES;
					}
				}
			}

			// fall-through failure
			std::cout << ftime() << "couldn't create POST processor for " << url << std::endl;

			delete req_resp;
			delete req;
			delete resp;

			// could send 500 INTERNAL SERVER ERROR?
			// or special 500 page
			return MHD_NO;
		}
	}

	*con_cls = (void *) req_resp;

	return result;
}


static int access_handler_next( void *cls, struct MHD_Connection *connection, const char *url, const char *method, const char *version,
									const char *upload_data, size_t *upload_data_size, void **con_cls ) {
	// short circuited from before?
	if (*con_cls == nullptr)
		return MHD_YES;

	// continuing previous request...
	ReqResp *req_resp = (ReqResp *) *con_cls;
	
	Request *req = req_resp->req;
	Response *resp = req_resp->resp;

	if (*upload_data_size != 0) {
		// not in a POST?
		if ( req->method != POST ) {
			std::cout << ftime() << "upload data present for non-POST method " << method << std::endl;

			delete req_resp;
			delete req;
			delete resp;
			
			// could return 400 BAD REQUEST?
			return MHD_NO;
		}

		// process next chunk...
		if ( req->post_processor ) {
			MHD_post_process( req->post_processor, upload_data, *upload_data_size );
		} else if ( req->handler->max_raw_post_data_size() ) {
			// raw POST data accepted
			if (req->handler->raw_post_processor( req_resp, upload_data, upload_data_size ) == MHD_NO)
				return MHD_NO;
		}
		
		*upload_data_size = 0;
          
		return MHD_YES;
	} 

	// we are done uploading
	// GOTCHA: destroying post process may call post_processor callback one last time!
	if (req->method == POST) {
		if (req->post_processor) {
			MHD_destroy_post_processor( req->post_processor );
			req->post_processor = nullptr;

			// signal to handler's post processor that all is done
			if (req->handler->post_processor( req_resp, MHD_POSTDATA_KIND, "", "", "", "", nullptr, 0, 0) == MHD_NO)
				return MHD_NO;
		} else if ( req->handler->max_raw_post_data_size() ) {
			// signal to handler's raw post processor that all is done
			if (req->handler->raw_post_processor( req_resp, nullptr, nullptr ) == MHD_NO)
				return MHD_NO;
		}
	}

	// actual processing
	// std::cout << ftime() << "processing: " << url << std::endl;

	int result = MHD_NO;
	try {
		result = req->handler->process( connection, req, resp );
	} catch (...) {
		// result is NO
	};

	if (result == MHD_YES) {
		if (HTTP_LOGGING)
			std::cout << ftime() << "OK/" << resp->status_code << " " << method << " " << url << std::endl;
			
		result = resp->send( connection );
	} else {
		std::cout << ftime() << "FAIL: " << method << " " << url << std::endl;
	}

	return result;
}


static int access_handler( void *cls, struct MHD_Connection *connection, const char *url, const char *method, const char *version,
							const char *upload_data, size_t *upload_data_size, void **con_cls ) {
	// headers only so far?
	if (*con_cls == nullptr)
		return access_handler_initial( cls, connection, url, method, version, upload_data, upload_data_size, con_cls );
	else
		return access_handler_next( cls, connection, url, method, version, upload_data, upload_data_size, con_cls );
}


static void request_completed( void *cls, struct MHD_Connection *connection, void **con_cls, enum MHD_RequestTerminationCode toe ) {
	// nothing to do?
	if ( (*con_cls) == nullptr)
		return;
		
	ReqResp *req_resp = (ReqResp *) *con_cls;
	
	Request *req = req_resp->req;
	Response *resp = req_resp->resp;
	
	if (req->handler)
		req->handler->cleanup();

	if (req->base_handler)
		req->base_handler->cleanup();

	delete req_resp;
	delete req;
	delete resp;
	
	*con_cls = nullptr;
}


static void handle_signal(int sig) {
	std::cout << ftime() << "!!! Received sig " << sig << " in thread " << pthread_getthreadid_np() << "!!!" << std::endl;
	time_to_die = true;
}



static std::string read_file(const std::string &src_filename) {
	try {
		std::ifstream fs;
		fs.exceptions ( std::ifstream::failbit | std::ifstream::badbit );
		fs.open(src_filename, std::ifstream::in | std::ifstream::ate);

		auto size = fs.tellg();

		auto buffer = std::make_unique<char[]>( size );

		fs.seekg(0);
		fs.read( buffer.get(), size );
		fs.close();

		return std::string( buffer.get(), size );
	} catch(const std::ifstream::failure& e) {
		std::cerr << "Can't open " << src_filename << ": " << strerror(errno) << std::endl;
		exit(2);
	}
}


static struct MHD_Daemon *init_daemon() {
	unsigned int flags = MHD_USE_THREAD_PER_CONNECTION | MHD_USE_DEBUG | MHD_USE_POLL | MHD_USE_ITC | MHD_USE_INTERNAL_POLLING_THREAD | MHD_ALLOW_UPGRADE;

	int pton_error;
	struct sockaddr_in6 sock_addr6;
	struct sockaddr_in sock_addr4;
	struct sockaddr *sock_addr;

	if ( !BIND_ADDRESS6.empty() ) {
		flags |= MHD_USE_IPv6;

		memset(&sock_addr6, 0, sizeof(sock_addr6));

		sock_addr6.sin6_family = AF_INET6;
		sock_addr6.sin6_port = htons(LISTEN_PORT);

		pton_error = inet_pton( AF_INET6, BIND_ADDRESS6.c_str(), &sock_addr6.sin6_addr );

		sock_addr = (struct sockaddr *)&sock_addr6;
	} else {
		memset(&sock_addr4, 0, sizeof(sock_addr4));

		sock_addr4.sin_family = AF_INET;
		sock_addr4.sin_port = htons(LISTEN_PORT);

		pton_error = inet_pton( AF_INET, BIND_ADDRESS.c_str(), &sock_addr4.sin_addr );

		sock_addr = (struct sockaddr *)&sock_addr4;
	}

	if (!pton_error) {
		std::cerr << "Couldn't parse bind address" << std::endl;
		exit(2);
	}

	if (pton_error == -1) {
		perror("inet_pton");
		exit(2);
	}

	std::string key_pem;
	std::string cert_pem;
	std::string ca_pem;

	if (USE_SSL) {
		flags |= MHD_USE_SSL;

		key_pem = read_file(SSL_KEY_FILENAME);
		cert_pem = read_file(SSL_CERT_FILENAME);
		ca_pem = read_file(SSL_CA_FILENAME);
	}

	unsigned short port = LISTEN_PORT;
	
	// callback to call to check which clients will be allowed to connect
	MHD_AcceptPolicyCallback apc = nullptr;
	void *apc_cls = nullptr;
	
	// default handler for all URIs
	MHD_AccessHandlerCallback dh = &access_handler;
	void *dh_cls = nullptr;
	
	// broken? (see below)
	// unsigned int conn_timeout = 30;
    // default value was 4096
    unsigned int conn_limit = UINT_MAX;
    // default value was 1024
    unsigned int listen_queue_size = 10240;

	if (USE_SSL)
		return MHD_start_daemon( flags, port, apc, apc_cls, dh, dh_cls,
									MHD_OPTION_NOTIFY_COMPLETED, &request_completed, nullptr,
									MHD_OPTION_SOCK_ADDR, sock_addr,
									// Broken? MHD_OPTION_CONNECTION_TIMEOUT, &conn_timeout,
									MHD_OPTION_CONNECTION_LIMIT, &conn_limit,
									MHD_OPTION_LISTEN_BACKLOG_SIZE, &listen_queue_size,
									MHD_OPTION_HTTPS_MEM_KEY, key_pem.c_str(),
									MHD_OPTION_HTTPS_MEM_CERT, cert_pem.c_str(),
									MHD_OPTION_HTTPS_MEM_TRUST, ca_pem.c_str(),
									MHD_OPTION_END );
	else
		return MHD_start_daemon( flags, port, apc, apc_cls, dh, dh_cls,
									MHD_OPTION_NOTIFY_COMPLETED, &request_completed, nullptr,
									MHD_OPTION_SOCK_ADDR, sock_addr,
									// Broken? MHD_OPTION_CONNECTION_TIMEOUT, &conn_timeout,
									MHD_OPTION_CONNECTION_LIMIT, &conn_limit,
									MHD_OPTION_LISTEN_BACKLOG_SIZE, &listen_queue_size,
									MHD_OPTION_END );
}


int main(int argc, char **argv, char **envp) {
	close(0);

	config_init();

	// drop privileges?
	if (RUN_AS_UID != -1) {
		if (setgid(RUN_AS_GID) != 0) {
			perror("Can't set GID");
			exit(2);
		}

		// drop all supplimental groups
		const gid_t gidset[1] = { RUN_AS_GID };
		if (setgroups(1, gidset) != 0) {
			perror("Can't set groups");
			exit(2);
		}

		if (setuid(RUN_AS_UID) != 0) {
			perror("Can't set UID");
			exit(2);
		}
	}

	std::cout << ftime() << "--- START ---" << std::endl;

	signal( SIGCHLD, SIG_IGN );
	signal( SIGINT, handle_signal );
	signal( SIGTERM, handle_signal );

	// block SIGINT and SIGTERM for all threads
	sigset_t sigs;
	sigaddset(&sigs, SIGINT);
	sigaddset(&sigs, SIGTERM);
	pthread_sigmask( SIG_BLOCK, &sigs, nullptr );

	// uses config
    // Check db connection
    short counter = 0;
    while (true){
        try{
            DORM::DB::connect( DB_URI, DB_USER, DB_PASSWORD, DB_SCHEMA );
            break;
        } catch (const DORM::DB::connection_issue &e) {
            // DB being hammered by miners - try again in a moment
            std::cerr  << ftime() << "[server::server] Too many connections! " << e.getErrorCode() << ": " << e.what() << std::endl;
            sleep(1);
        } catch(const sql::SQLException &e) {
            // Could not connect to db.
            std::cerr << ftime() << "[server::server] " << e.what() << std::endl;
            sleep(1);
        }
        std::cerr << ftime() << "[server::server] Trying to connect in a moment. Attempt: " << counter + 1 <<  std::endl;
        ++counter;
        if(counter + 1 == DB_CONNECTION_ATTEMPT_COUNT){
            std::cerr << ftime() << "[server::server] DB connect failed..." << std::endl;
            throw;
        }
    }

	Handler *base_handler = handler_factory();
	base_handler->global_init();

	struct MHD_Daemon *daemon = init_daemon();
	if (daemon == nullptr) {
		std::cerr << ftime() << "Can't start daemon!" << std::endl;
		return 1;
	}

	// reallow SIGINT and SIGTERM just for us
	pthread_sigmask( SIG_UNBLOCK, &sigs, nullptr );

	pthread_set_name_np( pthread_self(), "main" );

	std::cout << ftime() << "Server ready!" << std::endl;

	while(!time_to_die) {
		sleep(1);
	}

	// clean up
	std::cout << ftime() << "Server shutting down..." << std::endl;

	int listen_fd = MHD_quiesce_daemon(daemon);

	base_handler->global_shutdown();
	delete base_handler;

	MHD_stop_daemon( daemon );

	close(listen_fd);

	std::cout << ftime() << "Server shut down!" << std::endl;

	return 0;
}
