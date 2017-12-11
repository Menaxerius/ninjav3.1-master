#include "Account.hpp"

#include "Block.hpp"
#include "Share.hpp"

#include "DORM/sql/sqlOr.hpp"
#include "DORM/sql/sqlIsNull.hpp"
#include "DORM/sql/sqlLt.hpp"
#include "DORM/sql/sqlGe.hpp"

#include "JSON.hpp"
#include "config.hpp"
#include "BurstCoin.hpp"

#include <cctype>


// how many of an account's best deadlines in the last HISTORIC blocks to use when estimating miner's capacity
const int ESTIMATED_CAPACITY_DEADLINES = 8;


void Account::search_prep( DORM::Query &query ) const {
	if (needs_updating) {
		if ( needs_updating() )
			query.and_where( DORM::sqlOr( DORM::sqlIsNull("last_updated_when"), DORM::sqlLt<DORM::Timestamp>( "last_updated_when", time(nullptr) - ACCOUNT_UPDATE_TIMEOUT ) ) );
		else
			query.and_where( DORM::sqlGe<DORM::Timestamp>( "last_updated_when", time(nullptr) - ACCOUNT_UPDATE_TIMEOUT ) );
	}
}



void Account::save() {
	if ( accountID() && account_RS_string().empty() )
		account_RS_string( BurstCoin::accountID_to_RS_string( accountID() ) );

	DORM::Object::save();
}


uint64_t Account::estimate_capacity(const uint64_t latest_blockID) {
	const uint64_t from_blockID = latest_blockID - HISTORIC_CAPACITY_BLOCK_COUNT;

	Share shares;
	shares.accountID( accountID() );
	// don't include latest block as trying to access that contributes to transaction deadlocks
	shares.before_blockID( latest_blockID - 1 );
	shares.after_blockID( from_blockID );
	shares.weighted_deadline( true );
	shares.limit = ESTIMATED_CAPACITY_DEADLINES;
	shares.search();

	std::vector<uint64_t> deadlines;
	while( auto share = shares.result() )
		deadlines.push_back( share->deadline() ); // actually weighted deadline

	uint64_t total = 0;
	const int n_half_deadlines = deadlines.size() / 2;
	for(int i=0; i<n_half_deadlines; ++i)
		total += deadlines[i];

	if (n_half_deadlines == 0)
		return 0;

	const uint64_t mean_weighted_deadline = total / n_half_deadlines;
	if (mean_weighted_deadline == 0)
		return 0;

	// capacity in GiB
	const uint64_t fudge_factor = 5e12;
	return fudge_factor / mean_weighted_deadline;
}


void Account::update_check(BurstCoin &burst, uint64_t blockID) {
	std::string account = std::to_string( this->accountID() );

	try {
		JSON account_json( burst.get_account(account) );

		if ( !account_json.null("account") ) {
			// good result
			if ( !account_json.null("name") )
				this->account_name( account_json.get_string("name") );
			else
				this->undef_account_name();
		}

		// unless capacity is KNOWN then estimate
		if ( !this->defined_is_capacity_estimated() || this->is_capacity_estimated() ) {
			this->mining_capacity( this->estimate_capacity(blockID) );
			this->is_capacity_estimated(true);
		}

		this->last_updated_when( time(nullptr) );
		this->save();
	} catch (const CryptoCoins::server_issue &e) {
		// some (hopefully temporary) server issue not worth exiting for
	} catch (const JSON::parse_error &e) {
		// some (hopefully temporary) server issue not worth exiting for
	}
}


uint64_t Account::sum_capacities() {
	DORM::Query query;
	query.tables = static_table_name();
	query.cols = { "SUM(mining_capacity)" };

	return DORM::DB::fetch_uint64(query);
}


std::unique_ptr<Account> Account::load_or_create( uint64_t some_accountID ) {
	// passed a 0 accountID?
	if ( some_accountID == 0 )
		return std::unique_ptr<Account>();

	auto account = Account::load( some_accountID );

	if ( !account ) {
		account = std::make_unique<Account>();
		account->accountID( some_accountID );
		account->account_RS_string( BurstCoin::accountID_to_RS_string(some_accountID) );
	}

	return account;
}


static inline std::string pp( const std::string &prefix, const std::string &name ) {
	if ( prefix.empty() )
		return name;

	// need to uppercase first char of "name"
	std::string output( prefix );
	output += std::toupper( name[0] );
	output += name.substr(1);
	return output;
}


void Account::account_info_JSON( uint64_t accountID, JSON &info_json, const std::string &prefix ) {
	// don't use "update_check()" here as it massively slows down the whole server
	// there'll be a separate thread to update these accounts
	auto account = Account::load_or_create(accountID);

	if (account) {
		info_json.add_string( pp(prefix, "account"), account->account_RS_string() );
		info_json.add_string( pp(prefix, "accountId"), std::to_string(accountID) );
		info_json.add_uint( pp(prefix, "accountId32"), accountID & 0xFFFFFFFF );
		info_json.add_string( pp(prefix, "accountName"), account->account_name() );
		info_json.add_number( pp(prefix, "miningCapacityTB"), account->mining_capacity() / 1024.0 );
		if ( account->defined_is_capacity_estimated() && !account->is_capacity_estimated() )
			info_json.add_boolean( pp(prefix, "miningCapacityNotEstimated"), true );
	} else {
		info_json.add_string( pp(prefix, "account"), "???" );
		info_json.add_string( pp(prefix, "accountId"), "0" );
		info_json.add_uint( pp(prefix, "accountId32"), 0 );
		info_json.add_string( pp(prefix, "accountName"), "???" );
		info_json.add_number( pp(prefix, "miningCapacityTB"), 0 );
	}
}
