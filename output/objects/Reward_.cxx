// GENERATED - DO NOT EDIT!

// our class
#include "Reward_.hxx"
#include "Reward.hpp"

// other classes

	#include "Account.hpp"

	#include "Block.hpp"

	#include "Bonus.hpp"



#include <memory>


const std::vector<DORM::Object::Info> Reward_::column_info = {
	
		
		{
			"rewardID",
			1,
			true,
			false,
			false
		},
	
		
		{
			"accountID",
			2,
			false,
			true,
			false
		},
	
		
		{
			"blockID",
			3,
			false,
			true,
			false
		},
	
		
		{
			"bonusID",
			4,
			false,
			false,
			false
		},
	
		
		{
			"amount",
			5,
			false,
			true,
			false
		},
	
		
		{
			"is_paid",
			6,
			false,
			true,
			true
		},
	
		
		{
			"tx_id",
			7,
			false,
			false,
			false
		},
	
		
		{
			"is_confirmed",
			8,
			false,
			true,
			true
		},
	
		
		{
			"paid_at_block_id",
			9,
			false,
			false,
			false
		}
	
};


const std::vector< std::function< void(const DORM::Resultset &result, Reward_ &obj) > > Reward_::column_resultset_function = {
	
		
		[](const DORM::Resultset &result, Reward_ &obj){ obj.columns[0] = static_cast<uint64_t>( result.getUInt64(1) ); },
	
		
		[](const DORM::Resultset &result, Reward_ &obj){ obj.columns[1] = static_cast<uint64_t>( result.getUInt64(2) ); },
	
		
		[](const DORM::Resultset &result, Reward_ &obj){ obj.columns[2] = static_cast<uint64_t>( result.getUInt64(3) ); },
	
		
		[](const DORM::Resultset &result, Reward_ &obj){ obj.columns[3] = static_cast<uint64_t>( result.getUInt64(4) ); },
	
		
		[](const DORM::Resultset &result, Reward_ &obj){ obj.columns[4] = static_cast<uint64_t>( result.getUInt64(5) ); },
	
		
		[](const DORM::Resultset &result, Reward_ &obj){ obj.columns[5] = static_cast<bool>( result.getBoolean(6) ); },
	
		
		[](const DORM::Resultset &result, Reward_ &obj){ obj.columns[6] = static_cast<uint64_t>( result.getUInt64(7) ); },
	
		
		[](const DORM::Resultset &result, Reward_ &obj){ obj.columns[7] = static_cast<bool>( result.getBoolean(8) ); },
	
		
		[](const DORM::Resultset &result, Reward_ &obj){ obj.columns[8] = static_cast<uint64_t>( result.getUInt64(9) ); }
	
};


void Reward_::clear() {
	// explictly [re]set to "empty" values
	
		
		
			columns[0] = static_cast<uint64_t>(0);
		
	
		
		
			columns[1] = static_cast<uint64_t>(0);
		
	
		
		
			columns[2] = static_cast<uint64_t>(0);
		
	
		
		
			columns[3] = static_cast<uint64_t>(0);
		
	
		
		
			columns[4] = static_cast<uint64_t>(0);
		
	
		
		
			columns[5] = false;
		
	
		
		
			columns[6] = static_cast<uint64_t>(0);
		
	
		
		
			columns[7] = false;
		
	
		
		
			columns[8] = static_cast<uint64_t>(0);
		
	

	Object::clear();
}


std::unique_ptr<DORM::Object> Reward_::make_unique() {
	return std::make_unique<Reward>();
}


void Reward_::column_from_resultset( int i, const DORM::Resultset &result ) {
	( column_resultset_function[i] )( result, *this );
}


std::unique_ptr<Reward> Reward_::load() {
	DORM::Object::search();
	return result();
}


std::unique_ptr<Reward> Reward_::load( uint64_t key_rewardID ) {
	Reward obj;

	
		obj.rewardID( key_rewardID );
	

	return obj.load();
}


std::unique_ptr<Reward> Reward_::load(const DORM::Object &obj) {
	Reward tmp;
	tmp.copy_columns(obj, true);
	return tmp.load();
}


std::unique_ptr<Reward> Reward_::result() {
	if ( !resultset || !resultset->next() ) {
		resultset.reset();
		return std::unique_ptr<Reward>();
	}

	auto obj = std::make_unique<Reward>();
	obj->set_from_resultset( *resultset );
	return obj;
}


void Reward_::search_and_destroy() {
	search();

	while( auto victim = result() )
		victim->delete_obj();
}


std::unique_ptr<Reward> Reward_::clone() const {
	auto obj = std::make_unique<Reward>();

	obj->copy_columns(*this, false);

	for (auto &column : obj->columns)
		column.changed = false;
	return obj;
}


// navigators

	std::unique_ptr<Account> Reward_::account() const {
		
			return Account::load(*this);
		
	}

	std::unique_ptr<Block> Reward_::block() const {
		
			return Block::load(*this);
		
	}

	std::unique_ptr<Bonus> Reward_::bonus() const {
		
			return Bonus::load(*this);
		
	}

