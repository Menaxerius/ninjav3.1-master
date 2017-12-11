#ifndef DORM__INCLUDE__TIMESTAMP_HPP
#define DORM__INCLUDE__TIMESTAMP_HPP

#include <cppconn/sqlstring.h>

#include <string>
#include <ctime>
#include <sys/time.h>


namespace DORM {

	class Timestamp {
		public:
			struct timeval tv;

			Timestamp(): tv{0, 0} {};
			Timestamp(const time_t &t): tv{t, 0} {};
			Timestamp(const struct timeval &t): tv{t} {};

			Timestamp(const std::string time_str);
			Timestamp(sql::SQLString time_str);

			operator time_t() { return tv.tv_sec; };
			operator timeval() { return tv; };
	};

}

#endif
