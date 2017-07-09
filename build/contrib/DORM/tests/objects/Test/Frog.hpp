#ifndef TESTFROGOBJECT_HPP
#define TESTFROGOBJECT_HPP

/*
	create table TestFrogs (
		testID				bigint unsigned not null,
		frog_flavour		varchar(255) not null,
		primary key			(testID)
	);
*/

#include "Test/Frog_.hxx"

#include "sql/sqlLike.hpp"


class TestFrog: public TestFrog_ {
	public:
		DORM::SearchMod<std::string> like;
	
		void foo() { std::cout << "TEST-FROG FOO: " << frog_flavour() << std::endl; }
		
		void search_prep( DORM::Query &query ) const {
				if (like)
					query.and_where( DORM::sqlLike( "frog_flavour", "%" + like() + "%" ) );
		};
};


#endif
