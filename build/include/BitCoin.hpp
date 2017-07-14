#ifndef INCLUDE__BITCOIN_HPP
#define INCLUDE__BITCOIN_HPP

#include "CryptoCoins.hpp"

#include "JSON.hpp"
#include <bsd/stdlib.h>
#include <stdint.h>
#include <string>
#include <vector>
#include <map>
#include <stdexcept>


class BitCoin : public CryptoCoin {
	private:
		static std::map<std::string, uint64_t> block_height_by_hash;

		static const uint64_t amount_divisor = 100000000UL;
		std::string server_auth;

		JSON server_RPC( std::string method, JSON_Array *params = nullptr );
		
		JSON get_vout( JSON result, uint64_t vout_index );
		std::string get_vout_address( JSON vout );

		JSON get_raw_transaction( std::string tx_hash );
		uint64_t get_block_height( std::string block_hash );
		bool unlock_wallet( const std::string encoded_passphrase );

	public:
		static const std::string currency_suffix;

		BitCoin(std::string srv);
		
		virtual std::vector<CryptoCoinTx> get_recent_transactions( std::string account, time_t unix_time, bool include_unconfirmed = true );
		virtual CryptoCoinTx get_transaction( CryptoCoinTx tx );
		virtual bool send_transaction( CryptoCoinTx &tx );

		virtual CryptoCoinKeyPair generate_keypair();
		static std::string base58check_encode( const uint8_t* const str, const uint64_t len );

		virtual std::string pretty_amount( uint64_t amount );
};


#endif
