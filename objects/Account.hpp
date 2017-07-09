#ifndef OBJECTS__ACCOUNT_HPP
#define OBJECTS__ACCOUNT_HPP

/*
		Create table Accounts (
			accountID					bigint unsigned not null unique,
			first_seen_when				timestamp not null default current_timestamp,
			reward_recipient			bigint unsigned,
			last_updated_when			timestamp null default null,
			account_name				varchar(255),
			mining_capacity				bigint unsigned,		# GiB
			is_capacity_estimated		boolean,
			account_RS_string			varchar(255) not null,
			has_used_this_pool			boolean not null default false,
			last_checked_at_block		bigint unsigned,
			last_nonce_when				timestamp null default null,
			primary key					(accountID)
		);
*/

#include "Account_.hxx"

#include "BurstCoin.hpp"
#include "JSON.hpp"


class Account: public Account_ {
	public:
		DORM::SearchMod<bool> needs_updating;

		virtual void search_prep( DORM::Query &query ) const override;

		virtual void save() override;

		uint64_t estimate_capacity(const uint64_t latest_blockID);
		void update_check(BurstCoin &burst, uint64_t blockID);

		static uint64_t sum_capacities();
		static std::unique_ptr<Account> load_or_create( uint64_t some_accountID );

		static void account_info_JSON( uint64_t accountID, JSON &info_json, const std::string &prefix = "" );
};


#endif
