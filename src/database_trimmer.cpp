#include "Block.hpp"
#include "Nonce.hpp"
#include "Share.hpp"

#include "BlockCache.hpp"
#include "BaseHandler.hpp"
#include "config.hpp"

#include "BurstCoin.hpp"
#include "ftime.hpp"

#include "pthread_np_shim.hpp"

#include <algorithm>


void database_trimmer() {
    pthread_set_name_np(pthread_self(), "database_trimmer");

    std::cout << ftime() << "Database trimming thread started" << std::endl;

    uint64_t current_blockID = 0;

	while(!BaseHandler::time_to_die) {
		sleep(2);

		uint64_t latest_blockID = BlockCache::latest_blockID;

		if (latest_blockID == current_blockID)
			continue;	// no need to trim more than once per block

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
				std::cerr  << ftime() << "[database_trimmer::database_trimmer] Too many connections! " << e.getErrorCode() << ": " << e.what() << std::endl;
				is_continue = true;
				break;
			} catch(const sql::SQLException &e) {
				// Could not connect to db.
				std::cerr  << ftime() << "[database_trimmer::database_trimmer] " << e.what() << std::endl;
				std::cerr << ftime() << "[database_trimmer::database_trimmer] Trying to connect in a moment. Attempt: " << counter + 1 <<  std::endl;
				sleep(1);
			}
			++counter;
			if(counter + 1 == DB_CONNECTION_ATTEMPT_COUNT){
				std::cerr << ftime() << "[database_trimmer::database_trimmer] DB connect failed..." << std::endl;
				throw;
			}
		}
		if(is_continue){
			continue;
		}

		// wait a short while for new block fever to calm down
		// (also check there's not been another new block or that server termination has been requested)
		for(int i=0; i<20 && !BaseHandler::time_to_die && latest_blockID == BlockCache::latest_blockID; ++i)
			sleep(4);

		if (BaseHandler::time_to_die)
			return;

		// another new block already?
		if (latest_blockID != BlockCache::latest_blockID)
			continue;

		// how far to go back?
		const uint64_t block_count = std::max( HISTORIC_BLOCK_COUNT, HISTORIC_CAPACITY_BLOCK_COUNT );

		// get rid of old blocks
		Block blocks;
		blocks.before_blockID(latest_blockID - block_count - 1);
		blocks.search_and_destroy();

		// get rid of old nonces
		Nonce nonces;
		nonces.before_blockID(latest_blockID - block_count - 1);
		nonces.search_and_destroy();

		// get rid of old shares
		Share shares;
		shares.before_blockID(latest_blockID - block_count - 1);
		shares.search_and_destroy();

		// we could probably do a few "optimize table" calls here
		DORM::DB::execute("optimize table Accounts");
		DORM::DB::execute("optimize table Blocks");
		DORM::DB::execute("optimize table Bonuses");
		DORM::DB::execute("optimize table Nonces");
		DORM::DB::execute("optimize table Rewards");
		DORM::DB::execute("optimize table Shares");

		current_blockID = latest_blockID;
	}
}
