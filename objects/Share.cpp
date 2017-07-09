#include "Share.hpp"

#include "DORM/sql/sqlLt.hpp"
#include "DORM/sql/sqlGt.hpp"
#include "DORM/TableOrSubquery.hpp"


void Share::search_prep( DORM::Query &query ) const {
	if (before_blockID)
		query.and_where( DORM::sqlLt<uint64_t>( "Shares.blockID", before_blockID() ) );		// Table.column because of weighted_deadline ambiguity below

	if (after_blockID)
		query.and_where( DORM::sqlGt<uint64_t>( "Shares.blockID", after_blockID() ) );		// Table.column because of weighted_deadline ambiguity below

	if (historic_average) {
		query.cols = { "blockID", "accountID", "SUM(share_fraction) / n_blocks AS share_fraction", "deadline", "deadline_string", "miner" };

		DORM::Query subquery;
		subquery.tables = DORM::Tables( static_table_name() );
		subquery.cols = { "COUNT(DISTINCT blockID) AS n_blocks" };

		subquery.and_where( DORM::sqlLt<uint64_t>( "blockID", before_blockID() ) );
		subquery.and_where( DORM::sqlGt<uint64_t>( "blockID", after_blockID() ) );

		query.tables.join( "JOIN", DORM::Subquery(subquery, "Shares_with_blocks"), nullptr );

		query.order_by = "share_fraction DESC";
		query.group_by = "accountID";
	}

	if (weighted_deadline) {
		// having to use Table.column isn't great, nor is having to list the column names either
		query.cols = { "Shares.blockID", "accountID", "share_fraction", "Shares.deadline * base_target as weighted_deadline", "deadline_string", "miner" };

		// JOIN Blocks ON (blockID)
		// "NATURAL JOIN" won't work due to shared "deadline" column
		query.tables.join( "JOIN", "Blocks", DORM::sqlEq<DORM::ColName>("Shares.blockID", "Blocks.blockID") );

		query.order_by = "weighted_deadline ASC";
	}

	if (biggest_first)
		query.order_by = "share_fraction DESC";
}


std::unique_ptr<Share> Share::historic_shares( uint64_t to_blockID, uint64_t block_count ) {
	auto shares = std::make_unique<Share>();
	shares->after_blockID( to_blockID - block_count );
	// include this block ("before" is "less than" so we need to add 1)
	shares->before_blockID( to_blockID + 1 );
	shares->historic_average(true);
	return shares;
}
