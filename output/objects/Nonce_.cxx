// GENERATED - DO NOT EDIT!

// our class
#include "Nonce_.hxx"
#include "Nonce.hpp"

// other classes

	#include "Account.hpp"

	#include "Block.hpp"



#include <memory>


const std::vector<DORM::Object::Info> Nonce_::column_info = {
	
		
		{
			"accountID",
			1,
			true,
			true,
			false
		},
	
		
		{
			"blockID",
			2,
			true,
			true,
			false
		},
	
		
		{
			"nonce",
			3,
			true,
			true,
			false
		},
	
		
		{
			"submitted_when",
			4,
			false,
			true,
			true
		},
	
		
		{
			"deadline",
			5,
			false,
			true,
			false
		},
	
		
		{
			"deadline_string",
			6,
			false,
			true,
			false
		},
	
		
		{
			"forge_when",
			7,
			false,
			true,
			true
		},
	
		
		{
			"miner",
			8,
			false,
			false,
			false
		}
	
};


const std::vector< std::function< void(const DORM::Resultset &result, Nonce_ &obj) > > Nonce_::column_resultset_function = {
	
		
		[](const DORM::Resultset &result, Nonce_ &obj){ obj.columns[0] = static_cast<uint64_t>( result.getUInt64(1) ); },
	
		
		[](const DORM::Resultset &result, Nonce_ &obj){ obj.columns[1] = static_cast<uint64_t>( result.getUInt64(2) ); },
	
		
		[](const DORM::Resultset &result, Nonce_ &obj){ obj.columns[2] = static_cast<uint64_t>( result.getUInt64(3) ); },
	
		
		[](const DORM::Resultset &result, Nonce_ &obj){ obj.columns[3] = static_cast<DORM::Timestamp>( result.getString(4) ); },
	
		
		[](const DORM::Resultset &result, Nonce_ &obj){ obj.columns[4] = static_cast<uint64_t>( result.getUInt64(5) ); },
	
		
		[](const DORM::Resultset &result, Nonce_ &obj){ obj.columns[5] = static_cast<std::string>( result.getString(6) ); },
	
		
		[](const DORM::Resultset &result, Nonce_ &obj){ obj.columns[6] = static_cast<DORM::Timestamp>( result.getString(7) ); },
	
		
		[](const DORM::Resultset &result, Nonce_ &obj){ obj.columns[7] = static_cast<std::string>( result.getString(8) ); }
	
};


void Nonce_::clear() {
	// explictly [re]set to "empty" values
	
		
		
			columns[0] = static_cast<uint64_t>(0);
		
	
		
		
			columns[1] = static_cast<uint64_t>(0);
		
	
		
		
			columns[2] = static_cast<uint64_t>(0);
		
	
		
		
			columns[3] = DORM::Timestamp();
		
	
		
		
			columns[4] = static_cast<uint64_t>(0);
		
	
		
		
			columns[5] = std::string();
		
	
		
		
			columns[6] = DORM::Timestamp();
		
	
		
		
			columns[7] = std::string();
		
	

	Object::clear();
}


std::unique_ptr<DORM::Object> Nonce_::make_unique() {
	return std::make_unique<Nonce>();
}


void Nonce_::column_from_resultset( int i, const DORM::Resultset &result ) {
	( column_resultset_function[i] )( result, *this );
}


std::unique_ptr<Nonce> Nonce_::load() {
	DORM::Object::search();
	return result();
}


std::unique_ptr<Nonce> Nonce_::load( uint64_t key_accountID, uint64_t key_blockID, uint64_t key_nonce ) {
	Nonce obj;

	
		obj.accountID( key_accountID );
	
		obj.blockID( key_blockID );
	
		obj.nonce( key_nonce );
	

	return obj.load();
}


std::unique_ptr<Nonce> Nonce_::load(const DORM::Object &obj) {
	Nonce tmp;
	tmp.copy_columns(obj, true);
	return tmp.load();
}


std::unique_ptr<Nonce> Nonce_::result() {
	if ( !resultset || !resultset->next() ) {
		resultset.reset();
		return std::unique_ptr<Nonce>();
	}

	auto obj = std::make_unique<Nonce>();
	obj->set_from_resultset( *resultset );
	return obj;
}


void Nonce_::search_and_destroy() {
	search();

	while( auto victim = result() )
		victim->delete_obj();
}


std::unique_ptr<Nonce> Nonce_::clone() const {
	auto obj = std::make_unique<Nonce>();

	obj->copy_columns(*this, false);

	for (auto &column : obj->columns)
		column.changed = false;
	return obj;
}


// navigators

	std::unique_ptr<Account> Nonce_::account() const {
		
			return Account::load(*this);
		
	}

	std::unique_ptr<Block> Nonce_::block() const {
		
			return Block::load(*this);
		
	}

