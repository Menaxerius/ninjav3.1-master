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

	Test t;
	t.name("Fudge");
	t.age(5);
	t.save();

	std::cout << "t's autoinc column value: " << t.testID() << std::endl;

	DORM::Query query;
	query.cols.push_back("*");
	DORM::Tables test_tables("Tests");
	query.tables = test_tables;

	test_query(query);

	// change "name" column without object knowing
	DORM::DB::execute("UPDATE Tests SET name='Budge'");

	t.age(6);
	t.save();

	test_query(query);

	auto loaded_test = Test::load(1);	// should be testID for first row given fresh, blank database

	if (loaded_test->name() != "Budge")
		throw std::runtime_error("Name column incorrectly overwritten");

	exit(0);
}
