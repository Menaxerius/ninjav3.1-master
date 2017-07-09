#include "DB.hpp"
#include "Transaction.hpp"
#include "Query.hpp"

#include "db_credentials.hpp"

typedef void (*test_func_t)(void);


void test_active_dtor() {
	DORM::DB::execute("INSERT INTO test_result VALUES (true)");

	std::cout << "Test active dtor: START TRANSACTION followed by ROLLBACK" << std::endl;

	DORM::Transaction transaction_guard( DORM::Transaction::ROLLBACK );

	try {
		DORM::DB::execute("UPDATE test_result SET the_result=false");

		DORM::DB::execute("FAIL COMMAND");
	} catch(const sql::SQLException &e) {
		// any other exception is unexpected
		return;
	}

	throw std::runtime_error("SQL command didn't throw");
}


void test_nop_dtor() {
	DORM::DB::execute("INSERT INTO test_result VALUES (false)");

	std::cout << "Test NOP dtor: START TRANSACTION followed by COMMIT" << std::endl;

	DORM::Transaction transaction_guard( DORM::Transaction::ROLLBACK );
	DORM::DB::execute("UPDATE test_result SET the_result=true");
	transaction_guard.commit();		// shouldn't throw if OK
}


void test_rollback() {
	DORM::DB::execute("INSERT INTO test_result VALUES (true)");

	std::cout << "Test rollback: START TRANSACTION followed by ROLLBACK" << std::endl;

	DORM::Transaction transaction_guard( DORM::Transaction::COMMIT );
	DORM::DB::execute("UPDATE test_result SET the_result=false");
	transaction_guard.rollback();		// shouldn't throw if OK
}


void test_move_ctor() {
	DORM::DB::execute("INSERT INTO test_result VALUES (false)");

	std::cout << "Test move ctor: START TRANSACTION followed by COMMIT" << std::endl;

	DORM::Transaction transaction_guard( DORM::Transaction::ROLLBACK );

	DORM::Transaction new_guard = std::move(transaction_guard);
	DORM::DB::execute("UPDATE test_result SET the_result=true");
	new_guard.commit();

	// there should be no ROLLBACK on frame exit
}


void perform_test( test_func_t test_fn ) {
	std::cout << "Preparing for next test..." << std::endl;

	DORM::DB::execute("DELETE FROM test_result");

	test_fn();

	DORM::Query query;
	query.cols.push_back("*");
	query.tables = DORM::Tables("test_result");

	bool result = DORM::DB::fetch_bool(query);

	if ( !result )
		throw std::runtime_error( "Test failed");
}


int main() {
	DORM::DB::connect( DB_URI, DB_USER, DB_PASSWORD, DB_SCHEMA );

	DORM::DB::execute( "CREATE TEMPORARY TABLE test_result ( the_result boolean )" );

	perform_test(test_active_dtor);

	perform_test(test_nop_dtor);

	perform_test(test_rollback);

	perform_test(test_move_ctor);

	exit(0);
}
