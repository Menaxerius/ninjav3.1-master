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
			std::cout << "testID: " << results->getUInt64(1) << ", name: " << results->getString(2) << ", age: " << results->getInt(3)
						<< ", simple_time: " << results->getString(4) << ", complex_time: " << results->getString(5) << std::endl;
}


int main() {
	DORM::DB::connect( DB_URI, DB_USER, DB_PASSWORD, DB_SCHEMA );

	struct timeval tv = { time(nullptr), 123456 };

	Test t;
	t.name("Fudge");
	t.age(5);
	t.simple_time( time(nullptr) );
	t.complex_time( tv );
	t.save();

	DORM::Query query;
	query.cols.push_back("*");
	DORM::Tables test_tables("Tests");
	query.tables = test_tables;

	test_query(query);

	auto loaded_test = Test::load( t.testID() );

	if ( loaded_test->simple_time() != t.simple_time() )
		throw std::runtime_error("simple time mismatch");

	if ( loaded_test->complex_time() != t.complex_time() )
		throw std::runtime_error("complex time mismatch");

	t.name("Sludge");
	t.age(6);
	t.simple_time( time(nullptr) );
	t.complex_time( time(nullptr) );
	t.save();

	std::cout << "t's autoinc column value: " << t.testID() << std::endl;

	test_query(query);

	loaded_test = Test::load( t.testID() );

	if ( loaded_test->simple_time() != t.simple_time() )
		throw std::runtime_error("simple time mismatch");

	if ( loaded_test->complex_time() != t.complex_time() )
		throw std::runtime_error("complex time mismatch");

	exit(0);
}
