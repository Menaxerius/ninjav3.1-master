#include "WS/updates.hpp"

#include "config.hpp"
#include "BaseHandler.hpp"
#include "BlockCache.hpp"

#include "BurstCoin.hpp"
#include "DORM/DB.hpp"
#include "JSON.hpp"

#include "pthread_np_shim.hpp"


void blockchain_monitor() {
    pthread_set_name_np(pthread_self(), "monitor");

    std::cout << ftime() << "Blockchain monitoring started" << std::endl;

    BurstCoin burst(BURST_SERVERS, BURST_SERVER_TIMEOUT);

	while(!BaseHandler::time_to_die) {

		// we need DB connection from here on
		// Check db connection
		short i = 0;
		bool is_connected = false;
		bool is_continue = false;
		while (!is_connected){
			try{
				DORM::DB::check_connection();
				break;
			} catch (const DORM::DB::connection_issue &e) {
				// DB being hammered by miners - try again in a moment
				std::cerr  << ftime() << "[blockchain_monitor::blockchain_monitor] Too many connections! " << e.getErrorCode() << ": " << e.what() << std::endl;
				is_continue = true;
				sleep(1);
                break;
			} catch(const sql::SQLException &e) {
				// Could not connect to db.
				std::cerr  << ftime() << "[blockchain_monitor::blockchain_monitor] " << e.what() << std::endl;
				std::cerr << ftime() << "[blockchain_monitor::blockchain_monitor] Trying to connect in a moment. Attempt: " << i+1 <<  std::endl;
				sleep(1);
			}
			++i;
			if(i + 1 == DB_CONNECTION_ATTEMPT_COUNT){
				std::cerr << ftime() << "[blockchain_monitor::blockchain_monitor] DB connect failed..." << std::endl;
				throw;
			}
		}
		if(is_continue){
			continue;
		}

		JSON mining_info_JSON;

		try {
			std::string mining_info = burst.get_mining_info();
			mining_info_JSON = JSON(mining_info);
		} catch (const CryptoCoins::server_issue &e) {
			// not good, but not fatal
			sleep(1);
			continue;
		} catch (const JSON::parse_error &e) {
			// not good, but not fatal
			sleep(1);
			continue;
		}

		// decode
		const uint64_t blockID = mining_info_JSON.get_number("height");
		const uint64_t base_target = mining_info_JSON.get_uint64("baseTarget");
		const std::string generation_signature = mining_info_JSON.get_string("generationSignature");

		const bool is_new_latest_block = BlockCache::update_latest_block(blockID, base_target, generation_signature);
		if (is_new_latest_block)
			Handlers::WS::updates::wake_up();

		sleep(1);
	}
}
