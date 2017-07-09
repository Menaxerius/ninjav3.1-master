#ifndef TESTSINGLEOBJECT_HPP
#define TESTSINGLEOBJECT_HPP

/*
	create table TestSingles (
		testID				bigint unsigned not null,
		blah				varchar(255) not null,
		something			int not null default 123,
		primary key			(testID)
	);
*/

#include "Test/Single_.hxx"



class TestSingle: public TestSingle_ {
	public:
		void foo() { std::cout << "TEST-SINGLE FOO: " << blah() << std::endl; }
};


#endif
