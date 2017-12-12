#ifndef INCLUDE__BURSTCOIN_HPP
#define INCLUDE__BURSTCOIN_HPP

#include "CryptoCoins.hpp"

#include "JSON.hpp"
#include <bsd/stdlib.h>
#include <stdint.h>
#include <string>
#include <vector>
#include <map>


class BurstCoin : public CryptoCoin {
	private:
		static const uint64_t BURST_GENESIS_TIMESTAMP = 1407722400UL;
		static const uint64_t amount_divisor = 100000000UL;

		std::vector<std::string> servers;
		unsigned int server_timeout = 3;

		typedef std::map<std::string, std::string> param_map_t;

		JSON server_RPC( const std::string &request_type, param_map_t *params = nullptr, const bool first_server_only = false, const std::vector<int> &critical_error_codes = {} );

		CryptoCoinTx convert_transaction_JSON(const JSON &transaction_json);

	public:
		static const std::string currency_suffix;

		BurstCoin( const std::string &srv);
		BurstCoin( const std::vector<std::string> &srvs, const unsigned int timeout = 3 );
		
		virtual std::vector<CryptoCoinTx> get_recent_transactions( std::string account, time_t unix_time, bool include_unconfirmed = true );
		virtual CryptoCoinTx get_transaction( CryptoCoinTx tx );
		virtual bool send_transaction( CryptoCoinTx &tx );

		virtual CryptoCoinKeyPair generate_keypair();

		virtual uint64_t unix_to_crypto_time(time_t unix_time);
		virtual time_t crypto_to_unix_time(uint64_t crypto_time);

		virtual std::string pretty_amount( uint64_t amount );

		// these are essentially NXT API calls
		JSON get_account( const std::string &account );
		JSON get_reward_recipient( const std::string &account );
		JSON get_mining_info();
		JSON get_accounts_with_reward_recipient( const std::string &account );
		JSON submit_nonce( const uint64_t nonce, const uint64_t account, const std::string &secret );
		JSON get_block( const uint64_t blockID );
		JSON get_unconfirmed_transactionIDs( const uint64_t accountID = 0 );

		static std::string accountID_to_RS_string( const uint64_t accountID );
};


#endif
