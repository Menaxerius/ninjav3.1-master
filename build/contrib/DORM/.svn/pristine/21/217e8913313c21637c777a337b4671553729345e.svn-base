#include "DB.hpp"

#include "Test.hpp"
#include "Test/Frog.hpp"

#include "db_credentials.hpp"


int main() {
	DORM::DB::connect( DB_URI, DB_USER, DB_PASSWORD, DB_SCHEMA );

	Test test;
	test.name("Fudge");
	test.age(5);
	test.save();

	TestFrog frog;
	frog.testID( test.testID() );
	frog.frog_flavour("chocolate");
	frog.save();

	Test tests;
	tests.search();

	while( auto test = tests.result() ) {
		std::cout << "testID: " << test->testID() << ", name: " << test->name() << ", age: " << test->age() << std::endl;
		test->foo();

		auto frogs = test->test_frogs();
		frogs->search();

		while( auto frog = frogs->result() ) {
			std::cout << "testID: " << frog->testID() << ", flavour: " << frog->frog_flavour() << std::endl;
			frog->foo();

			// if we reach here - test is ok
			exit(0);
		}

		throw std::runtime_error("No child TestFrog records found");
	}

	throw std::runtime_error("No parent Test records found");
}
