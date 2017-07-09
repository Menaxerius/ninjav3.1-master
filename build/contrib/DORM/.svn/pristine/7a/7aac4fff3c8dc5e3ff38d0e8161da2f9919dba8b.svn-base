#include "DB.hpp"

#include "Test.hpp"

#include "db_credentials.hpp"


int main() {
	DORM::DB::connect( DB_URI, DB_USER, DB_PASSWORD, DB_SCHEMA );

	Test t;
	t.name("Fudge");
	t.age(5);
	t.save();

	Test tests;
	tests.age(5);

	auto test = tests.load();
	std::cout << "testID: " << test->testID() << ", name: " << test->name() << ", age: " << test->age() << std::endl;
	test->foo();

	test = Test::load(1);	// should be testID for first row given fresh, blank database
	std::cout << "testID: " << test->testID() << ", name: " << test->name() << ", age: " << test->age() << std::endl;
	test->foo();

	exit(0);
}
