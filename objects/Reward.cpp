#include "Reward.hpp"

#include "DORM/sql/sqlLt.hpp"


void Reward::search_prep( DORM::Query &query ) const {
	if (before_blockID)
		query.and_where( DORM::sqlLt<uint64_t>( "blockID", before_blockID() ) );

	if (below_amount)
		query.and_where( DORM::sqlLt<uint64_t>( "amount", below_amount() ) );

	if (paid_before_block_id)
		query.and_where( DORM::sqlLt<uint64_t>( "paid_at_block_id", paid_before_block_id() ) );

	if (oldest_first)
		query.order_by = "blockID ASC";

	if (one_result_per_account)
		query.group_by = "accountID";
}


uint64_t Reward::sum_amount() {
	DORM::Query query;
	query.tables = DORM::Tables( static_table_name() );
	query.cols = { "SUM(amount)" };

	search_prep_columns(query);
	search_prep(query);

	return DORM::DB::fetch_uint64(query);
}


uint64_t Reward::total_unpaid() {
	Reward rewards;
	rewards.is_paid(false);
	return rewards.sum_amount();
}


uint64_t Reward::total_paid_by_accountID( uint64_t accountID ) {
	Reward rewards;
	rewards.is_paid(true);
	rewards.is_confirmed(true);
	rewards.accountID(accountID);
	return rewards.sum_amount();
}


uint64_t Reward::total_unconfirmed_by_accountID( uint64_t accountID ) {
	Reward rewards;
	rewards.is_paid(true);
	rewards.is_confirmed(false);
	rewards.accountID(accountID);
	return rewards.sum_amount();
}
