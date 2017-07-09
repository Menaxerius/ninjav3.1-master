// GENERATED - DO NOT EDIT!

// our class
#include "Share_.hxx"
#include "Share.hpp"

// other classes

	#include "Block.hpp"

	#include "Account.hpp"



#include <memory>


const std::vector<DORM::Object::Info> Share_::column_info = {
	
		
		{
			"blockID",
			1,
			true,
			true,
			false
		},
	
		
		{
			"accountID",
			2,
			true,
			true,
			false
		},
	
		
		{
			"share_fraction",
			3,
			false,
			true,
			false
		},
	
		
		{
			"deadline",
			4,
			false,
			true,
			false
		},
	
		
		{
			"deadline_string",
			5,
			false,
			true,
			false
		},
	
		
		{
			"miner",
			6,
			false,
			false,
			false
		}
	
};


const std::vector< std::function< void(const DORM::Resultset &result, Share_ &obj) > > Share_::column_resultset_function = {
	
		
		[](const DORM::Resultset &result, Share_ &obj){ obj.columns[0] = static_cast<uint64_t>( result.getUInt64(1) ); },
	
		
		[](const DORM::Resultset &result, Share_ &obj){ obj.columns[1] = static_cast<uint64_t>( result.getUInt64(2) ); },
	
		
		[](const DORM::Resultset &result, Share_ &obj){ obj.columns[2] = static_cast<double>( result.getDouble(3) ); },
	
		
		[](const DORM::Resultset &result, Share_ &obj){ obj.columns[3] = static_cast<uint32_t>( result.getUInt(4) ); },
	
		
		[](const DORM::Resultset &result, Share_ &obj){ obj.columns[4] = static_cast<std::string>( result.getString(5) ); },
	
		
		[](const DORM::Resultset &result, Share_ &obj){ obj.columns[5] = static_cast<std::string>( result.getString(6) ); }
	
};


void Share_::clear() {
	// explictly [re]set to "empty" values
	
		
		
			columns[0] = static_cast<uint64_t>(0);
		
	
		
		
			columns[1] = static_cast<uint64_t>(0);
		
	
		
		
			columns[2] = static_cast<double>(0);
		
	
		
		
			columns[3] = static_cast<uint32_t>(0);
		
	
		
		
			columns[4] = std::string();
		
	
		
		
			columns[5] = std::string();
		
	

	Object::clear();
}


std::unique_ptr<DORM::Object> Share_::make_unique() {
	return std::make_unique<Share>();
}


void Share_::column_from_resultset( int i, const DORM::Resultset &result ) {
	( column_resultset_function[i] )( result, *this );
}


std::unique_ptr<Share> Share_::load() {
	DORM::Object::search();
	return result();
}


std::unique_ptr<Share> Share_::load( uint64_t key_blockID, uint64_t key_accountID ) {
	Share obj;

	
		obj.blockID( key_blockID );
	
		obj.accountID( key_accountID );
	

	return obj.load();
}


std::unique_ptr<Share> Share_::load(const DORM::Object &obj) {
	Share tmp;
	tmp.copy_columns(obj, true);
	return tmp.load();
}


std::unique_ptr<Share> Share_::result() {
	if ( !resultset || !resultset->next() ) {
		resultset.reset();
		return std::unique_ptr<Share>();
	}

	auto obj = std::make_unique<Share>();
	obj->set_from_resultset( *resultset );
	return obj;
}


void Share_::search_and_destroy() {
	search();

	while( auto victim = result() )
		victim->delete_obj();
}


std::unique_ptr<Share> Share_::clone() const {
	auto obj = std::make_unique<Share>();

	obj->copy_columns(*this, false);

	for (auto &column : obj->columns)
		column.changed = false;
	return obj;
}


// navigators

	std::unique_ptr<Block> Share_::block() const {
		
			return Block::load(*this);
		
	}

	std::unique_ptr<Account> Share_::account() const {
		
			return Account::load(*this);
		
	}

