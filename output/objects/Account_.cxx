// GENERATED - DO NOT EDIT!

// our class
#include "Account_.hxx"
#include "Account.hpp"

// other classes



#include <memory>


const std::vector<DORM::Object::Info> Account_::column_info = {
	
		
		{
			"accountID",
			1,
			true,
			true,
			false
		},
	
		
		{
			"first_seen_when",
			2,
			false,
			true,
			true
		},
	
		
		{
			"reward_recipient",
			3,
			false,
			false,
			false
		},
	
		
		{
			"last_updated_when",
			4,
			false,
			false,
			true
		},
	
		
		{
			"account_name",
			5,
			false,
			false,
			false
		},
	
		
		{
			"mining_capacity",
			6,
			false,
			false,
			false
		},
	
		
		{
			"is_capacity_estimated",
			7,
			false,
			false,
			false
		},
	
		
		{
			"account_RS_string",
			8,
			false,
			true,
			false
		},
	
		
		{
			"has_used_this_pool",
			9,
			false,
			true,
			true
		},
	
		
		{
			"last_checked_at_block",
			10,
			false,
			false,
			false
		},
	
		
		{
			"last_nonce_when",
			11,
			false,
			false,
			true
		}
	
};


const std::vector< std::function< void(const DORM::Resultset &result, Account_ &obj) > > Account_::column_resultset_function = {
	
		
		[](const DORM::Resultset &result, Account_ &obj){ obj.columns[0] = static_cast<uint64_t>( result.getUInt64(1) ); },
	
		
		[](const DORM::Resultset &result, Account_ &obj){ obj.columns[1] = static_cast<DORM::Timestamp>( result.getString(2) ); },
	
		
		[](const DORM::Resultset &result, Account_ &obj){ obj.columns[2] = static_cast<uint64_t>( result.getUInt64(3) ); },
	
		
		[](const DORM::Resultset &result, Account_ &obj){ obj.columns[3] = static_cast<DORM::Timestamp>( result.getString(4) ); },
	
		
		[](const DORM::Resultset &result, Account_ &obj){ obj.columns[4] = static_cast<std::string>( result.getString(5) ); },
	
		
		[](const DORM::Resultset &result, Account_ &obj){ obj.columns[5] = static_cast<uint64_t>( result.getUInt64(6) ); },
	
		
		[](const DORM::Resultset &result, Account_ &obj){ obj.columns[6] = static_cast<bool>( result.getBoolean(7) ); },
	
		
		[](const DORM::Resultset &result, Account_ &obj){ obj.columns[7] = static_cast<std::string>( result.getString(8) ); },
	
		
		[](const DORM::Resultset &result, Account_ &obj){ obj.columns[8] = static_cast<bool>( result.getBoolean(9) ); },
	
		
		[](const DORM::Resultset &result, Account_ &obj){ obj.columns[9] = static_cast<uint64_t>( result.getUInt64(10) ); },
	
		
		[](const DORM::Resultset &result, Account_ &obj){ obj.columns[10] = static_cast<DORM::Timestamp>( result.getString(11) ); }
	
};


void Account_::clear() {
	// explictly [re]set to "empty" values
	
		
		
			columns[0] = static_cast<uint64_t>(0);
		
	
		
		
			columns[1] = DORM::Timestamp();
		
	
		
		
			columns[2] = static_cast<uint64_t>(0);
		
	
		
		
			columns[3] = DORM::Timestamp();
		
	
		
		
			columns[4] = std::string();
		
	
		
		
			columns[5] = static_cast<uint64_t>(0);
		
	
		
		
			columns[6] = false;
		
	
		
		
			columns[7] = std::string();
		
	
		
		
			columns[8] = false;
		
	
		
		
			columns[9] = static_cast<uint64_t>(0);
		
	
		
		
			columns[10] = DORM::Timestamp();
		
	

	Object::clear();
}


std::unique_ptr<DORM::Object> Account_::make_unique() {
	return std::make_unique<Account>();
}


void Account_::column_from_resultset( int i, const DORM::Resultset &result ) {
	( column_resultset_function[i] )( result, *this );
}


std::unique_ptr<Account> Account_::load() {
	DORM::Object::search();
	return result();
}


std::unique_ptr<Account> Account_::load( uint64_t key_accountID ) {
	Account obj;

	
		obj.accountID( key_accountID );
	

	return obj.load();
}


std::unique_ptr<Account> Account_::load(const DORM::Object &obj) {
	Account tmp;
	tmp.copy_columns(obj, true);
	return tmp.load();
}


std::unique_ptr<Account> Account_::result() {
	if ( !resultset || !resultset->next() ) {
		resultset.reset();
		return std::unique_ptr<Account>();
	}

	auto obj = std::make_unique<Account>();
	obj->set_from_resultset( *resultset );
	return obj;
}


void Account_::search_and_destroy() {
	search();

	while( auto victim = result() )
		victim->delete_obj();
}


std::unique_ptr<Account> Account_::clone() const {
	auto obj = std::make_unique<Account>();

	obj->copy_columns(*this, false);

	for (auto &column : obj->columns)
		column.changed = false;
	return obj;
}


// navigators

