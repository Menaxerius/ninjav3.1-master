#include "DB.hpp"

#include "Test.hpp"
#include "Test/Single.hpp"

#include "db_credentials.hpp"


int main() {
	DORM::DB::connect( DB_URI, DB_USER, DB_PASSWORD, DB_SCHEMA );

	Test test;
	test.name("Fudge");
	test.age(5);
	test.save();

	TestSingle single;
	single.testID( test.testID() );
	single.blah("BLAH!");
	single.save();

	Test tests;
	tests.search();

	while( auto test = tests.result() ) {
		std::cout << "testID: " << test->testID() << ", name: " << test->name() << ", age: " << test->age() << std::endl;
		test->foo();

		auto single = test->test_single();
		if (single) {
			std::cout << "testID: " << single->testID() << ", blah: " << single->blah() << ", something: " << single->something() << std::endl;
			single->foo();

			exit(0);
		}

		throw std::runtime_error("No child TestSingle records found");
	}

	throw std::runtime_error("No parent Test records found");
}
