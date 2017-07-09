#ifndef OBJECTS__NONCE_HPP
#define OBJECTS__NONCE_HPP

/*
		Create table Nonces (
			accountID					bigint unsigned not null,
			blockID						bigint unsigned not null,
			nonce						bigint unsigned not null,
			submitted_when				timestamp not null default current_timestamp,
			deadline					bigint unsigned not null,
			deadline_string				varchar(255) not null,
			forge_when					timestamp not null default "0000-00-00 00:00:00",
			miner						varchar(255),
			primary key					(accountID, blockID, nonce),
			index						(blockID, accountID),
			index						(blockID, deadline desc)
		);
*/

#include "Nonce_.hxx"

class Nonce: public Nonce_ {
	public:
		DORM::SearchMod<bool> newest_first;
		DORM::SearchMod<bool> oldest_first;
		DORM::SearchMod<bool> best_first;
		DORM::SearchMod<bool> worst_first;
		DORM::SearchMod<uint64_t> before_blockID;

		virtual void search_prep( DORM::Query &query ) const override;

		static uint8_t *plot_nonce( uint64_t account_id, uint64_t nonce );

		static uint64_t calculate_deadline( uint64_t account_id, uint64_t nonce, uint64_t blockID, uint32_t scoop, uint64_t base_target, const std::string &gen_sig_str );
		static std::string deadline_to_string( uint64_t deadline );
};

#endif
