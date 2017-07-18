#include "WS/updates.hpp"

#include "Block.hpp"
#include "Nonce.hpp"
#include "Share.hpp"
#include "Account.hpp"
#include "Reward.hpp"

#include "BlockCache.hpp"
#include "SubmissionCache.hpp"
#include "config.hpp"
#include "mining_info.hpp"

#include "pthread_np_shim.hpp"


// private static
std::string Handlers::WS::updates::block_msg;
std::string Handlers::WS::updates::shares_msg;
bool Handlers::WS::updates::shares_msg_changed = false;
std::string Handlers::WS::updates::awards_msg;
std::string Handlers::WS::updates::historic_shares_msg;
const time_t Handlers::WS::updates::started_when = time(nullptr);
std::mutex Handlers::WS::updates::updates_mutex;


// private non-static
uint64_t Handlers::WS::updates::latest_blockID = 0;
time_t Handlers::WS::updates::latest_block_when = 0;
std::map<uint64_t,time_t> Handlers::WS::updates::all_seen_accounts;


// public static
const std::string Handlers::WS::updates::websocket_protocol = "updates";


void Handlers::WS::updates::check_block() {
	if ( BlockCache::latest_blockID <= latest_blockID )
		return;

	// new block!
	auto latest_block = BlockCache::clone_latest_block();

	JSON json;
	json.add_uint( "block", latest_block->blockID() );
	json.add_uint( "newBlockWhen", latest_block->first_seen_when() );
	json.add_uint( "accountsRewardingUs", latest_block->num_potential_miners() );
	json.add_uint64( "difficulty", BLOCK0_BASE_TARGET / latest_block->base_target() );
	json.add_uint( "scoop", latest_block->scoop() );

	if ( !awards_msg.empty() ) {
		JSON awards_json( awards_msg.substr(7) );
		awards_json.delete_item( "Uptime" );
		awards_json.delete_item( "Deferred Payouts" );
		awards_json.delete_item( "Queued Payouts" );
		awards_json.delete_item( "Unconfirmed Payouts");

		json.add_item("awards", awards_json);
	}

	block_msg = "BLOCK:" + json.to_string();

	// now safe to update static value
	latest_blockID = latest_block->blockID();
	latest_block_when = latest_block->first_seen_when();

	// also update historic shares
	update_historic_shares();
}


void Handlers::WS::updates::update_historic_shares() {
	const uint64_t estimated_block_reward = Block::previous_reward_post_fee();

	auto historic_shares = Share::historic_shares( latest_blockID - 1, HISTORIC_BLOCK_COUNT );
	historic_shares->search();

	JSON_Array historic_shares_json_array;

	while( auto share = historic_shares->result() ) {
		update_account_info( share->accountID() );

		JSON share_json;
		share_json.add_string( "accountId", std::to_string( share->accountID() ) );
		share_json.add_double( "share", share->share_fraction() );

		// we should be able to estimate reward if we win current block
		share_json.add_number( "estimatedReward", share->share_fraction() * estimated_block_reward * (100 - CURRENT_BLOCK_REWARD_PERCENT) / 100 );

		Reward unpaid_rewards;
		unpaid_rewards.is_paid(false);
		unpaid_rewards.before_blockID( latest_blockID - MIN_PAYOUT_BLOCK_DELAY );
		unpaid_rewards.accountID( share->accountID() );
		unpaid_rewards.oldest_first(true);
		unpaid_rewards.search();

        bool ok_to_pay = false;
        uint64_t sum_amount = 0;
        // Sum of delayed payouts older than MAX_PAYOUT_BLOCK_DELAY
        uint64_t late_sum_amount = 0;
        uint64_t deferred_amount = 0;

        while( auto reward = unpaid_rewards.result() ) {
            // check whether miner has been waiting for ages
            // sum delayed payouts seperately.
            if (reward->defined_blockID() && reward->blockID() < latest_blockID - MAX_PAYOUT_BLOCK_DELAY) {
                late_sum_amount = reward->amount();
            } else {
                sum_amount += reward->amount();
            }
        }

        if ( sum_amount + late_sum_amount >= MINIMUM_PAYOUT ){
            ok_to_pay = true;
            sum_amount += late_sum_amount;
        } else if(late_sum_amount > 0) {
            // If there is delayed payouts pay just those
            ok_to_pay = true;
            deferred_amount = sum_amount;
            sum_amount = late_sum_amount;
        } else {
            deferred_amount = sum_amount;
        }

        if (ok_to_pay)
            share_json.add_uint64( "queuedPayouts", sum_amount );
        if (deferred_amount > 0)
            share_json.add_uint64( "deferredPayouts", deferred_amount );

		const uint64_t unconfirmed = Reward::total_unconfirmed_by_accountID( share->accountID() );

		if (unconfirmed > 0)
			share_json.add_uint64( "unconfirmedPayouts", unconfirmed );

		share_json.add_uint64( "totalPayouts", Reward::total_paid_by_accountID( share->accountID() ) );

		historic_shares_json_array.push_back( share_json );
	}

	historic_shares_msg = "HISTORIC:" + historic_shares_json_array.to_string();
}


void Handlers::WS::updates::check_shares() {
	shares_msg_changed = false;

	uint64_t estimated_block_reward = Block::previous_reward_post_fee();

	JSON_Array shares_json_array;

	const std::list< std::unique_ptr<Share> > &shares = SubmissionCache::clone_shares();
	for( const auto &share : shares ) {
		update_account_info( share->accountID() );

		JSON share_json;
		share_json.add_string( "accountId", std::to_string( share->accountID() ) );
		share_json.add_double( "share", share->share_fraction() );
		share_json.add_uint( "deadline", share->deadline() );
		// deadline string generated by client's browser

		// we should be able to estimate reward if we win current block
		share_json.add_number( "estimatedReward", share->share_fraction() * estimated_block_reward * CURRENT_BLOCK_REWARD_PERCENT / 100 );

		// miner info
		share_json.add_string( "miner", share->miner() );

		shares_json_array.push_back( share_json );
	}

	if (shares_json_array.size() == 0)
		return;

	std::string new_shares_msg = "SHARES:" + shares_json_array.to_string();

	if ( new_shares_msg == shares_msg )
		return;

	shares_msg = new_shares_msg;
	shares_msg_changed = true;

	// shares JSON has changed so also generate new awards
	generate_awards();
}


void Handlers::WS::updates::generate_awards() {
	JSON awards_json;

	auto latest_block = BlockCache::clone_latest_block();
	if (!latest_block)
		return;

	const time_t block_start_time = latest_block->first_seen_when();

	auto nonce = SubmissionCache::clone_best_nonce();
	if (nonce)
		add_account_award( awards_json, "Best Deadline", *nonce, block_start_time );

	nonce = SubmissionCache::clone_first_nonce();
	if (nonce)
		add_account_award( awards_json, "First Miner To Submit", *nonce, block_start_time );

	nonce = SubmissionCache::clone_worst_nonce();
	if (nonce)
		add_account_award( awards_json, "Worst Deadline", *nonce, block_start_time );

	nonce = SubmissionCache::clone_last_nonce();
	if (nonce)
		add_account_award( awards_json, "Last Miner To Submit", *nonce, block_start_time );

	awards_json.add_uint( "Miners Responded", SubmissionCache::share_count() );

	const uint64_t good_nonce_count = SubmissionCache::nonce_count();
	const uint64_t rejected_nonce_count = BlockCache::rejected_nonce_count();

	std::string nonces_award = std::to_string(good_nonce_count) + " good / " + std::to_string(rejected_nonce_count) + " bad";
	awards_json.add_string( "Nonces Submitted", nonces_award );

	// uptime
	awards_json.add_string( "Uptime", Nonce::deadline_to_string( time(nullptr) - started_when ) );

	// reward payouts in queue
	Reward rewards;
	rewards.is_paid( false );
	rewards.below_amount( MINIMUM_PAYOUT * BURST_TO_NQT );
	const uint64_t too_small = rewards.count();
	awards_json.add_uint( "Deferred Payouts", too_small );

	rewards.clear();
	rewards.is_paid( false );
	const uint64_t unpaid = rewards.count() - too_small;
	awards_json.add_uint( "Queued Payouts", unpaid );

	rewards.is_paid( true );
	rewards.is_confirmed( false );
	awards_json.add_uint( "Unconfirmed Payouts", rewards.count() );

	awards_msg = "AWARDS:" + awards_json.to_string();
}


void Handlers::WS::updates::add_account_award( JSON &awards, const std::string &award_name, Nonce &nonce, time_t block_start_time ) {
	update_account_info( nonce.accountID() );

	JSON award_info;

	award_info.add_string( "accountId", std::to_string( nonce.accountID() ) );
	award_info.add_uint( "submittedWhen", nonce.submitted_when() - block_start_time );

	awards.add_item( award_name, award_info );
}


void Handlers::WS::updates::update_account_info( uint64_t accountID ) {
	// don't use "update_check()" here as it massively slows down the whole server
	// there'll be a separate thread to update these accounts
	auto account = Account::load_or_create(accountID);

	if (account) {
		// we only need to add this account to the JSON if
		//  we've not seen it before OR
		//  it's newer than our timestamp

		if ( all_seen_accounts.find(accountID) == all_seen_accounts.end() || account->last_updated_when() > all_seen_accounts[accountID] )
			all_seen_accounts[accountID] = account->last_updated_when();
	}
}


void Handlers::WS::updates::send_new_accounts() {
	JSON_Array accounts_json_array;

	for(const auto &seen_account_pair : all_seen_accounts ) {
		auto insert_result = sent_accounts.insert( std::make_pair(seen_account_pair.first, 0) );
		auto &sent_account_pair = insert_result.first;

		if (sent_account_pair->second <= seen_account_pair.second) {
			sent_account_pair->second = time(nullptr);

			JSON account_json;
			Account::account_info_JSON( seen_account_pair.first, account_json );

			accounts_json_array.push_back( account_json );
		}
	}

	if (accounts_json_array.size() > 0) {
		const std::string accounts_msg = "ACCOUNTS:" + accounts_json_array.to_string();

		WebSocketFrame wsf;
		wsf.pack(accounts_msg);
		send_websocket_frame(wsf);
	}
}


Handlers::WS::updates::updates(MHD_socket sock, struct MHD_UpgradeResponseHandle *urh, Request *req) {
	this->sock = sock;
	this->urh = urh;
}


void Handlers::WS::updates::websocket_alert_prepare() {
	check_block();
	check_shares();
}


void Handlers::WS::updates::websocket_alert() {
    pthread_set_name_np(pthread_self(), "updates-alert");


    // Check db connection
    short counter = 0;
    while (true){
        try{
            DORM::DB::check_connection();
            break;
        } catch (const DORM::DB::connection_issue &e) {
            // DB being hammered by miners - try again in a moment
            std::cerr  << ftime() << "[updates::websocket_alert] Too many connections! " << e.getErrorCode() << ": " << e.what() << std::endl;
            return;
        } catch(const sql::SQLException &e) {
            // Could not connect to db.
            std::cerr << ftime() << "[updates::websocket_alert] " << e.what() << std::endl;
            std::cerr << ftime() << "[updates::websocket_alert] Trying to connect in a moment. Attempt: " << i+1 <<  std::endl;
            sleep(1);
        }
        ++counter;
        if(counter + 1 == DB_CONNECTION_ATTEMPT_COUNT){
            std::cerr << ftime() << "[updates::websocket_alert] DB connect failed..." << std::endl;
            throw;
        }
    }

	send_new_accounts();

	if (latest_blockID > our_blockID) {
		WebSocketFrame wsf;
		wsf.pack(block_msg);
		send_websocket_frame(wsf);

		wsf.pack(historic_shares_msg);
		send_websocket_frame(wsf);
	}

	if (shares_msg_changed || our_blockID == 0) {
		WebSocketFrame wsf;
		wsf.pack(shares_msg);
		send_websocket_frame(wsf);

		wsf.pack(awards_msg);
		send_websocket_frame(wsf);
	}

	our_blockID = latest_blockID;
}
