#include "Block.hpp"

#include "Nonce.hpp"
#include "Share.hpp"
#include "Reward.hpp"

#include "mining_info.hpp"
#include "config.hpp"

#include "DORM/sql/sqlLt.hpp"
#include "DORM/sql/sqlGt.hpp"
#include "DORM/sql/sqlIsNotNull.hpp"

#include "BurstCoin.hpp"
#include "ftime.hpp"

extern "C" {
	#include "shabal.h"
};

#include <cmath>


void Block::search_prep( DORM::Query &query ) const {
	if (after_blockID)
		query.and_where( DORM::sqlGt<uint64_t>( "blockID", after_blockID() ) );

	if (before_blockID)
		query.and_where( DORM::sqlLt<uint64_t>( "blockID", before_blockID() ) );

	if (has_reward_value)
		query.and_where( DORM::sqlIsNotNull("block_reward") );

	if (newest_first)
		query.order_by = "blockID DESC";
}


void Block::reward_miners() {
	BurstCoin burst("No server, just needed for pretty_amount()");

	uint64_t reward_to_share = this->block_reward() * BURST_TO_NQT;

	// remove fees, etc. from potential reward to be shared
	uint64_t pool_fee = reward_to_share * POOL_FEE_FRACTION;
	reward_to_share -= pool_fee - PAYMENT_SEND_FEE;

	std::map<uint64_t, uint64_t > reward_amounts_by_accountID;

	// shares for finding current block
	Share current_shares;
	current_shares.blockID( this->blockID() );
	current_shares.search();

	uint64_t sum_finding_rewards = 0;

	while( auto share = current_shares.result() ) {
		uint64_t amount = reward_to_share * share->share_fraction() * CURRENT_BLOCK_REWARD_PERCENT / 100;

		reward_amounts_by_accountID[ share->accountID() ] = amount;
		sum_finding_rewards += amount;
	}

	// shares for historic contribution to block finding
	auto historic_shares = Share::historic_shares( blockID() - 1, HISTORIC_BLOCK_COUNT );
	historic_shares->search();

	uint64_t sum_historic_rewards = 0;

	while( auto share = historic_shares->result() ) {
		uint64_t amount = reward_to_share * share->share_fraction() * (100 - CURRENT_BLOCK_REWARD_PERCENT) / 100;

		reward_amounts_by_accountID.insert( std::make_pair( share->accountID(), 0 ) );

		reward_amounts_by_accountID[ share->accountID() ] += amount;
		sum_historic_rewards += amount;
	}

	// miners pay transaction fees
	uint64_t transaction_fees = reward_amounts_by_accountID.size() * PAYMENT_SEND_FEE;
	reward_to_share -= transaction_fees;

	uint64_t sum_rewarded = 0;
	for(const auto &it : reward_amounts_by_accountID) {
		const uint64_t reward_amount = it.second;

		Reward reward;
		reward.blockID( blockID() );
		reward.accountID( it.first );
		reward.amount( reward_amount );
		reward.save();

		sum_rewarded += reward.amount();
	}

	this->has_been_shared(true);
	this->save();

	std::cout << ftime() << "Block " << blockID() << " reward: " << block_reward() << " BURST, pool fee: " << burst.pretty_amount(pool_fee) <<
				", rewarded: " << burst.pretty_amount(sum_rewarded) << ", tx fees: " << burst.pretty_amount(transaction_fees) << std::endl;
}


/*
std::unique_ptr<Block> Block::latest_block() {
	Block blocks;
	blocks.newest_first(true);
	return blocks.load();
}


uint64_t Block::latest_blockID() {
	// this code is slightly more raw/closer to SQL
	// as we're just after the blockID
	// so no need to load the whole block record
	// plus blockID should come direct from index
	DORM::Query query;
	query.tables = DORM::Tables( static_table_name() );
	query.cols = { "blockID" };

	query.order_by = "blockID DESC";
	query.limit = 1;

	return DORM::DB::fetch_uint64(query);
}


std::unique_ptr<Block> Block::latest_won_block() {
	Block blocks;
	blocks.is_our_block(true);
	blocks.newest_first(true);
	return blocks.load();
}
*/


/*
std::unique_ptr<Nonce> Block::find_best_nonce( const uint64_t blockID ) {
	Nonce nonces;
	nonces.blockID( blockID );
	nonces.is_blocks_best_deadline(true);
	return nonces.load();
}
*/


/*
void Block::recalculate_shares( const uint64_t blockID ) {
	// now assumes it is already within a block-serialized transaction

	Nonce nonces;
	nonces.blockID(blockID);
	nonces.is_accounts_best_deadline(true);
	nonces.search();

	std::vector< std::unique_ptr<Nonce> > nonces_to_share;
	double total_shares = 0.0;
	std::vector<double> shares;

	while( auto nonce = nonces.result() ) {
		// it's actually possible (although rare) for deadline to be zero!
		const uint64_t deadline = nonce->deadline() + 1;

		double share = pow( static_cast<double>(deadline), SHARE_POWER_FACTOR );

		share = 1.0 / share;
		shares.push_back( share );

		total_shares += share;

		nonces_to_share.push_back( std::move(nonce) );
	}

	// no need to wipe old shares
	// as existing shares only get insert/updated, never deleted
	for(int i=0; i<nonces_to_share.size(); i++) {
		const auto &nonce = nonces_to_share[i];
		const double share_fraction = shares[i] / total_shares;

		Share share;
		share.blockID(blockID);
		share.accountID( nonce->accountID() );
		share.share_fraction( share_fraction );
		share.deadline( nonce->deadline() );
		share.deadline_string( nonce->deadline_string() );
		share.miner( nonce->miner() );
		share.save();
	}
}
*/


uint64_t Block::previous_reward_post_fee() {
	Block blocks;
	blocks.newest_first(true);
	blocks.has_reward_value(true);

	auto prev_block = blocks.load();

	if (!prev_block)
		return 0;

	// works even if prev_block_reward is 0
	return prev_block->block_reward() * (1 - POOL_FEE_FRACTION);
}


void Block::unpack_generation_signature( const std::string &gen_sig_str, gen_sig_array_t &gen_sig_array ) {
	for(int i=0; i<32; i++)
		gen_sig_array[i] = std::stoul( gen_sig_str.substr( i<<1, 2).c_str(), nullptr, 16 );
}


uint32_t Block::calculate_scoop( const uint64_t scoop_blockID, const std::string &scoop_generation_signature ) {
	// shabal256 hash of gen_sig (in binary form) and then block number (uint64_t big-endian)
	shabal_context sc;
	gen_sig_array_t binary_generation_signature;
	unpack_generation_signature( scoop_generation_signature, binary_generation_signature );

	shabal_init(&sc, 256);
	shabal(&sc, binary_generation_signature, 32);

	const uint64_t block_swapped = htobe64( scoop_blockID );
	shabal(&sc, &block_swapped, sizeof(block_swapped));

	uint8_t new_generation_signature[32];
	shabal_close(&sc, 0, 0, new_generation_signature);

	// finally we get to determine scoop number
	return ((new_generation_signature[30] & 0x0F) << 8) | new_generation_signature[31];
}
