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
		sleep(0.2);

		const uint64_t latest_blockID = BlockCache::latest_blockID;

		// we need DB connection from here on
        // Check db connection
        short counter = 0;
        bool is_continue = false;account
        while (true){
            try{
                DORM::DB::check_connection();
                break;
            } catch (const DORM::DB::connection_issue &e) {
                // DB being hammered by miners - try again in a moment
                std::cerr  << ftime() << "[account_updater::account_updater] Too many connections! " << e.getErrorCode() << ": " << e.what() << std::endl;
                is_continue = true;
                break;
            } catch(const sql::SQLException &e) {
                // Could not connect to db.
                std::cerr  << ftime() << "[account_updater::account_updater] " << e.what() << std::endl;
                std::cerr << ftime() << "[account_updater::account_updater] Trying to connect in a moment. Attempt: " << counter + 1 <<  std::endl;
                sleep(1);
            }
            ++counter;
            if(counter + 1 == DB_CONNECTION_ATTEMPT_COUNT){
                std::cerr << ftime() << "[account_updater::account_updater] DB connect failed..." << std::endl;
                throw;
            }
        }
        if(is_continue){
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
			sleep(0.1);
		}
	}
}
