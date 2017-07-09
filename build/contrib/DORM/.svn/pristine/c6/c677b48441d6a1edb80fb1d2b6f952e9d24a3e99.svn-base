#include "Lock.hpp"
#include "DB.hpp"
#include "Resultset.hpp"

#include <cppconn/prepared_statement.h>


namespace DORM {

	Lock::Lock( std::string lock_name, int timeout ) {
		name = lock_name;

		const std::string sql = "SELECT GET_LOCK(?, ?)";

		std::unique_ptr<sql::PreparedStatement> pstmt = DB::prepare(sql);

		pstmt->setString(1, name);
		pstmt->setInt(2, timeout);

		std::unique_ptr<Resultset> result( static_cast<Resultset *>( pstmt->executeQuery() ) );

		if ( !result || !result->next() || !result->getInt(1) ) {
			// if result is NULL then something bad happened
			if ( result->isNull(1) )
				throw sql::SQLException("GET_LOCK() failed");

			std::string message( "Couldn't obtain database lock" );

			if (timeout != -1)
				message += " within timeout";

			throw Lock::Exception(message);
		}

		is_active = true;
	}


	Lock::~Lock() {
		if (!is_active)
			return;

		release();
	}


	void Lock::release() {
		const std::string sql = "DO RELEASE_LOCK(?)";

		std::unique_ptr<sql::PreparedStatement> pstmt = DB::prepare(sql);

		pstmt->setString(1, name);

		std::unique_ptr<Resultset> result( static_cast<Resultset *>( pstmt->executeQuery() ) );

		is_active = false;
	}

}
