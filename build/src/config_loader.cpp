#include "config_loader.hpp"

#include <iostream>
#include <fstream>
#include <memory>
#include <cstring>

#include <pwd.h>
#include <grp.h>


const std::string CONFIG_FILENAME = "config.js";

// actual config storage with some defaults
int DB_POOL_SIZE = 10;
std::string BIND_ADDRESS;
std::string BIND_ADDRESS6;
int LISTEN_PORT = 9999;
std::string DOC_ROOT = "static";

bool USE_SSL = false;
std::string SSL_KEY_FILENAME;
std::string SSL_CERT_FILENAME;
std::string SSL_CA_FILENAME;

std::string DB_URI = "unix:///tmp/mysql.sock";
std::string DB_SCHEMA = "test";
std::string DB_USER = "root";
std::string DB_PASSWORD = "";

bool HTTP_LOGGING = false;

uid_t RUN_AS_UID = -1;
gid_t RUN_AS_GID = -1;

short DB_CONNECTION_ATTEMPT_COUNT = 5;

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
		std::cerr << "Can't open " << src_filename << ": " << std::strerror(errno) << std::endl;
		exit(2);
	}
}


void config_init() {
	std::string config = read_file(CONFIG_FILENAME);

	try {
		JSON json = JSON(config);

		if ( json.null("bindAddress") && json.null("bindAddress6") ) {
			std::cerr << "Config file missing 'bindAddress' or 'bindAddress6'" << std::endl;
			exit(2);
		}

		if ( !json.null("bindAddress") )
			BIND_ADDRESS = json.get_string("bindAddress");
		if ( !json.null("bindAddress6") )
			BIND_ADDRESS6 = json.get_string("bindAddress6");

		LISTEN_PORT = json.get_number("listenPort");
		DOC_ROOT = json.get_string("documentRoot");

		if ( !json.null("useSSL") && json.get_boolean("useSSL") ) {
			USE_SSL = true;

			SSL_KEY_FILENAME = json.get_string("keyPEM");
			SSL_CERT_FILENAME = json.get_string("certPEM");
			SSL_CA_FILENAME = json.get_string("CAPEM");
		}

		DB_POOL_SIZE = json.get_number("databasePoolSize");
		DB_URI = json.get_string("databaseURI");
		DB_SCHEMA = json.get_string("databaseSchema");
		DB_USER = json.get_string("databaseUser");
		DB_PASSWORD = json.get_string("databasePassword");
	
		if ( !json.null("httpLogging") )
			HTTP_LOGGING = json.get_boolean("httpLogging");

		if ( !json.null("runAsUser") ) {
			const std::string run_as_user = json.get_string("runAsUser");

			struct passwd *passwd_entry = getpwnam( run_as_user.c_str() );
			endpwent();

			if (passwd_entry == nullptr) {
				std::cerr << "Can't map username '" << run_as_user << "' to UID" << std::endl;
				exit(2);
			}

			RUN_AS_UID = passwd_entry->pw_uid;

			// separate group?
			if ( !json.null("runAsGroup") ) {
				const std::string run_as_group = json.get_string("runAsGroup");

				struct group *group_entry = getgrnam( run_as_group.c_str() );
				endgrent();

				if (group_entry == nullptr) {
					std::cerr << "Can't map group '" << run_as_group << "' to GID" << std::endl;
					exit(2);
				}

				RUN_AS_GID = group_entry->gr_gid;
			} else {
				RUN_AS_GID = passwd_entry->pw_gid;
			}
		}

		// pass on to application-specific config handler
		more_config(json);
	} catch(const std::exception &e) {
		std::cerr << e.what() << std::endl;
		exit(2);
	}
}
