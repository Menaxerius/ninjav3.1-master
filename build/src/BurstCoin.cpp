#include "BurstCoin.hpp"

#include "JSON.hpp"
#include "fetch.hpp"
#include "ftime.hpp"

#include <algorithm>
#include <iostream>
#include <regex>
#include <cmath>
#include <thread>
#include <future>


// same order as hpp file please //

const std::string BurstCoin::currency_suffix = "BURST";


class ResponseCounts: public std::map<std::string,int> {
	private:
		typedef std::pair<std::string,int> our_pair_t;

	public:
		bool operator() (const our_pair_t &pair1, const our_pair_t &pair2) { return pair1.second < pair2.second; };

		std::string best_response() {
			const auto &best_it = std::max_element( this->begin(), this->end(), *this );

			return best_it->first;
		}
};


std::string BurstCoin::server_RPC( const std::string &request_type, param_map_t *params, bool first_server_only ) {
	std::string data = "requestType=" + request_type;

	if (params)
		for(const auto &map_it : *params)
			data += "&" + map_it.first + "=" + map_it.second;

	// in "first_server_only" mode we want to do this sequentially with standard CURL timeout
	// otherwise do all requests in parallel

	if (first_server_only) {
		for(const auto &server_name : servers) {
			std::string url = "http://" + server_name + "/burst";

			#ifdef DEBUG_BURSTCOIN
				std::cout << "TO " << url << ": " << data << std::endl;
			#endif

			std::string response = fetch( url, data, "application/x-www-form-urlencoded", "" );

			#ifdef DEBUG_BURSTCOIN
				std::cout << "FROM " << url << ": " << response << std::endl;
			#endif

			if ( response.empty() )
				continue;

			return response;
		}

		// No response from any server?
		throw CryptoCoins::server_issue();
	}

	// parallel mode
	ResponseCounts response_counts;		// only actually used in parallel mode
	std::vector< std::future<std::string> > futures;

	for(const auto &server_name : servers) {
		std::string url = "http://" + server_name + "/burst";

		std::future<std::string> fetch_future = std::async( std::launch::async, fetch, url, data, "application/x-www-form-urlencoded", "", server_timeout );
		futures.push_back( std::move(fetch_future) );
	}

	// short-sleep spin until all futures are ready/timed-out
	while( futures.size() > 0 ) {
		  std::chrono::milliseconds spin_period(100);

		  for(int i=0; i<futures.size(); ++i)
			  if ( futures[i].wait_for(spin_period) == std::future_status::ready ) {
				// this future is cooked!
				std::string response = futures[i].get();

				// remove from vector
				futures.erase( futures.begin() + i );
				--i;

				if ( response.empty() )
					continue;

				if ( response.find( "\"requestProcessingTime\":" ) == std::string::npos )
					continue;

				// strip out requestProcessingTime, which may vary between servers
				response = std::regex_replace( response, std::regex("\"requestProcessingTime\":[[:digit:]]+"), "\"requestProcessingTime\":0" );

				// "response" is the key into the map, if entry doesn't exist, initialize with 0
				auto insert_ret = response_counts.insert( std::make_pair(response, 0) );
				// increase count
				++insert_ret.first;
			}
	}

	// try to clean "data"
	data.replace(0, std::string::npos, data.size(), 'x');

	// No response from any server?
	if ( response_counts.size() == 0 )
		throw CryptoCoins::server_issue();

	// pick response with highest count
	return response_counts.best_response();
}


CryptoCoinTx BurstCoin::convert_transaction_JSON(const JSON &transaction_json) {
	CryptoCoinTx tx;
	tx.currency = "BURST";

	tx.tx_id = transaction_json.get_string("transaction");

	if ( transaction_json.exists("height") )
		tx.block_height = transaction_json.get_uint64("height");
	else
		tx.block_height = 0;
	// sometimes transactions have a height of max-signed-32bit-int aka 2^31 - 1 aka 2147483647
	// these are transactions that haven't made it into a block
	if (tx.block_height == 2147483647)
		tx.block_height = 0;

	if ( transaction_json.exists("confirmations") )
		tx.confirmations = transaction_json.get_uint64("confirmations");
	else
		tx.confirmations = 0;

	tx.sender = transaction_json.get_string("senderRS");

	if ( transaction_json.exists("fee") )
		tx.fee = transaction_json.get_uint64("feeNQT");
	else
		tx.fee = 0;

	tx.fee_inclusive = false;

	if ( transaction_json.exists("attachment") ) {
		JSON attachment_json = transaction_json.get_item("attachment");

		if ( transaction_json.exists("message") )
			tx.message = transaction_json.get_string("message");
	}

	tx.crypto_timestamp = transaction_json.get_number("timestamp");
	tx.unix_timestamp = crypto_to_unix_time(tx.crypto_timestamp);

	std::string recipientRS = transaction_json.get_string("recipientRS");
	uint64_t amountNQT = transaction_json.get_uint64("amountNQT");

	tx.recipient_amounts.push_back( CryptoCoinRecvPair(recipientRS, amountNQT) );

	// also SHA256 hash of the transaction contents to help detect replacement before fully confirmed
	std::string full_hash = transaction_json.get_string("fullHash");
	memcpy( tx.contents_SHA256, full_hash.c_str(), sizeof(tx.contents_SHA256) );

	return tx;
}

		
BurstCoin::BurstCoin( const std::string &srv ) {
	servers = { srv };
	currency = "BURST";
}


BurstCoin::BurstCoin( const std::vector<std::string> &srvs, const unsigned int timeout ) {
	servers = srvs;
	currency = "BURST";
	server_timeout = timeout;
}


std::vector<CryptoCoinTx> BurstCoin::get_recent_transactions( std::string account, time_t unix_time, bool include_unconfirmed ) {
	uint64_t burst_time = unix_to_crypto_time( unix_time );

	// "timestamp" seems to be a block timestamp rather than a transaction's timestamp
	// so transactions appear in block-time-granularity

	param_map_t params;
	params["account"] = account;
	params["timestamp"] = std::to_string(burst_time);
	params["numberOfConfirmations"] = std::to_string(include_unconfirmed ? 0 : 1);

	std::string response_json = server_RPC("getAccountTransactions", &params);	// NB: deprecated in NRS, to be removed in NRS v1.6

	JSON root;

	try {
		root = JSON(response_json);
	} catch( const JSON::parse_error &e) {
		throw CryptoCoins::server_issue();
	}

	if ( root.null("transactions") ) {
		std::cerr << ftime() << "[BurstCoin::get_recent_transactions] getAccountTransactions RPC returned odd JSON:\n" << response_json << std::endl;
		throw CryptoCoins::server_issue();
	}

	std::vector<CryptoCoinTx> transactions;

	JSON_Array transactions_json = root.get_array("transactions");

	for(int i=0; i<transactions_json.size(); i++) {
		JSON transaction_json = transactions_json.get_item(i);

		if (transaction_json.get_number("type") == 0) {
			CryptoCoinTx tx = convert_transaction_JSON(transaction_json);
			transactions.push_back( tx );
		}
	}

	return transactions;
}


CryptoCoinTx BurstCoin::get_transaction( CryptoCoinTx info_tx ) {
	param_map_t params;
	params["transaction"] = info_tx.tx_id;

	std::string response_json = server_RPC("getTransaction", &params);

	JSON transaction_json;

	try {
		transaction_json = JSON(response_json);
	} catch( const JSON::parse_error &e) {
		throw CryptoCoins::server_issue();
	}

	if ( transaction_json.null("type") || transaction_json.get_number("type") != 0 )
		throw CryptoCoins::unknown_transaction();

	return convert_transaction_JSON(transaction_json);
}


bool BurstCoin::send_transaction( CryptoCoinTx &tx ) {
	// need exactly one recipient
	if ( tx.recipient_amounts.size() != 1 )
		return false;

	// don't even try to send zero or negative amount
	if ( tx.recipient_amounts[0].amount <= 0 )
		return false;
		
	// fee is amount / 1000 rounded up to nearest 1 BURST (i.e. 1e8 NQT)
	uint64_t feeNQT = tx.fee != 0 ? tx.fee : (((tx.recipient_amounts[0].amount - 1) / amount_divisor / 1000) + 1) * amount_divisor;
	uint64_t amountNQT = tx.recipient_amounts[0].amount;

	// if fee is inclusive make sure it's not more than the actual amount
	if (tx.fee_inclusive && feeNQT >= amountNQT)
		return false;

	if (tx.fee_inclusive)
		amountNQT -= feeNQT;

	param_map_t params;
	params["recipient"] = tx.recipient_amounts[0].recipient;
	params["deadline"] = std::to_string(1440);	// seems to be the norm
	params["amountNQT"] = std::to_string(amountNQT);
	params["feeNQT"] = std::to_string(feeNQT);

	if ( !tx.message.empty() ) {
		params["messageIsText"] = "true";
		params["message"] = URL_encode(tx.message);
	}

	int pass_len = tx.encoded_passphrase.size();
	char pass[pass_len];
	memset(pass, 0, pass_len);

	for(int i=0; i<pass_len; i++)
		pass[i] = tx.encoded_passphrase[i] ^ ( (i+1) % 16 );

	// use
	params["secretPhrase"] = std::string(pass, pass_len);

	// clean
	memset(pass, 0, pass_len);

	#ifdef DEBUG_BURSTCOIN
		std::cout << ftime() << "Sending payment of " << pretty_amount(amountNQT) <<
					(tx.fee_inclusive ? " (including " : " (+ ") << pretty_amount(feeNQT) <<
					") to " << tx.recipient_amounts[0].recipient << std::endl;
	#endif

	std::string send_money_response = server_RPC("sendMoney", &params, true);

	if ( send_money_response.empty() )
		return false;

	JSON response_json;

	try {
		response_json = JSON(send_money_response);
	} catch( const JSON::parse_error &e) {
		throw CryptoCoins::server_issue();
	}

	if ( !response_json.exists("signatureHash") )
		return false;

	tx.tx_id = response_json.get_string("transaction");

	return true;
}


CryptoCoinKeyPair BurstCoin::generate_keypair() {
	#ifdef NO_ARC4RANDOM
		throw std::runtime_error("Couldn't generate Burstcoin keypair: no arc4random");
	#else
		// generate passphrase
		char passphrase[256];
		memset(passphrase, 0, sizeof(passphrase));

		// semi-random length
		int len = 200 + arc4random_uniform(40);

		static const char CHARS[] = "abcdefghijklmnopqrstuvwxyz_ABCDEFGHIJKLMNOPQRSTUVWXYZ 0123456789";

		for(int i=0; i<len; i++)
			passphrase[i] = CHARS[ arc4random_uniform(sizeof(CHARS) - 1) ];	// sizeof(CHARS) - 1 as we don't want to include NUL C-string terminator

	
		param_map_t params;
		params["secretPhrase"] = URL_encode( std::string(passphrase, len) );

		// generate account
		std::string response = server_RPC("getAccountId", &params);

		JSON response_json;

		try {
			response_json = JSON(response);
		} catch( const JSON::parse_error &e) {
			throw CryptoCoins::server_issue();
		}

		if ( !response_json.exists("accountRS") )
			throw std::runtime_error("Failed to generate keypair");
	
		CryptoCoinKeyPair kp;
		// TODO check passphrase variable shouldn't be going outside of scope.
		kp.priv_key = passphrase;
		kp.pub_address = response_json.get_string("accountRS");
		
		return kp;
	#endif
}


time_t BurstCoin::crypto_to_unix_time( uint64_t crypto_time ) {
	return crypto_time == 0 ? 0 : crypto_time + BURST_GENESIS_TIMESTAMP;
}


uint64_t BurstCoin::unix_to_crypto_time( time_t unix_time ) {
	return unix_time == 0 ? 0 : unix_time - BURST_GENESIS_TIMESTAMP;
}


std::string BurstCoin::pretty_amount( uint64_t amount ) {
	return _pretty_amount(amount, amount_divisor, currency_suffix);
}


std::string BurstCoin::get_account( const std::string &account ) {
	param_map_t params;
	params["account"] = account;
	return server_RPC("getAccount", &params);
}


std::string BurstCoin::get_reward_recipient( const std::string &account ) {
	param_map_t params;
	params["account"] = account;
	return server_RPC("getRewardRecipient", &params);
}


std::string BurstCoin::get_mining_info() {
	return server_RPC("getMiningInfo");
}


std::string BurstCoin::get_accounts_with_reward_recipient( const std::string &account ) {
	param_map_t params;
	params["account"] = account;
	return server_RPC("getAccountsWithRewardRecipient", &params);
}


std::string BurstCoin::submit_nonce( const uint64_t nonce, const uint64_t account, const std::string &secret ) {
	param_map_t params;
	params["nonce"] = std::to_string(nonce);
	params["accountId"] = std::to_string(account);
	params["secretPhrase"] = secret;

	std::string output = server_RPC("submitNonce", &params);

	// try to clean up secret in memory (arg "secret" is caller's responsiblity)
	std::string &clean_me = params["secretPhrase"];
	clean_me.replace(0, std::string::npos, clean_me.size(), 'x');

	return output;
}


std::string BurstCoin::get_block( const uint64_t blockID ) {
	param_map_t params;
	params["height"] = std::to_string(blockID);
	return server_RPC("getBlock", &params);
}


static const char initial_codeword[] = {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
static const char gexp[] = {1, 2, 4, 8, 16, 5, 10, 20, 13, 26, 17, 7, 14, 28, 29, 31, 27, 19, 3, 6, 12, 24, 21, 15, 30, 25, 23, 11, 22, 9, 18, 1};
static const char glog[] = {0, 0, 1, 18, 2, 5, 19, 11, 3, 29, 6, 27, 20, 8, 12, 23, 4, 10, 30, 17, 7, 22, 28, 26, 21, 25, 9, 16, 13, 14, 24, 15};
static const char codeword_map[] = {3, 2, 1, 0, 7, 6, 5, 4, 13, 14, 15, 16, 12, 8, 9, 10, 11};
static const char alphabet[] = "23456789ABCDEFGHJKLMNPQRSTUVWXYZ";

#define BASE_32_LENGTH 13
#define BASE_10_LENGTH 20


static char gmult(unsigned char a, unsigned char b) {
    if (a == 0 || b == 0)
        return 0;

    unsigned char idx = (glog[a] + glog[b]) % 31;

    return gexp[idx];
}


std::string BurstCoin::accountID_to_RS_string( const uint64_t accountID ) {
	if (accountID == 0)
		return "";

	// convert accountID into array of digits
	std::string plain_string = std::to_string( accountID );
	int length = plain_string.length();

	char plain_string_10[BASE_10_LENGTH];
	memset(plain_string_10, 0, BASE_10_LENGTH);

    for(int i = 0; i < length; i++)
        plain_string_10[i] = (char)plain_string[i] - (char)'0';

    int codeword_length = 0;
    char codeword[ sizeof(initial_codeword) ];
    memcpy(codeword, initial_codeword, sizeof(initial_codeword));

    do {  // base 10 to base 32 conversion
        int new_length = 0;
        int digit_32 = 0;

        for (int i = 0; i < length; i++) {
            digit_32 = digit_32 * 10 + plain_string_10[i];

            if (digit_32 >= 32) {
                plain_string_10[new_length] = digit_32 >> 5;
                digit_32 &= 31;
                new_length++;
            } else if (new_length > 0) {
                plain_string_10[new_length] = 0;
                new_length++;
            }
        }

        length = new_length;
        codeword[codeword_length] = digit_32;
        codeword_length++;
    } while(length > 0);

    char p[] = {0, 0, 0, 0};
    for (int i = BASE_32_LENGTH - 1; i >= 0; i--) {
        char fb = codeword[i] ^ p[3];
        p[3] = p[2] ^ gmult(30, fb);
        p[2] = p[1] ^ gmult(6, fb);
        p[1] = p[0] ^ gmult(9, fb);
        p[0] =        gmult(17, fb);
    }

    for(int i = 0; i<sizeof(p); i++)
    	codeword[BASE_32_LENGTH + i] = p[i];

    std::string account_RS_string = "BURST-";

    for (int i = 0; i < sizeof(initial_codeword); i++) {
        unsigned char codeword_index = codeword_map[i];
        unsigned char alphabet_index = codeword[codeword_index];

        account_RS_string += alphabet[alphabet_index];

        if ((i & 3) == 3 && i < 13)
            account_RS_string += "-";
    }

    return account_RS_string;
}
