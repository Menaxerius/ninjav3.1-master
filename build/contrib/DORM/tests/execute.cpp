#include "DB.hpp"

#include "db_credentials.hpp"


int main() {
	DORM::DB::connect( DB_URI, DB_USER, DB_PASSWORD, DB_SCHEMA );

	DORM::DB::execute("drop table if exists conntest");
	DORM::DB::execute("create temporary table conntest ( blah int )");
	
	DORM::DB::execute("optimize table conntest");
	DORM::DB::execute("optimize table conntest");	// this one might fail
	
	exit(0);
}
