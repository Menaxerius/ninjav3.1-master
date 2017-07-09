#include "DB.hpp"
#include "Query.hpp"
#include "Resultset.hpp"

#include "sql/sqlEq.hpp"
#include "sql/sqlAnd.hpp"

#include "db_credentials.hpp"


void test() {
	DORM::DB::connect( DB_URI, DB_USER, DB_PASSWORD, DB_SCHEMA );

	DORM::DB::execute("insert into Tests values (null, 'Dom', 43, null, null)");

	DORM::sqlEq<int> age_eq("age", 5);
	DORM::sqlEq<std::string> name_eq("name", "Fudge");

	std::vector< SPC<DORM::Where> > inserts{ age_eq.make_shared(), name_eq.make_shared() };

	DORM::DB::writerow("Tests", inserts);
	
	DORM::Query query;
	query.cols.push_back("*");
	query.tables = DORM::Tables("Tests");

	std::unique_ptr<DORM::Resultset> results( DORM::DB::select(query) );

	int n_rows = 0;
	
	if (results)
		while( results->next() ) {
			++n_rows;
			std::cout << "Name: " << results->getString(2) << ", age: " << results->getInt(3) << std::endl;
		}
	
	if (n_rows != 2)
		throw std::runtime_error("Incorrect number of rows found");
}


int main() {
	test();

	exit(0);
}
