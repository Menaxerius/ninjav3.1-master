#include "Timestamp.hpp"
#include "DB.hpp"


namespace DORM {

	Timestamp::Timestamp(const std::string time_str) {
		tv = DB::unix_timeval(time_str);
	}


	Timestamp::Timestamp(sql::SQLString time_str) {
		tv = DB::unix_timeval(time_str);	// should auto-convert via std::string
	}

}
