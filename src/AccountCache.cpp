#include "AccountCache.hpp"
#include "BlockCache.hpp"
#include "config.hpp"

AccountCache::account_map_t		AccountCache::accounts;
std::mutex						AccountCache::accounts_mutex;

AccountCache::request_map_t		AccountCache::requests;
std::mutex						AccountCache::requests_mutex;


std::unique_ptr<Account> &AccountCache::find_or_cache(const uint64_t accountID) {
	// we assume accounts mutex is already locked
	auto account_it = accounts.find(accountID);

	if ( account_it != accounts.end() )
		return account_it->second;

	auto account = Account::load_or_create(accountID);
	return accounts.insert( std::make_pair( accountID, std::move(account) ) ).first->second;
}


bool AccountCache::has_cached_correct_reward_recipient(const uint64_t accountID) {
	std::lock_guard<std::mutex>	accounts_guard(accounts_mutex);

	const auto &account_it = accounts.find(accountID);
	if ( account_it == accounts.end() )
		return false;

	if (account_it->second->last_checked_at_block() != BlockCache::latest_blockID)
		return false;

	return account_it->second->reward_recipient() == OUR_ACCOUNT_ID;
}


void AccountCache::ensure_request_mutex(const uint64_t accountID) {
	std::lock_guard<std::mutex> requests_guard(requests_mutex);

	if ( requests.find(accountID) != requests.end() )
		return;

	requests[accountID];
}


uint64_t AccountCache::request_reward_recipient(const uint64_t accountID) {
	std::lock_guard<std::mutex> request_mutex( requests[accountID] );

	// we've gained the lock
	// check cache again in case we were blocked by another request that received results
	if ( has_cached_correct_reward_recipient(accountID) )
		return OUR_ACCOUNT_ID;

	// looks like we need to send a request then
	try {
		BurstCoin burst(BURST_SERVERS, BURST_SERVER_TIMEOUT);

		JSON reward_json( burst.get_reward_recipient( std::to_string(accountID) ) );
		uint64_t reward_recipient = reward_json.get_uint64("rewardRecipient");

		if (reward_recipient != OUR_ACCOUNT_ID) {
			// if we have a cached account, it will need updating
			std::lock_guard<std::mutex>	accounts_guard(accounts_mutex);

			const auto &account_it = accounts.find(accountID);
			if ( account_it == accounts.end() )
				return reward_recipient;

			// we have cached account
			auto &account = account_it->second;
			account->reward_recipient(reward_recipient);
			account->last_checked_at_block(BlockCache::latest_blockID);
			account->save();

			return reward_recipient;
		}
	} catch(const CryptoCoins::server_issue &e) {
		// pass this to caller
		throw(e);
	} catch(const JSON::parse_error &e) {
		// communication with server didn't go to plan...
		throw CryptoCoins::server_issue();
	}

	std::lock_guard<std::mutex>	accounts_guard(accounts_mutex);

	// update account
	auto &account = find_or_cache(accountID);
	account->reward_recipient(OUR_ACCOUNT_ID);
	account->last_checked_at_block(BlockCache::latest_blockID);
	account->save();

	return OUR_ACCOUNT_ID;
}


uint64_t AccountCache::get_account_reward_recipient(const uint64_t accountID) {
	// this is a separate call so can reduce the amount of time we hold the accounts mutex
	if ( has_cached_correct_reward_recipient(accountID) )
		return OUR_ACCOUNT_ID;

	// check via wallet nodes
	// ONLY ONE REQUEST SHOULD BE ONGOING PER ACCOUNTID
	// other requests for the same accountID should block until there's a valid answer
	ensure_request_mutex(accountID);

	return request_reward_recipient(accountID);
}


time_t AccountCache::last_submit_timestamp(const uint64_t accountID) {
	std::lock_guard<std::mutex>	accounts_guard(accounts_mutex);

	auto &account = find_or_cache(accountID);
	return account->last_nonce_when();
}


void AccountCache::new_submit_timestamp(const uint64_t accountID, const time_t new_timestamp) {
	std::lock_guard<std::mutex>	accounts_guard(accounts_mutex);

	auto &account = find_or_cache(accountID);
	account->last_nonce_when(new_timestamp);
}


void AccountCache::update_account_capacity(const uint64_t accountID, const uint64_t capacity) {
	std::lock_guard<std::mutex>	accounts_guard(accounts_mutex);

	auto &account = find_or_cache(accountID);
	account->is_capacity_estimated(false);
	account->mining_capacity(capacity);
	account->save();
}


void AccountCache::update_reward_recipients(const uint64_t latest_blockID, const JSON_Array &accounts_json_array) {
	const int n_accounts = accounts_json_array.size();
	for (int i=0; i<n_accounts; ++i) {
		const uint64_t accountID = accounts_json_array.get_uint64(i);

		std::lock_guard<std::mutex>	accounts_guard(accounts_mutex);

		// update account
		auto &account = find_or_cache(accountID);
		account->reward_recipient(OUR_ACCOUNT_ID);
		account->last_checked_at_block(latest_blockID);
		account->save();
	}
}
