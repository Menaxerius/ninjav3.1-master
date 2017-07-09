#ifndef OBJECTS__SHARE_HPP
#define OBJECTS__SHARE_HPP

/*
		Create table Shares (
			blockID						bigint unsigned not null,
			accountID					bigint unsigned not null,
			share_fraction				double not null,
			deadline					int unsigned not null,
			deadline_string				varchar(255) not null,
			miner						varchar(255),
			primary key					(blockID, accountID),
			index						(blockID, share_fraction)
		);
*/

#include "Share_.hxx"

class Share: public Share_ {
	public:
		DORM::SearchMod<uint64_t> before_blockID;
		DORM::SearchMod<uint64_t> after_blockID;
		DORM::SearchMod<bool> historic_average;
		DORM::SearchMod<bool> weighted_deadline;
		DORM::SearchMod<bool> biggest_first;

		virtual void search_prep( DORM::Query &query ) const override;

		static std::unique_ptr<Share> historic_shares( uint64_t to_blockID, uint64_t block_count );
};

#endif
