#include "DB.hpp"
#include "Query.hpp"

#include <mysql/mysql.h>
#include <cppconn/driver.h>

#include <thread>
#include <sstream>

#include "db_credentials.hpp"


void processlist( std::string prefix = "") {
	DORM::Query query;
	query.cols.push_back( "count(*) FROM information_schema.PROCESSLIST" );
	
	std::stringstream ss;
	ss << prefix << "Connections: " << DORM::DB::fetch_int64(query) << std::endl;
	std::cout << ss.str();
}


void new_db_thread(int i) {
	std::stringstream ss;
	ss << "[" << i << "] Thread START" << std::endl;
	std::cout << ss.str();

	DORM::DB::check_connection();

	processlist( "[" + std::to_string(i) + "] " );

	ss.str("");
	ss << "[" << i << "] Thread END" << std::endl;
	std::cout << ss.str();
}


int main() {
	std::cout << "[MAIN] Start" << std::endl;

	DORM::DB::connect( DB_URI, DB_USER, DB_PASSWORD, DB_SCHEMA );
	
	processlist("[MAIN] ");
	
	std::cout << "[MAIN] Connected - creating threads" << std::endl;
	
	std::vector<std::thread> threads;
	
	for(int i=0; i<3; i++) {
		std::thread t( new_db_thread, i );
		threads.push_back( std::move(t) );
	}

	std::cout << "[MAIN] Waiting for threads" << std::endl;

	for(int i=0; i<threads.size(); i++) {
		std::thread &t( threads[i] );
		t.join();
		
		processlist("[MAIN] ");
	}
	
	std::cout << "[MAIN] Exit" << std::endl;
		
	exit(0);
}
