#include "BlockCache.hpp"

#include "Nonce.hpp"

#include "AccountCache.hpp"
#include "SubmissionCache.hpp"
#include "config.hpp"

#include "BurstCoin.hpp"
#include "ftime.hpp"
#include "JSON.hpp"


std::unique_ptr<Block>	BlockCache::latest_block;
std::unique_ptr<Block>	BlockCache::previous_block;
std::mutex				BlockCache::block_mutex;
std::string				BlockCache::mining_info;
std::string				BlockCache::latest_generation_signature;

std::atomic<uint64_t>	BlockCache::latest_blockID( 0 );
std::atomic<uint64_t>	BlockCache::pool_balance( 0 );
std::atomic<uint64_t>	BlockCache::deferred_total( 0 );


std::string BlockCache::get_mining_info() {
	std::lock_guard<std::mutex>	block_guard(block_mutex);

	// we really need to return a copy to be thread-safe
	// this -should- use the copy-constructor, avoiding copy-elision
	return std::string(mining_info);
}


bool BlockCache::update_latest_block(const uint64_t blockID, const uint64_t base_target, const std::string &generation_signature) {
	// quick exit?
	if (blockID < latest_blockID || generation_signature == latest_generation_signature)
		return false;

	std::lock_guard<std::mutex>	block_guard(block_mutex);

	if ( blockID > latest_blockID )
		std::swap(previous_block, latest_block);

	bool latest_block_already_existed = true;
	latest_block = Block::load(blockID);
	if (!latest_block) {
		latest_block = std::make_unique<Block>();
		latest_block->blockID(blockID);
		latest_block->first_seen_when( time(nullptr) );

		latest_block_already_existed = false;
	}

	latest_block->base_target(base_target);
	latest_block->generation_signature(generation_signature);
	latest_block->scoop( Block::calculate_scoop(blockID, generation_signature) );

	// number of potential miners and batch-update of accounts' reward recipient
	bool bulk_updated_reward_recipients = false;
	try {
		BurstCoin burst(BURST_SERVERS, BURST_SERVER_TIMEOUT);

		JSON accounts_json = burst.get_accounts_with_reward_recipient(OUR_ACCOUNT_RS);

		if ( !accounts_json.null("accounts") ) {
			const JSON_Array accounts_json_array = accounts_json.get_array("accounts");
			latest_block->num_potential_miners( accounts_json_array.size() );

			// update a whole load of accounts' reward recipient for free
			// to save a ton of calls when each account submits their first nonce for this block
			AccountCache::update_reward_recipients(blockID, accounts_json_array);
			bulk_updated_reward_recipients = true;
		}
	} catch (const CryptoCoins::server_issue &e) {
		// not good, but not fatal
	}

	latest_block->save();

	if ( blockID > latest_blockID ) {
		std::cout << ftime() << "New block! Block: " << blockID << ", scoop: " << latest_block->scoop() <<
				(bulk_updated_reward_recipients ? ", with bulk reward recipient update" : ", no reward recipients") << std::endl;
	} else {
		std::cout << ftime() << "Block " << blockID << " generation signature changed! Forked blockchain recoalescing? New scoop: " << latest_block->scoop() <<
				(bulk_updated_reward_recipients ? ", with bulk reward recipient update" : ", no reward recipients") << std::endl;
		// start again
		latest_block_already_existed = false;
	}

	// generate new mining_info string
	JSON mining_info_JSON;
	mining_info_JSON.add_string( "generationSignature", generation_signature );
	mining_info_JSON.add_string( "baseTarget", std::to_string(base_target) );
	mining_info_JSON.add_int( "requestProcessingTime", 0 );
	mining_info_JSON.add_string( "height", std::to_string(blockID) );
	mining_info_JSON.add_string( "targetDeadline", std::to_string(DEADLINE_MAX) );
	mining_info = mining_info_JSON.to_string();

	latest_blockID = blockID;
	latest_generation_signature = generation_signature;

	SubmissionCache::new_block_reset(latest_block_already_existed);

	return true;
}


void BlockCache::inc_rejected_nonces() {
	std::lock_guard<std::mutex>	block_guard(block_mutex);

	if (!latest_block)
		return;

	latest_block->num_rejected_nonces( latest_block->num_rejected_nonces() + 1);
	latest_block->save();
}


void BlockCache::update_best_nonce( const std::unique_ptr<Nonce> &nonce ) {
	std::lock_guard<std::mutex>	block_guard(block_mutex);

	if (!latest_block)
		return;

	// is block's current deadline still the best?
	if ( latest_block->defined_our_best_deadline() && latest_block->our_best_deadline() < nonce->deadline() )
		return;

	// supplied nonce has best deadline
	latest_block->our_best_deadline( nonce->deadline() );
	latest_block->nonce( nonce->nonce() );
	latest_block->best_nonce_account_id( nonce->accountID() );
	latest_block->save();
}


std::unique_ptr<Block> BlockCache::clone_latest_block() {
	std::lock_guard<std::mutex>	block_guard(block_mutex);

	if (!latest_block)
		return std::unique_ptr<Block>();

	return latest_block->clone();
}



std::unique_ptr<Block> BlockCache::clone_previous_block() {
	std::lock_guard<std::mutex>	block_guard(block_mutex);

	if (!previous_block)
		return std::unique_ptr<Block>();

	return previous_block->clone();
}


uint64_t BlockCache::rejected_nonce_count() {
	std::lock_guard<std::mutex>	block_guard(block_mutex);

	if (!latest_block)
		return 0;

	return latest_block->num_rejected_nonces();
}
