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

	try {
		std::string block_info = burst.get_block(blockID);

		JSON block_json(block_info);

		if ( !block_json.null("generator") ) {
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
		}
	} catch (const CryptoCoins::server_issue &e) {
		// failed - try again soon?
		return false;
	} catch (const JSON::parse_error &e) {
		// failed - try again soon?
		return false;
	}

	return true;
}


void blockchain_refresh() {
    pthread_set_name_np(pthread_self(), "refresh");

    std::cout << ftime() << "Blockchain refreshing started" << std::endl;

    BurstCoin burst(BURST_SERVERS);

	while(!BaseHandler::time_to_die) {
		sleep(1);

        // we need DB connection from here on
        // Check db connection
        short counter = 0;
        bool is_continue = false;
        while (true){
            try{
                DORM::DB::check_connection();
                break;
            } catch (const DORM::DB::connection_issue &e) {
                // DB being hammered by miners - try again in a moment
                std::cerr  << ftime() << "[blockchain_refresh::blockchain_refresh] Too many connections! " << e.getErrorCode() << ": " << e.what() << std::endl;
                is_continue = true;
                break;
            } catch(const sql::SQLException &e) {
                // Could not connect to db.
                std::cerr  << ftime() << "[blockchain_refresh::blockchain_refresh] " << e.what() << std::endl;
                std::cerr << ftime() << "[blockchain_refresh::blockchain_refresh] Trying to connect in a moment. Attempt: " << i+1 <<  std::endl;
                sleep(1);
            }
            ++counter;
            if(counter + 1 == DB_CONNECTION_ATTEMPT_COUNT){
                std::cerr << ftime() << "[blockchain_refresh::blockchain_refresh] DB connect failed..." << std::endl;
                throw;
            }
        }
        if(is_continue){
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
