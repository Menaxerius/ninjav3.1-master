#include "Account.hpp"
#include "Block.hpp"
#include "Nonce.hpp"

#include "BlockCache.hpp"
#include "config.hpp"
#include "BaseHandler.hpp"

#include "BurstCoin.hpp"
#include "DORM/DB.hpp"
#include "JSON.hpp"
#include "ftime.hpp"

#include "pthread_np_shim.hpp"


uint64_t current_blockID = 0;
uint64_t gap_check_blockID = 0;
uint64_t recent_blockID = 0;
bool finished_checking_for_gaps = false;


bool refresh_previous_block( const uint64_t blockID ) {
	auto block = Block::load( blockID );

	if (block == nullptr) {
		block = std::make_unique<Block>();
		block->blockID( blockID );
	}

	BurstCoin burst(BURST_SERVERS);

	JSON block_json;

	try {
		block_json = burst.get_block(blockID);
	} catch (const CryptoCoins::server_issue &e) {
		// failed - try again soon?
		return false;
	}

	// blocks really should have a generator!
	if ( block_json.null("generator") )
		return false;

	// update info about who forged the block
	const uint64_t generator = block_json.get_uint64("generator");
	const uint64_t nonce = block_json.get_uint64("nonce");
	const unsigned int scoop = block_json.get_number("scoopNum");
	const uint64_t base_target = block_json.get_uint64("baseTarget");
	const std::string generation_signature = block_json.get_string("generationSignature");


	block->generator_account_id( generator );
	block->nonce( nonce );
	block->base_target( base_target );
	block->generation_signature( generation_signature );
	block->scoop( scoop );
	block->block_reward( block_json.get_uint64("blockReward") );
	block->block_id( block_json.get_uint64("block") );
	block->tx_fees( block_json.get_uint64("totalFeeNQT") );

	uint64_t timestamp = block_json.get_uint64("timestamp");
	block->forged_when( burst.crypto_to_unix_time(timestamp) );

	uint64_t deadline = Nonce::calculate_deadline( generator, nonce, blockID, scoop, base_target, generation_signature );
	block->deadline( deadline );

	// update check with our best nonce to see if we won block
	Nonce nonces;
	nonces.blockID( blockID );
	nonces.best_first(true);
	auto best_nonce = nonces.load();
	if (best_nonce) {
		block->best_nonce_account_id( best_nonce->accountID() );
		block->our_best_deadline( best_nonce->deadline() );

		block->is_our_block( best_nonce->accountID() == generator );
	}

	block->save();

	// also ensure there's at least a skeleton Account record
	// so that the account_updater thread can check for account name for prettiness
	Account account;
	account.accountID( generator );
	account.save();

	return true;
}


void blockchain_refresh() {
    pthread_set_name_np(pthread_self(), "refresh");

    std::cout << ftime() << "Blockchain refreshing started" << std::endl;

    BurstCoin burst(BURST_SERVERS);

	while(!BaseHandler::time_to_die) {
		sleep(3);

		try {
			DORM::DB::check_connection();
		} catch (const DORM::DB::connection_issue &e) {
			// DB being hammered by miners - try again in a moment
			continue;
		}

		// when a new block happens, refresh some recent blocks
		// so we can find out if the network agrees about which blocks we won
		uint64_t latest_blockID = BlockCache::latest_blockID;

		const uint64_t final_recent_blockID = latest_blockID - MIN_PAYOUT_BLOCK_DELAY - 1;

		// latest block changed? start again
		if (latest_blockID != current_blockID) {
			recent_blockID = latest_blockID - 1;
			current_blockID = latest_blockID;

			std::cout << ftime() << "Refreshing blocks " << final_recent_blockID << " to " << recent_blockID << " due to new block " << latest_blockID << std::endl;
		}

		if (recent_blockID >= final_recent_blockID) {
			bool refreshed = refresh_previous_block(recent_blockID);

			if (!refreshed)
				continue;

			--recent_blockID;
		}

		if (finished_checking_for_gaps)
			continue;

		// first-time initialization
		if (gap_check_blockID == 0)
			gap_check_blockID = latest_blockID - 1;

		// no more work?
		if (gap_check_blockID < latest_blockID - HISTORIC_BLOCK_COUNT) {
			finished_checking_for_gaps = true;
			std::cout << ftime() << "Finished checking for missing blocks at block " << gap_check_blockID << std::endl;

			continue;
		}

		// checking for gaps...
		Block blocks;
		blocks.blockID( gap_check_blockID );

		auto block = blocks.load();

		if ( !block || !block->generator_account_id() ) {
			bool refreshed = refresh_previous_block(gap_check_blockID);

			if (!refreshed)
				continue;
		}

		--gap_check_blockID;
	}
}
