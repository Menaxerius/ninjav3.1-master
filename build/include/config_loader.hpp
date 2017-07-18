#ifndef INCLUDE__CONFIG_LOADER_HPP
#define INCLUDE__CONFIG_LOADER_HPP

#include "JSON.hpp"

// the following are declared here and defined in the corresponding .cpp file
extern int DB_POOL_SIZE;
extern std::string BIND_ADDRESS;
extern std::string BIND_ADDRESS6;
extern int LISTEN_PORT;
extern std::string DOC_ROOT;

extern bool USE_SSL;
extern std::string SSL_KEY_FILENAME;
extern std::string SSL_CERT_FILENAME;
extern std::string SSL_CA_FILENAME;

extern std::string DB_URI;
extern std::string DB_SCHEMA;
extern std::string DB_USER;
extern std::string DB_PASSWORD;

extern bool HTTP_LOGGING;

extern uid_t RUN_AS_UID;
extern gid_t RUN_AS_GID;

// Connection attempt count if DB connection failed.
extern short DB_CONNECTION_ATTEMPT_COUNT;

void config_init();

// this is supplied somewhere else outside of this library
void more_config( const JSON &json );

#endif
