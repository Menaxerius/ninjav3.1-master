#include "Account.hpp"

#include "BlockCache.hpp"
#include "BaseHandler.hpp"
#include "config.hpp"

#include "BurstCoin.hpp"
#include "ftime.hpp"

#include "pthread_np_shim.hpp"


void account_updater() {
    pthread_set_name_np(pthread_self(), "account_updater");

    std::cout << ftime() << "Miner account refreshing started" << std::endl;

    BurstCoin burst(BURST_SERVERS);

	while(!BaseHandler::time_to_die) {
		sleep(2);

		const uint64_t latest_blockID = BlockCache::latest_blockID;

		// we need DB connection from here on
		try {
			DORM::DB::check_connection();
		} catch (const DORM::DB::connection_issue &e) {
			// DB being hammered by miners - try again in a moment
			continue;
		}

		Account accounts;
		accounts.needs_updating(true);
		accounts.search();

		while( auto account = accounts.result() ) {
			if (BaseHandler::time_to_die)
				return;		// quicker process exit

			account->update_check(burst, latest_blockID);
			account->save();
			sleep(1);
		}
	}
}
