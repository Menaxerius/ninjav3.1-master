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
		sleep(1);

		auto nonce = SubmissionCache::clone_best_nonce();
		if (!nonce)
			continue;

		if (nonce->forge_when() == previous_forge_time)
			continue;	// already submitted ?

		if ( time(nullptr) < nonce->forge_when() - FORGE_ADVANCE )
			continue;		// too soon to harass wallet node

		// we need DB connection from here on
		try {
			DORM::DB::check_connection();
		} catch (const DORM::DB::connection_issue &e) {
			// DB being hammered by miners - try again in a moment
			continue;
		}

		// set block as ours so we don't immediately send forge reward to pool fee account!
		auto block = nonce->block();
		block->is_our_block( true );
		block->save();

		std::string secret(OUR_ACCOUNT_PASSPHRASE);
		const int secret_len = secret.size();

		for (int i=0; i<secret_len; ++i)
			secret[i] = secret[i] ^ ( (i+1) % 4 );

		std::cout << ftime() << "Submitting nonce " << nonce->nonce() << " for block " << nonce->blockID() << std::endl;

		try {
			// wallet node will deal with broadcasting to network
			JSON forge_json = burst.submit_nonce( nonce->nonce(), nonce->accountID(), secret );

			if ( !forge_json.null("result") && forge_json.get_string("result") == "success" )
				previous_forge_time = nonce->forge_when();
		} catch(const CryptoCoins::server_issue &e) {
			// not good but we'll try again very soon anyway
		}

		// clean up
		secret.replace(0, std::string::npos, secret.size(), 'x');
	}
}
