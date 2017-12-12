#ifndef OBJECTS__FEE_HPP
#define OBJECTS__FEE_HPP

/*
		Create table Fees (
			feeID						serial,
			amount						bigint unsigned not null,
			is_paid						boolean not null default false,
			tx_id						bigint unsigned,
			is_confirmed				boolean not null default false,
			paid_at_block_id			bigint unsigned,
			primary key					(feeID),
			index						(is_paid, is_confirmed)
		);
*/

#include "Fee_.hxx"

class Fee: public Fee_ {
	public:
		DORM::SearchMod<uint64_t> paid_before_block_id;
		DORM::SearchMod<bool> oldest_first;

		virtual void search_prep( DORM::Query &query ) const override;
};

#endif
