#include "DB.hpp"
#include "Query.hpp"
#include "Resultset.hpp"

#include "sql/sqlEq.hpp"
#include "sql/sqlAnd.hpp"

#include "db_credentials.hpp"


void test_query(DORM::Query query) {
	std::unique_ptr<DORM::Resultset> results( DORM::DB::select(query) );

	if (results)
		while( results->next() )
			std::cout << "Name: " << results->getString(2) << ", age: " << results->getInt(3) << std::endl;
}


void test() {
	DORM::DB::connect( DB_URI, DB_USER, DB_PASSWORD, DB_SCHEMA );

	DORM::DB::execute("insert into Tests values (null, 'Dom', 43, null, null),(null, 'Fudge', 5, null, null)");

	DORM::Query query;
	query.cols.push_back("*");
	query.tables = DORM::Tables("Tests");

	test_query(query);

	DORM::sqlEq<int> age_eq("age", 5);
	DORM::DB::deleterow("Tests", age_eq);

	test_query(query);
}


int main() {
	test();

	exit(0);
}
