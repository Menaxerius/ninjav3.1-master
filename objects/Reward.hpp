#ifndef OBJECTS__REWARD_HPP
#define OBJECTS__REWARD_HPP

/*
		Create table Rewards (
			rewardID					serial,
			accountID					bigint unsigned not null,
			blockID						bigint unsigned not null,
			bonusID						bigint unsigned,						# set if reward based on a bonus payment
			amount						bigint unsigned not null,
			is_paid						boolean not null default false,
			tx_id						bigint unsigned,
			is_confirmed				boolean not null default false,
			paid_at_block_id			bigint unsigned,
			primary key					(rewardID),
			index						(is_paid, is_confirmed, accountID, blockID)
		);
*/

#include "Reward_.hxx"

class Reward: public Reward_ {
	public:
		DORM::SearchMod<uint64_t> after_blockID;
		DORM::SearchMod<uint64_t> before_blockID;
		DORM::SearchMod<uint64_t> below_amount;
		DORM::SearchMod<uint64_t> paid_before_block_id;
		DORM::SearchMod<bool> oldest_first;
		DORM::SearchMod<bool> oldest_per_account_first;
		DORM::SearchMod<bool> one_result_per_account;

		virtual void search_prep( DORM::Query &query ) const override;

		uint64_t sum_amount();

		static uint64_t total_unpaid();
		static uint64_t total_paid_by_accountID( uint64_t accountID );
		static uint64_t total_unconfirmed_by_accountID( uint64_t accountID );
};

#endif
