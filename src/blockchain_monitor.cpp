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
		try {
			DORM::DB::check_connection();
		} catch (const DORM::DB::connection_issue &e) {
			// DB being hammered by miners - try again in a moment
			sleep(1);
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
