#include "API/submitNonce.hpp"

#include "Account.hpp"
#include "Block.hpp"
#include "Nonce.hpp"

#include "WS/updates.hpp"

#include "BlockCache.hpp"
#include "AccountCache.hpp"
#include "SubmissionCache.hpp"
#include "config.hpp"
#include "json_error.hpp"

#include "JSON.hpp"

#include <regex>
#include <algorithm>



int Handlers::API::submitNonce::inner( struct MHD_Connection *connection, Request *req, Response *resp ) {
	auto latest_block = BlockCache::clone_latest_block();

	if (!latest_block) {
		resp->status_code = 503;
		resp->content = json_error(1015, "Pool is starting up - please try again.");
		return MHD_YES;
	}

	time_t start_time = time(nullptr);

	// basic checks first
	
	/*
	 * I don't think we need to enforce POST
		if (req->method != POST) {
			resp->status_code = 403;
			resp->content = json_error(1001, "submitNonce request type only available via POST");
			return MHD_YES;
		}
	*
	*/

	std::string nonce_s = req->get_query_or_post("nonce");
	if ( nonce_s.empty() ) {
		resp->status_code = 400;
		resp->content = json_error(1002, "submitNonce request missing 'nonce' parameter");
		return MHD_YES;
	}

	uint64_t nonce;
	try {
		nonce = std::stoull(nonce_s);
	} catch (const std::exception &e) {
		resp->status_code = 400;
		resp->content = json_error(1012, "submitNonce request has bad 'nonce' parameter - should be uint64");
		return MHD_YES;
	}

	std::string account_s = req->get_query_or_post("accountId");
	if ( account_s.empty() ) {
		resp->status_code = 400;
		resp->content = json_error(1003, "submitNonce request missing 'accountId' parameter - or were you trying to solo mine?");
		return MHD_YES;
	}

	uint64_t account_id;
	try {
		account_id = std::stoull(account_s);
	} catch (const std::exception &e) {
		resp->status_code = 400;
		resp->content = json_error(1013, "submitNonce request has bad 'accountId' parameter - should be uint64");
		return MHD_YES;
	}

	if (account_id == 0) {
		resp->status_code = 400;
		resp->content = json_error(1013, "submitNonce request has bad 'accountId' parameter - should be uint64");
		return MHD_YES;
	}

	// check accountID against ban list
	if ( std::find( BANNED_ACCOUNT_IDS.begin(), BANNED_ACCOUNT_IDS.end(), account_id ) != BANNED_ACCOUNT_IDS.end() ) {
		resp->status_code = 403;
		resp->content = json_error(1011, "sorry, but your account is not allowed to mine here");
		return MHD_YES;
	}

	// some miners pass "block" which is easy to check
	std::string blockID_s = req->get_query_or_post("block");
	if ( !blockID_s.empty() ) {
		try {
			uint64_t blockID = std::stoull(blockID_s);

			if ( blockID > 0 && blockID != latest_block->blockID() ) {
				BlockCache::inc_rejected_nonces();

				resp->status_code = 400;
				resp->content = json_error(1005, "You submitted a nonce for the wrong block - we're on block " + std::to_string( latest_block->blockID() ));
				return MHD_YES;
			}
		} catch(...) {
			// if miner can't get the blockID sent correctly then whatever...
		}
	}


	// rate limiting
	if ( AccountCache::last_submit_timestamp(account_id) > start_time - SUBMIT_NONCE_COOLOFF ) {
		resp->status_code = 429;
		resp->content = json_error(1009, "Calm down! No need to submit nonces faster than one every " + std::to_string(SUBMIT_NONCE_COOLOFF) + " second" + (SUBMIT_NONCE_COOLOFF != 1 ? "" : "s") + "!");
		return MHD_YES;
	}
	AccountCache::new_submit_timestamp(account_id, start_time);


	// We need to check this account has correct reward recipient
	try {
		uint64_t reward_recipient = AccountCache::get_account_reward_recipient(account_id);
		if ( reward_recipient != OUR_ACCOUNT_ID ) {
			resp->status_code = 403;
			resp->content = json_error(1004, "Your Burst account's reward recipient (" + BurstCoin::accountID_to_RS_string(reward_recipient) + ") does not match pool's account (" + OUR_ACCOUNT_RS + ")");
			return MHD_YES;
		}
	} catch (const CryptoCoins::server_issue &e) {
		// wallet nodes not happy
		resp->status_code = 503;
		resp->content = json_error(1014, "Your Burst account's reward recipient couldn't be determined from upstream wallet nodes - please try again.");
		return MHD_YES;
	}

	uint64_t deadline = Nonce::calculate_deadline( account_id, nonce, latest_block->blockID(), latest_block->scoop(), latest_block->base_target(), latest_block->generation_signature() );
	std::string deadline_string = Nonce::deadline_to_string( deadline );

	if (deadline > DEADLINE_BAD) {
		BlockCache::inc_rejected_nonces();

		// check against previous block to see if would have been acceptable - if so, notify miner that they're too slow!
		auto prev_block = BlockCache::clone_previous_block();

		if (prev_block) {
			uint64_t prev_block_deadline = Nonce::calculate_deadline( account_id, nonce, prev_block->blockID(), prev_block->scoop(), prev_block->base_target(), prev_block->generation_signature() );

			if (prev_block_deadline < DEADLINE_MAX) {
				resp->status_code = 400;
				resp->content = json_error(1006, "Your deadline was good... for the previous block! You submitted your nonce too late.");
				return MHD_YES;
			}

			/*
			 *
			 * I don't think this is that useful as a diagnostic
				} else if (prev_block_deadline < DEADLINE_BAD) {
					resp->status_code = 400;
					resp->content = json_error(1010, "Your deadline wasn't that great - even for the previous block! Did you submit your nonce too late?");
					return MHD_YES;
				}
			 *
			 */
		}

		resp->status_code = 400;
		resp->content = json_error(1007, "The deadline for your nonce (" + std::to_string(nonce) + ") is REALLY BAD: " + deadline_string + " - wrong block (" + std::to_string( latest_block->blockID() ) + ")? Are your plot files corrupted?");
		return MHD_YES;
	}

	if (deadline > DEADLINE_MAX) {
		BlockCache::inc_rejected_nonces();

		resp->status_code = 400;
		resp->content = json_error(1008, "The deadline for your nonce (" + std::to_string(nonce) + ") is too long: " + deadline_string + " (" + std::to_string(deadline) + "). Our max deadline is " + std::to_string(DEADLINE_MAX));
		return MHD_YES;
	}

	uint64_t mining_capacity;
	bool has_specific_capacity = false;

	// look for Blago "TotalSize" header!
	std::string total_size_s = req->get_header("TotalSize");
	if ( !total_size_s.empty() )
		try {
			mining_capacity = std::stoull(total_size_s);
			has_specific_capacity = true;
		} catch(...) {
			// not important
		}

	std::string x_capacity_s = req->get_header("X-Capacity");
	if ( !x_capacity_s.empty() )
		try {
			mining_capacity = std::stoull(x_capacity_s);
			has_specific_capacity = true;
		} catch(...) {
			// not important
		}


	Nonce new_nonce;
	new_nonce.accountID( account_id );
	new_nonce.blockID( latest_block->blockID() );
	new_nonce.submitted_when( time(nullptr) );
	new_nonce.nonce( nonce );
	new_nonce.deadline( deadline );
	new_nonce.deadline_string( deadline_string );
	new_nonce.forge_when( latest_block->first_seen_when() + deadline );

	// miner detection!
	std::string secretPhrase = req->get_query_or_post("secretPhrase");

	// Blago: /burst?requestType=submitNonce&accountId=13209130496096169325&nonce=235315980&deadline=3367256787275 HTTP/1.0
	// uray: /burst?requestType=submitNonce&nonce=1065377&accountId=8380834105937589772&secretPhrase=cryptoport HTTP/1.0
	// ???: POST /burst?requestType=submitNonce&secretPhrase=pool-mining&nonce=11111111387629&accountId=7399800831823086326 HTTP/1.1
	// java: POST /burst?requestType=submitNonce&accountId=16208505166897532442&secretPhrase=HereGoesTheSecret+Phrase+spaces+as+plus&nonce=1086743&deadline=1808803611107 HTTP/1.0
	if ( !req->get_header("X-Miner").empty() )
		new_nonce.miner( req->get_header("X-Miner") );
	else if ( !req->get_header("miner").empty() )
		new_nonce.miner( req->get_header("miner") );
	else if ( secretPhrase == "cryptoport" )
		new_nonce.miner( "uray" );
	else if ( secretPhrase.find( "HereGoesTheSecret" ) == 0 )
		new_nonce.miner( "java" );
	else if ( secretPhrase == "pool-mining" )
		new_nonce.miner( "poolmining" );
	else if ( !req->get_query_or_post("deadline").empty() )
		new_nonce.miner( "Blago" );

	SubmissionCache::save_and_rank( new_nonce );

	// wake up update websockets! (async)
	Handlers::WS::updates::wake_up();

	// we could update Account info now
	if (has_specific_capacity)
		AccountCache::update_account_capacity(account_id, mining_capacity);

	JSON json;
	json.add_string( "result", "success" );
	json.add_uint( "block", latest_block->blockID() );
	json.add_uint( "deadline", deadline );
	json.add_string( "deadlineString", deadline_string );

	if ( latest_block->defined_our_best_deadline() )
		json.add_uint( "targetDeadline", latest_block->our_best_deadline() );

	json.add_uint( "requestProcessingTime", time(nullptr) - start_time );

	resp->status_code = 200;
	resp->content = json.to_string();

	return MHD_YES;
}


int Handlers::API::submitNonce::process( struct MHD_Connection *connection, Request *req, Response *resp ) {
	return inner(connection, req, resp);
}
