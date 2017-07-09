#include "DB.hpp"

#include <mysql/mysql.h>
#include <cppconn/driver.h>

#include "db_credentials.hpp"


int main() {
	// mySQL init
	mysql_library_init(0, NULL, NULL);
	sql::Driver *driver = get_driver_instance();

	sql::Connection *conn = nullptr;
	
	try {
		conn = driver->connect(DB_URI, DB_USER, DB_PASSWORD);
	} catch( sql::SQLException &e ) {
		std::cerr << "[DORM] " << e.getErrorCode() << ": " << e.what() << std::endl;
	}
	
	if (conn == nullptr) {
		std::cerr << "Can't connect to database" << std::endl;
		exit(2);
	}

	bool opt_reconnect = true;
	conn->setClientOption("OPT_RECONNECT", &opt_reconnect);
	conn->setSchema( DB_SCHEMA );

	// use existing connection
	DORM::DB::connect( conn );
	
	// create connection using credentials
	DORM::DB::connect( DB_URI, DB_USER, DB_PASSWORD, DB_SCHEMA );
	
	exit(0);
}
