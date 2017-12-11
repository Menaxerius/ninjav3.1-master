#ifndef INCLUDE__CRYPTOCOINS_HPP
#define INCLUDE__CRYPTOCOINS_HPP

#include <stdexcept>
#include <string>
#include <vector>

#include <stdint.h>
#include <time.h>


class CryptoCoinKeyPair {
	public:
		std::string			priv_key;
		std::string			pub_address;
};


class CryptoCoinRecvPair {
	public:
		std::string			recipient;
		uint64_t			amount;
		
		CryptoCoinRecvPair( std::string r, uint64_t a ) : recipient(r), amount(a) {};
};


class CryptoCoinTx {
	public:
		std::string						currency;
		std::string						tx_id;
		uint64_t						block_height;	// meaningless unless confirmations >= 1
		uint64_t						confirmations;
		std::string						sender;
		uint64_t						fee = 0;
		bool							fee_inclusive;
		uint64_t						crypto_timestamp;
		time_t							unix_timestamp;
		std::string						message;
		std::string						encoded_passphrase;
		std::vector<CryptoCoinRecvPair>	recipient_amounts;
		// SHA256 hash of the transaction contents (inputs and outputs) to check for changes until fully confirmed
		char							contents_SHA256[65];	// hex char string

		// CryptoCoinTx(): fee(0) {};
};


class CryptoCoin {
	protected:
		std::string server;

		static std::string _pretty_amount( uint64_t amount, uint64_t amount_divisor, std::string currency_suffix );

	public:
		std::string	currency;

		virtual ~CryptoCoin() {};

		virtual std::vector<CryptoCoinTx> get_recent_transactions( std::string account, time_t unix_time, bool include_unconfirmed = true ) =0;
		virtual CryptoCoinTx get_transaction( CryptoCoinTx tx ) =0;
		virtual bool send_transaction( CryptoCoinTx &tx ) =0;
		virtual int get_confirmations( CryptoCoinTx tx );
		virtual CryptoCoinKeyPair generate_keypair() =0;

		virtual uint64_t unix_to_crypto_time(time_t unix_time) { return unix_time; };
		virtual time_t crypto_to_unix_time(uint64_t crypto_time) { return crypto_time; };

		virtual std::string pretty_amount( uint64_t amount ) =0;
};


class CryptoCoins {
	private:
		std::vector<CryptoCoin *> coin_servers;
		
	public:
		~CryptoCoins();
		void add_coin_server( CryptoCoin *coin );

		std::vector<CryptoCoinTx> get_recent_transactions( std::string currency, std::string address, time_t unix_time, bool include_unconfirmed = true );
		CryptoCoinTx get_transaction( CryptoCoinTx tx );
		bool send_transaction( CryptoCoinTx &tx );
		int get_confirmations( CryptoCoinTx tx );
		CryptoCoinKeyPair generate_keypair( std::string currency );

		std::string pretty_amount( std::string currency, uint64_t amount );

		class unknown_transaction : public std::runtime_error {
			public: unknown_transaction() : std::runtime_error("unknown cryptocoin transaction") {};
		};

		class not_enough_funds : public std::runtime_error {
			public: not_enough_funds() : std::runtime_error("not enough funds") {};
		};

		class server_issue : public std::runtime_error {
			public: server_issue() : std::runtime_error("cryptocoin server issue") {};
		};
};

#endif
