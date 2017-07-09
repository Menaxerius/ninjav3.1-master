#include "DB.hpp"
#include "Query.hpp"
#include "Resultset.hpp"

#include "sql/sqlEq.hpp"
#include "sql/sqlAnd.hpp"

#include "Test.hpp"

#include "db_credentials.hpp"


void test_query(DORM::Query query) {
	std::unique_ptr<DORM::Resultset> results( DORM::DB::select(query) );

	if (results)
		while( results->next() )
			std::cout << "testID: " << results->getUInt64(1) << ", name: " << results->getString(2) << ", age: " << results->getInt(3) << std::endl;
}


int main() {
	DORM::DB::connect( DB_URI, DB_USER, DB_PASSWORD, DB_SCHEMA );

	DORM::DB::execute("insert into Tests values (null, 'Fudge', 5, null, null), (null, 'Dominic', 43, null, null)");

	Test tests;
	tests.younger_than(20);

	const uint64_t found_rows = tests.search();

	std::cout << "Found rows: " << found_rows << std::endl;

	if (found_rows != 1)
		throw std::runtime_error("Incorrect number of found rows");

	while( auto test = tests.result() ) {
		std::cout << "testID: " << test->testID() << ", name: " << test->name() << ", age: " << test->age() << std::endl;
		test->foo();

		exit(0);
	}

	throw std::runtime_error("No Test records found?");
}
