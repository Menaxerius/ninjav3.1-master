#ifndef OBJECTS__BONUS_HPP
#define OBJECTS__BONUS_HPP

/*
		Create table Bonuses (
			bonusID						serial,
			tx_id						bigint unsigned not null,
			amount						bigint unsigned not null,
			seen_when					timestamp not null default current_timestamp,
			primary key					(bonusID),
			index						(tx_id)
		);
*/

#include "Bonus_.hxx"

class Bonus: public Bonus_ {
	public:
		DORM::SearchMod<bool> newest_first;

		virtual void search_prep( DORM::Query &query ) const override;
};

#endif
