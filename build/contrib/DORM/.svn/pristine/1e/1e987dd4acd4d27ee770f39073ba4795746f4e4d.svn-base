#ifndef TESTOBJECT_HPP
#define TESTOBJECT_HPP

/*
	create table Tests (
		testID				serial,
		name				varchar(255) not null,
		age					int unsigned not null,
		simple_time			timestamp,
		complex_time		timestamp(6),
		primary key			(testID),
		index				(name, age),
		BAD-LINE
	);
*/

#include "Test_.hxx"

#include "sql/sqlLt.hpp"

CHILD_OBJECTS(TestFrog, test_frogs);
CHILD_OBJECT(TestSingle, test_single);


class Test: public Test_ {
	public:
		DORM::SearchMod<unsigned int> younger_than;
	
		void foo() { std::cout << "TEST FOO: " << name() << std::endl; }
		
		void search_prep( DORM::Query &query ) const {
			if (younger_than)
				query.and_where( DORM::sqlLt<unsigned int>( "age", younger_than() ) );
		};
};


#endif
