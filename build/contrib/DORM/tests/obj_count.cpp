#include "DB.hpp"

#include "Test.hpp"

#include "db_credentials.hpp"


int main() {
	DORM::DB::connect( DB_URI, DB_USER, DB_PASSWORD, DB_SCHEMA );

	DORM::DB::execute("insert into Tests values (null, 'Fudge', 5, null, null), (null, 'Dominic', 43, null, null)");

	Test tests;

	const uint64_t row_count = tests.count();

	std::cout << "Row count: " << row_count << std::endl;

	if (row_count != 2)
		throw std::runtime_error("Wrong row count");

	exit(0);
}
