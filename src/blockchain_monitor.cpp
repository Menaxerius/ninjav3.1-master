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
		sleep(1);

		try {
			DORM::DB::check_connection();
		} catch (const DORM::DB::connection_issue &e) {
			// DB being hammered by miners - try again in a moment
			continue;
		}

		JSON mining_json;

		try {
			mining_json = burst.get_mining_info();
		} catch (const CryptoCoins::server_issue &e) {
			// not good, but not fatal
			continue;
		}

		// decode
		const uint64_t blockID = mining_json.get_number("height");
		const uint64_t base_target = mining_json.get_uint64("baseTarget");
		const std::string generation_signature = mining_json.get_string("generationSignature");

		const bool is_new_latest_block = BlockCache::update_latest_block(blockID, base_target, generation_signature);

		if (is_new_latest_block)
			Handlers::WS::updates::wake_up();
	}
}
