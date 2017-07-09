#include "Bonus.hpp"

#include "DORM/sql/sqlLt.hpp"


void Bonus::search_prep( DORM::Query &query ) const {
	if (newest_first)
		query.order_by = "bonusID DESC";
}
