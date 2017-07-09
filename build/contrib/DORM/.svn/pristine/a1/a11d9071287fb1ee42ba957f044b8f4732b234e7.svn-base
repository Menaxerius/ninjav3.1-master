#ifndef DORM__INCLUDE__RESULTSET__HPP
#define DORM__INCLUDE__RESULTSET__HPP

#include <cppconn/resultset.h>
#include <sys/time.h>


namespace DORM {

	class Resultset: public sql::ResultSet {
		public:
			// CamelCase to follow style of sql::ResultSet
			time_t getTimestamp(uint32_t columnIndex);
			struct timeval getFTimestamp( uint32_t columnIndex );
	};

}

#endif
