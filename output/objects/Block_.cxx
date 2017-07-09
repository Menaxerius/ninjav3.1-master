// GENERATED - DO NOT EDIT!

// our class
#include "Block_.hxx"
#include "Block.hpp"

// other classes

	#include "Nonce.hpp"

	#include "Share.hpp"

	#include "Reward.hpp"



#include <memory>


const std::vector<DORM::Object::Info> Block_::column_info = {
	
		
		{
			"blockID",
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
			"best_nonce_account_id",
			3,
			false,
			false,
			false
		},
	
		
		{
			"generator_account_id",
			4,
			false,
			false,
			false
		},
	
		
		{
			"block_reward",
			5,
			false,
			false,
			false
		},
	
		
		{
			"is_our_block",
			6,
			false,
			true,
			true
		},
	
		
		{
			"has_been_shared",
			7,
			false,
			true,
			true
		},
	
		
		{
			"base_target",
			8,
			false,
			false,
			false
		},
	
		
		{
			"forged_when",
			9,
			false,
			false,
			true
		},
	
		
		{
			"scoop",
			10,
			false,
			false,
			false
		},
	
		
		{
			"nonce",
			11,
			false,
			false,
			false
		},
	
		
		{
			"generation_signature",
			12,
			false,
			false,
			false
		},
	
		
		{
			"deadline",
			13,
			false,
			false,
			false
		},
	
		
		{
			"our_best_deadline",
			14,
			false,
			false,
			false
		},
	
		
		{
			"num_potential_miners",
			15,
			false,
			false,
			false
		},
	
		
		{
			"num_rejected_nonces",
			16,
			false,
			true,
			true
		}
	
};


const std::vector< std::function< void(const DORM::Resultset &result, Block_ &obj) > > Block_::column_resultset_function = {
	
		
		[](const DORM::Resultset &result, Block_ &obj){ obj.columns[0] = static_cast<uint64_t>( result.getUInt64(1) ); },
	
		
		[](const DORM::Resultset &result, Block_ &obj){ obj.columns[1] = static_cast<DORM::Timestamp>( result.getString(2) ); },
	
		
		[](const DORM::Resultset &result, Block_ &obj){ obj.columns[2] = static_cast<uint64_t>( result.getUInt64(3) ); },
	
		
		[](const DORM::Resultset &result, Block_ &obj){ obj.columns[3] = static_cast<uint64_t>( result.getUInt64(4) ); },
	
		
		[](const DORM::Resultset &result, Block_ &obj){ obj.columns[4] = static_cast<uint64_t>( result.getUInt64(5) ); },
	
		
		[](const DORM::Resultset &result, Block_ &obj){ obj.columns[5] = static_cast<bool>( result.getBoolean(6) ); },
	
		
		[](const DORM::Resultset &result, Block_ &obj){ obj.columns[6] = static_cast<bool>( result.getBoolean(7) ); },
	
		
		[](const DORM::Resultset &result, Block_ &obj){ obj.columns[7] = static_cast<uint64_t>( result.getUInt64(8) ); },
	
		
		[](const DORM::Resultset &result, Block_ &obj){ obj.columns[8] = static_cast<DORM::Timestamp>( result.getString(9) ); },
	
		
		[](const DORM::Resultset &result, Block_ &obj){ obj.columns[9] = static_cast<uint32_t>( result.getUInt(10) ); },
	
		
		[](const DORM::Resultset &result, Block_ &obj){ obj.columns[10] = static_cast<uint64_t>( result.getUInt64(11) ); },
	
		
		[](const DORM::Resultset &result, Block_ &obj){ obj.columns[11] = static_cast<std::string>( result.getString(12) ); },
	
		
		[](const DORM::Resultset &result, Block_ &obj){ obj.columns[12] = static_cast<uint32_t>( result.getUInt(13) ); },
	
		
		[](const DORM::Resultset &result, Block_ &obj){ obj.columns[13] = static_cast<uint32_t>( result.getUInt(14) ); },
	
		
		[](const DORM::Resultset &result, Block_ &obj){ obj.columns[14] = static_cast<uint32_t>( result.getUInt(15) ); },
	
		
		[](const DORM::Resultset &result, Block_ &obj){ obj.columns[15] = static_cast<uint32_t>( result.getUInt(16) ); }
	
};


void Block_::clear() {
	// explictly [re]set to "empty" values
	
		
		
			columns[0] = static_cast<uint64_t>(0);
		
	
		
		
			columns[1] = DORM::Timestamp();
		
	
		
		
			columns[2] = static_cast<uint64_t>(0);
		
	
		
		
			columns[3] = static_cast<uint64_t>(0);
		
	
		
		
			columns[4] = static_cast<uint64_t>(0);
		
	
		
		
			columns[5] = false;
		
	
		
		
			columns[6] = false;
		
	
		
		
			columns[7] = static_cast<uint64_t>(0);
		
	
		
		
			columns[8] = DORM::Timestamp();
		
	
		
		
			columns[9] = static_cast<uint32_t>(0);
		
	
		
		
			columns[10] = static_cast<uint64_t>(0);
		
	
		
		
			columns[11] = std::string();
		
	
		
		
			columns[12] = static_cast<uint32_t>(0);
		
	
		
		
			columns[13] = static_cast<uint32_t>(0);
		
	
		
		
			columns[14] = static_cast<uint32_t>(0);
		
	
		
		
			columns[15] = static_cast<uint32_t>(0);
		
	

	Object::clear();
}


std::unique_ptr<DORM::Object> Block_::make_unique() {
	return std::make_unique<Block>();
}


void Block_::column_from_resultset( int i, const DORM::Resultset &result ) {
	( column_resultset_function[i] )( result, *this );
}


std::unique_ptr<Block> Block_::load() {
	DORM::Object::search();
	return result();
}


std::unique_ptr<Block> Block_::load( uint64_t key_blockID ) {
	Block obj;

	
		obj.blockID( key_blockID );
	

	return obj.load();
}


std::unique_ptr<Block> Block_::load(const DORM::Object &obj) {
	Block tmp;
	tmp.copy_columns(obj, true);
	return tmp.load();
}


std::unique_ptr<Block> Block_::result() {
	if ( !resultset || !resultset->next() ) {
		resultset.reset();
		return std::unique_ptr<Block>();
	}

	auto obj = std::make_unique<Block>();
	obj->set_from_resultset( *resultset );
	return obj;
}


void Block_::search_and_destroy() {
	search();

	while( auto victim = result() )
		victim->delete_obj();
}


std::unique_ptr<Block> Block_::clone() const {
	auto obj = std::make_unique<Block>();

	obj->copy_columns(*this, false);

	for (auto &column : obj->columns)
		column.changed = false;
	return obj;
}


// navigators

	std::unique_ptr<Nonce> Block_::block_nonces() const {
		
			auto obj = std::make_unique<Nonce>();
			obj->copy_columns(*this, true);
			return obj;
		
	}

	std::unique_ptr<Share> Block_::block_shares() const {
		
			auto obj = std::make_unique<Share>();
			obj->copy_columns(*this, true);
			return obj;
		
	}

	std::unique_ptr<Reward> Block_::block_rewards() const {
		
			auto obj = std::make_unique<Reward>();
			obj->copy_columns(*this, true);
			return obj;
		
	}

