#include "Block.hpp"
#include "Nonce.hpp"

#include "BlockCache.hpp"
#include "SubmissionCache.hpp"
#include "BaseHandler.hpp"
#include "config.hpp"

#include "BurstCoin.hpp"
#include "ftime.hpp"

#include "pthread_np_shim.hpp"


// how many seconds in advance do we submitting our winning nonce to the wallet?
const int FORGE_ADVANCE = 10;

time_t previous_forge_time = 0;


void block_forger() {
    pthread_set_name_np(pthread_self(), "block_forger");

    std::cout << ftime() << "Block forger started" << std::endl;

    BurstCoin burst(BURST_SERVERS, BURST_SERVER_TIMEOUT);

	while(!BaseHandler::time_to_die) {
		sleep(0.1);

		auto nonce = SubmissionCache::clone_best_nonce();
		if (!nonce)
			continue;

		if (nonce->forge_when() == previous_forge_time)
			continue;	// already submitted ?

		if ( time(nullptr) < nonce->forge_when() - FORGE_ADVANCE )
			continue;		// too soon to harass wallet node

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
				std::cerr  << ftime() << "[block_forger::block_forger] Too many connections! " << e.getErrorCode() << ": " << e.what() << std::endl;
				is_continue = true;
				break;
			} catch(const sql::SQLException &e) {
				// Could not connect to db.
				std::cerr  << ftime() << "[block_forger::block_forger] " << e.what() << std::endl;
				std::cerr << ftime() << "[block_forger::block_forger] Trying to connect in a moment. Attempt: " << counter + 1 <<  std::endl;
				sleep(1);
			}
			++counter;
			if(counter + 1 == DB_CONNECTION_ATTEMPT_COUNT){
				std::cerr << ftime() << "[block_forger::block_forger] DB connect failed..." << std::endl;
				throw;
			}
		}
		if(is_continue){
			continue;
		}

		// set block as ours so we don't immediately send forge reward to pool fee account!
		auto block = nonce->block();
		block->is_our_block( true );
		block->save();

		std::string secret(OUR_ACCOUNT_PASSPHRASE);
		const int secret_len = secret.size();

		for(int i=0; i<secret_len; i++)
			secret[i] = secret[i] ^ ( (i+1) % 16 );

		std::cout << ftime() << "Submitting nonce " << nonce->nonce() << " for block " << nonce->blockID() << std::endl;

		try {
			// wallet node will deal with broadcasting to network
			std::string forge_json = burst.submit_nonce( nonce->nonce(), nonce->accountID(), secret );

			JSON json(forge_json);
			if ( json.get_string("result") == "success" )
				previous_forge_time = nonce->forge_when();
		} catch(const CryptoCoins::server_issue &e) {
			// not good but we'll try again very soon anyway
		} catch(const JSON::parse_error &e) {
			// not good but we'll try again very soon anyway
		}

		// clean up
		secret.replace(0, std::string::npos, secret.size(), 'x');
	}
}
