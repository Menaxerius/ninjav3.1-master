#ifndef OBJECTS__BLOCK_HPP
#define OBJECTS__BLOCK_HPP

/*
		Create table Blocks (
			blockID						bigint unsigned not null unique,
			first_seen_when				timestamp not null default current_timestamp,
			best_nonce_account_id		bigint unsigned,
			generator_account_id		bigint unsigned,
			block_reward				bigint unsigned,
			is_our_block				boolean not null default false,
			has_been_shared				boolean not null default false,
			base_target					bigint unsigned,
			forged_when					timestamp null default null,
			scoop						int unsigned,
			nonce						bigint unsigned,
			generation_signature		char(64),
			deadline					int unsigned,
			our_best_deadline			int unsigned,
			num_potential_miners		int unsigned,
			num_rejected_nonces			int unsigned not null default 0,
			block_id					bigint unsigned,
			tx_fees						bigint unsigned,
			primary key					(blockID),
			index						(is_our_block, has_been_shared)
		);
*/

#include "Block_.hxx"


CHILD_OBJECTS(Nonce, block_nonces);
CHILD_OBJECTS(Share, block_shares);
CHILD_OBJECTS(Reward, block_rewards);



class Block: public Block_ {
	private:
		typedef uint8_t gen_sig_array_t[32];

	public:
		DORM::SearchMod<uint64_t> after_blockID;
		DORM::SearchMod<uint64_t> before_blockID;
		DORM::SearchMod<bool> has_reward_value;
		DORM::SearchMod<bool> newest_first;

		virtual void search_prep( DORM::Query &query ) const override;

		void reward_miners();

		static uint64_t calc_block_reward( const uint64_t block_height );
		static void unpack_generation_signature( const std::string &gen_sig_str, gen_sig_array_t &gen_sig_array );
		static uint32_t calculate_scoop( const uint64_t scoop_blockID, const std::string &scoop_generation_signature );
};

#endif
