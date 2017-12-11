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

	uint64_t reward_to_share = this->block_reward() * BURST_TO_NQT + this->tx_fees();

	// remove fees, etc. from potential reward to be shared
	const uint64_t pool_fee = reward_to_share * POOL_FEE_FRACTION;
	reward_to_share -= pool_fee;

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

	std::cout << ftime() << "Block " << blockID() << " reward: " << block_reward() << " BURST + tx fees: " << burst.pretty_amount( tx_fees() )
			<< ", pool fee: " << burst.pretty_amount(pool_fee)
			<< ", rewarded: " << burst.pretty_amount(sum_finding_rewards) << " (current) + " << burst.pretty_amount(sum_historic_rewards) << " (historic) = "
			<< burst.pretty_amount(sum_rewarded) << std::endl;
}


uint64_t Block::calc_block_reward( const uint64_t block_height ) {
	// code converted from BlockImpl.java
	if (block_height == 0 || block_height >= 1944000)
		return 0;

	const int month = block_height / 10800;		// 10800 * 4mins = 30days (~1 month)

	const uint64_t reward = std::floor( 10000.0 * std::pow( 0.95, month ) );

	return reward * BURST_TO_NQT;
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
