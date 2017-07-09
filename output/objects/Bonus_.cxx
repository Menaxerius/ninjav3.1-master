// GENERATED - DO NOT EDIT!

// our class
#include "Bonus_.hxx"
#include "Bonus.hpp"

// other classes



#include <memory>


const std::vector<DORM::Object::Info> Bonus_::column_info = {
	
		
		{
			"bonusID",
			1,
			true,
			false,
			false
		},
	
		
		{
			"tx_id",
			2,
			false,
			true,
			false
		},
	
		
		{
			"amount",
			3,
			false,
			true,
			false
		},
	
		
		{
			"seen_when",
			4,
			false,
			true,
			true
		}
	
};


const std::vector< std::function< void(const DORM::Resultset &result, Bonus_ &obj) > > Bonus_::column_resultset_function = {
	
		
		[](const DORM::Resultset &result, Bonus_ &obj){ obj.columns[0] = static_cast<uint64_t>( result.getUInt64(1) ); },
	
		
		[](const DORM::Resultset &result, Bonus_ &obj){ obj.columns[1] = static_cast<uint64_t>( result.getUInt64(2) ); },
	
		
		[](const DORM::Resultset &result, Bonus_ &obj){ obj.columns[2] = static_cast<uint64_t>( result.getUInt64(3) ); },
	
		
		[](const DORM::Resultset &result, Bonus_ &obj){ obj.columns[3] = static_cast<DORM::Timestamp>( result.getString(4) ); }
	
};


void Bonus_::clear() {
	// explictly [re]set to "empty" values
	
		
		
			columns[0] = static_cast<uint64_t>(0);
		
	
		
		
			columns[1] = static_cast<uint64_t>(0);
		
	
		
		
			columns[2] = static_cast<uint64_t>(0);
		
	
		
		
			columns[3] = DORM::Timestamp();
		
	

	Object::clear();
}


std::unique_ptr<DORM::Object> Bonus_::make_unique() {
	return std::make_unique<Bonus>();
}


void Bonus_::column_from_resultset( int i, const DORM::Resultset &result ) {
	( column_resultset_function[i] )( result, *this );
}


std::unique_ptr<Bonus> Bonus_::load() {
	DORM::Object::search();
	return result();
}


std::unique_ptr<Bonus> Bonus_::load( uint64_t key_bonusID ) {
	Bonus obj;

	
		obj.bonusID( key_bonusID );
	

	return obj.load();
}


std::unique_ptr<Bonus> Bonus_::load(const DORM::Object &obj) {
	Bonus tmp;
	tmp.copy_columns(obj, true);
	return tmp.load();
}


std::unique_ptr<Bonus> Bonus_::result() {
	if ( !resultset || !resultset->next() ) {
		resultset.reset();
		return std::unique_ptr<Bonus>();
	}

	auto obj = std::make_unique<Bonus>();
	obj->set_from_resultset( *resultset );
	return obj;
}


void Bonus_::search_and_destroy() {
	search();

	while( auto victim = result() )
		victim->delete_obj();
}


std::unique_ptr<Bonus> Bonus_::clone() const {
	auto obj = std::make_unique<Bonus>();

	obj->copy_columns(*this, false);

	for (auto &column : obj->columns)
		column.changed = false;
	return obj;
}


// navigators

