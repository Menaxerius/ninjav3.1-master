#include "DB.hpp"
#include "Timestamp.hpp"

#include <cmath>
#include <time.h>

int main() {
	DORM::Timestamp ts;
		if ( ts.tv.tv_sec != 0 || ts.tv.tv_usec != 0 )
		throw std::runtime_error("Default constructor didn't zero timeval");
	
	ts = DORM::Timestamp( std::string("0000-00-00 00:00:00") );
	if ( ts.tv.tv_sec != 0 || ts.tv.tv_usec != 0 )
		throw std::runtime_error("Zeroed SQL timestamp didn't produce zeroed timeval");

	ts = DORM::Timestamp( sql::SQLString("2004-01-10 13:37:04") );	// 2**30
	if ( ts.tv.tv_sec != std::pow(2, 30) )
		throw std::runtime_error("SQLString constructor didn't produce correct value");

	ts = DORM::Timestamp( std::string("xxx") );
	if ( ts.tv.tv_sec != -1 )
		throw std::runtime_error("Invalid string constructor didn't produce -1 value");

	ts = DORM::Timestamp( std::string("1234-56-78 99:99:99") );
	if ( ts.tv.tv_sec != -1 )
		throw std::runtime_error("Invalid string constructor didn't produce -1 value");

	time_t now = time(nullptr);

	ts = DORM::Timestamp(now);
	if ( ts.tv.tv_sec != now || ts.tv.tv_usec != 0 )
		throw std::runtime_error("time_t constructor didn't produce correct timeval");

	struct timeval tv;
	gettimeofday(&tv, nullptr);

	ts = DORM::Timestamp(tv);
	if ( ts.tv.tv_sec != tv.tv_sec || ts.tv.tv_usec != tv.tv_usec )
		throw std::runtime_error("timeval constructor didn't produce correct timeval");

	struct timeval out_tv = ts;
	if ( out_tv.tv_sec != tv.tv_sec || out_tv.tv_usec != tv.tv_usec )
		throw std::runtime_error("timeval operator didn't return correct timeval");

	time_t tv_sec = ts;
	if ( tv_sec != tv.tv_sec  )
		throw std::runtime_error("time_t operator didn't return correct value");

	exit(0);
}
