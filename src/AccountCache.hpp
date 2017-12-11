#ifndef SRC__ACCOUNTCACHE_HPP
#define SRC__ACCOUNTCACHE_HPP

#include "Account.hpp"

#include <map>
#include <mutex>
#include <unistd.h>

class AccountCache {
	private:
		typedef std::map< uint64_t, std::unique_ptr<Account> > account_map_t;
	
		static account_map_t		accounts;
		static std::mutex			accounts_mutex;
		
		typedef std::map< uint64_t, std::mutex > request_map_t;
		static request_map_t		requests;
		static std::mutex			requests_mutex;
		
		static std::unique_ptr<Account> &find_or_cache(const uint64_t accountID);
		static bool has_cached_correct_reward_recipient(const uint64_t accountID);
		
		static void ensure_request_mutex(const uint64_t accountID);
		static uint64_t request_reward_recipient(const uint64_t accountID);
		
	public:
		static uint64_t get_account_reward_recipient(const uint64_t accountID);
		static time_t last_submit_timestamp(const uint64_t accountID);
		static void new_submit_timestamp(const uint64_t accountID, const time_t new_timestamp);
		static void update_account_capacity(const uint64_t accountID, const uint64_t capacity);
		static void update_reward_recipients(const uint64_t latest_blockID, const JSON_Array &accounts_json_array);
};

#endif
