#include "DB.hpp"

#include "Test.hpp"

#include "db_credentials.hpp"


int main() {
	DORM::DB::connect( DB_URI, DB_USER, DB_PASSWORD, DB_SCHEMA );

	DORM::DB::execute("insert into Tests values (null, 'Fudge', 5, null, null), (null, 'Dominic', 43, null, null)");

	Test tests;
	tests.age(5);

	bool is_found = tests.present();

	std::cout << "Row with age=5 present: " << (is_found ? "YES" : "NO") << std::endl;

	if (!is_found)
		throw std::runtime_error("Expected row not found");

	exit(0);
}
