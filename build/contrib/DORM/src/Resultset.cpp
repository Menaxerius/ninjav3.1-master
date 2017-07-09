#include "Resultset.hpp"
#include "DB.hpp"


time_t DORM::Resultset::getTimestamp( uint32_t columnIndex ) {
	return DORM::DB::unix_timestamp( getString( columnIndex ) );
}


struct timeval DORM::Resultset::getFTimestamp( uint32_t columnIndex ) {
	return DORM::DB::unix_timeval( getString( columnIndex ) );
}
