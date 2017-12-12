#include "Fee.hpp"

#include "DORM/sql/sqlLt.hpp"


void Fee::search_prep( DORM::Query &query ) const {
	if (paid_before_block_id)
		query.and_where( DORM::sqlLt<uint64_t>( "paid_at_block_id", paid_before_block_id() ) );

	if (oldest_first)
		query.order_by = "blockID ASC";
}
